#if __MOBICORE__
#if __BUILD_FOR_GAME__
#include "stdafx.h"
#endif

#include "ch_manager.h"

#if __BUILD_FOR_GAME__
#include "char.h"
#include "char_manager.h"
#include "p2p.h"
#include "desc.h"
#include "desc_client.h"
#include "messenger_manager.h"
#include "desc_manager.h"
#include "config.h"
#include "buffer_manager.h"
#include "packet.h"
#include "../../../common/tables.h"
#include "../../../common/length.h"
#include "db.h"
#endif

#include <Singletons/log_manager.h>

#include "constants/packet_constants.h"
#include "mobi_client.h"

#if __BUILD_FOR_GAME__
extern bool g_bShutdown; //Check in functions which are using DESC_MANAGER etc.
#endif

namespace mobi_game {

	static bool IsLoadFailed(EMobiLoad code) {
		switch (code)
		{
		case EMobiLoad::SUCCESS:
		case EMobiLoad::LOADING:
			return false;
		default:
			return true;
		}
	}

	TMobiCharacter::~TMobiCharacter() noexcept {
#if __BUILD_FOR_GAME__
		if (g_bShutdown) return;

		auto& desc_manager = DESC_MANAGER::instance();
		auto* d = desc_manager.FindByHandle(this->handle_id);
		if (!d) return;

		LOG_TRACE("MobiDesc(hid: ?, pid: ?) found and destroying", this->handle_id, this->info.pid);
		/*isinlanmalarda login_key'in kalmasini istiyorum*/
		/*if (!g_bAuthServer) { //Auth server'da henuz login islemi bitmemis olacaktir
			LOG_TRACE("Destroying login key(?)", d->GetLoginKey());
			desc_manager.DestroyLoginKey(this->info.login);
		}*/
		desc_manager.DestroyDesc(d);
#endif
	};

	CMobiCharManager::~CMobiCharManager() noexcept = default;

	void CMobiCharManager::NotifyStatus(DESC* d, EMobiLoad code) {
		if (!d) return;
		if (!d->is_mobile_request) {
			LOG_TRACE("Desc(hid: ?) not loaded for mobile, code(?)", d->GetHandle(), code);
			return;
		}

		auto* ptr = MobiGetByHandleID(d->GetHandle());
		if (!ptr) {
			LOG_TRACE("MobiData is nullptr for handle_id(?), code(?)", d->GetHandle(), code);
			return;
		}
		uint32_t pid = ptr->info.pid;

		mobileInstance.sendChLoadStatus(pid, code);
		LOG_TRACE("Code(?) notified to server for pid(?)", code, pid);
		
		if (!IsLoadFailed(code)) return;
		
		LOG_TRACE("Load progress failed, desc(hid: ?) will remove: code(?), pid(?)", ptr->handle_id, code, pid);

		descs_.EraseByKey(pid);
	}

	TMobiCharacter* CMobiCharManager::MobiGetByHandleID(uint32_t hid) const {
		auto found = std::find_if(descs_.begin(), descs_.end(), [hid](const auto& mobi) {
			if (!mobi) return false;
			return hid == mobi->handle_id;
		});

		if (found == descs_.end()) {
			LOG_TRACE("Desc(hid:?) is not exists for mobi", hid);
			return nullptr;
		}
		return found->get();
	}

#if __BUILD_FOR_GAME__
	DESC* CMobiCharManager::CreateDesc() const {
		if (g_bShutdown) {
			LOG_TRACE("Shutdown progress");
			return nullptr;
		}
		auto& desc_manager = DESC_MANAGER::instance();

		//=============================== SETUP
		auto* newd = M2_NEW DESC;
		if (!newd) {
			LOG_TRACE("Memory allocation error while creating desc.");
			return nullptr;
		}

		uint32_t handle_id = ++desc_manager.m_iHandleCount;
		newd->m_dwHandle = handle_id;
		newd->m_bHandshaking = false;
		newd->SetPong(true);
		newd->m_stHost = "1";
		newd->is_mobile_request = true;
		newd->last_activity = std::chrono::steady_clock::now();

		desc_manager.m_map_handle.emplace(handle_id, newd);
		desc_manager.m_set_pkDesc.emplace(newd);
		//login name map'ine otomatik olarak eklenecektir.

		++desc_manager.m_iSocketsConnected;
		//=============================== EOF SETUP
		return newd;
	}
#endif

	//====================================================== AUTH BEGIN
	ELoadChResult CMobiCharManager::CharacterLoad(TMobiLoginInfo&& info, const std::string& pw) {
#if __BUILD_FOR_GAME__
		if (g_bShutdown) {
			LOG_TRACE("Shutdown progress");
			return ELoadChResult::SYSTEM_ERR;
		}
#endif

		uint32_t pid = info.pid;
		if (auto* mobi_ptr = descs_.FindByKey(pid))/*auth'da desc hala duruyorsa*/ {
			LOG_TRACE("Same request received for pid(?), already loading.", pid);
			return ELoadChResult::LOADING;
		}

#if __BUILD_FOR_GAME__
		if (auto* desc_found = DESC_MANAGER::instance().FindByLoginName(info.login)) {
			LOG_ERR("Desc(login: ?) already exists in this core(port: ?, host: ?), for_mobile(?)", info.login, mother_port, g_szPublicIP, desc_found->is_mobile_request);
			return desc_found->is_mobile_request ? ELoadChResult::ALREADY_FOR_MOBILE : ELoadChResult::IN_GAME;
		}

		//LoginSuccess'dan sonraki islemler icin tum core'lar arasinda arama yap.
		if (CCI * p2p_found = P2P_MANAGER::instance().FindByPID(pid)) {
			p2p_found->dwPID = 0;
			mobileInstance.sendChLoadStatus(pid, p2p_found->is_mobile_request ? EMobiLoad::SUCCESS : EMobiLoad::INGAME_REAL);

			if (!p2p_found->is_mobile_request) {
				LOG_TRACE("Pid(?) found in another core but it's normal login, skipping to load character", pid);
				return ELoadChResult::IN_GAME;
			}
			else {
				LOG_TRACE("Character(?) already loaded for mobile", pid); 
				return ELoadChResult::ALREADY_FOR_MOBILE;
			}
		}

		auto* newd = CreateDesc();
		if (!newd) {
			LOG_TRACE("Memory allocation error for desc(pid:?).", pid);
			return ELoadChResult::SYSTEM_ERR;
		}

		LOG_TRACE("Desc(hid: ?) created for pid: ?", newd->GetHandle(), pid);

		//=============================== PREPARE LOGIN PACK
		TPacketCGLogin3 LoginPacket{}; //it will also initialize adwClientKey with zero
		LoginPacket.header = HEADER_CG_LOGIN3;
		TMP_BUFFER::str_copy(LoginPacket.login, LOGIN_MAX_LEN, info.login.c_str());
		TMP_BUFFER::str_copy(LoginPacket.passwd, PASSWD_MAX_LEN, pw.c_str());
		LoginPacket.login[LOGIN_MAX_LEN] = '\0';
		LoginPacket.passwd[PASSWD_MAX_LEN] = '\0';


		descs_.Emplace(pid, std::move(info), newd->GetHandle());

		//=============================== LOGIN
		LOG_TRACE("Id(?), pw(?) trying to login for mobile", LoginPacket.login, LoginPacket.passwd);

		newd->SetPhase(PHASE_AUTH);
		newd->m_inputAuth.Login(newd, reinterpret_cast<const char*>(&LoginPacket));
		//=============================== EOF LOGIN
#endif
		return ELoadChResult::LOADING;
	}

#if __BUILD_FOR_GAME__
	void CMobiCharManager::SendMobiLogin(LPDESC d) {
		if (!d) return;
		if (!d->is_mobile_request) {
			LOG_TRACE("Desc(hid: ?) is not for mobile, skipping", d->GetHandle());
			return;
		}

		if (!g_bAuthServer) {
			LOG_TRACE("Channel(id:?, port:?) is not auth server.", g_bChannel, mother_port);
			return;
		}

		auto* ptr = MobiGetByHandleID(d->GetHandle());
		if (!ptr) {
			LOG_TRACE("MobiData is nullptr for handle_id(?)", d->GetHandle());
			return;
		}
		uint32_t pid = ptr->info.pid;

		TMobiGD pack{};
		pack.pid = pid;
		pack.login_key = d->GetLoginKey();
		db_clientdesc->DBPacket(HEADER_GD_MOBI_LOGIN, 0, &pack, sizeof(pack)); //en uygun core bulunup loginKey ile giris yapilir.
	}

	void CMobiCharManager::SendMobiWarp(LPDESC d, int32_t addr, uint16_t port) {
		if (!d) return;
		if (!d->is_mobile_request) {
			LOG_TRACE("Desc(hid: ?) is not for mobile, skipping", d->GetHandle());
			return;
		}
		d->last_activity = std::chrono::steady_clock::now();

		if (g_bAuthServer) {
			LOG_TRACE("You cannot warp in auth server.");
			return;
		}

		/*if (mother_port == port) {
			LOG_TRACE("Same core(port: ?) detected, directly entering", port);
			return;
		}*/

		auto* ptr = MobiGetByHandleID(d->GetHandle());
		if (!ptr) {
			LOG_TRACE("MobiData is nullptr for handle_id(?)", d->GetHandle());
			return;
		}

		uint32_t pid = ptr->info.pid;

		TMobiGDWarp pack{};
		pack.info.pid = pid;
		pack.info.login_key = d->GetLoginKey();
		pack.addr = addr;
		pack.port = port;
		db_clientdesc->DBPacket(HEADER_GD_MOBI_WARP, 0, &pack, sizeof(pack)); //hedef core'a giris
	}

	void CMobiCharManager::HandleLoginResult(DESC* d, bool res) {
		if (!d) return;
		if (g_bShutdown) {
			LOG_TRACE("Shutdown progress");
			return;
		}
		if (!d->is_mobile_request) {
			LOG_TRACE("Desc(hid: ?) is not loading for mobile", d->GetHandle());
			return;
		}
		LOG_TRACE("EOF AUTH: Login result for hid(?): ?", d->GetHandle(), res);

		mobileChInstance.NotifyStatus(d, res ? EMobiLoad::LOADING : EMobiLoad::WRONG_PWD);

		if (!res) return;

		uint32_t hid = d->GetHandle();
		auto desc = DESC_MANAGER::instance().FindByHandle(hid);
		if (!desc) {
			LOG_TRACE("Desc couldn't found by hid: ?", hid);
			return;
		}

		LOG_TRACE("Desc(hid: ?) found in auth", hid);
		SendMobiLogin(d);
	}
	//====================================================== EOF AUTH 


	//g_pkAuthMasterDesc, db_clientdesc
	//====================================================== LOGIN BEGIN
	//this function running in most avail core for logging to game
	//HEADER_DG_MOBI_LOGIN tetikler.
	void CMobiCharManager::SendLoginRequest(const char* data) {
		LOG_TRACE("Login request received to channel(?)", g_bChannel);
		if (g_bShutdown) {
			LOG_TRACE("Shutdown progress");
			return;
		}

		auto* received = reinterpret_cast<const TMobiDGLogin*>(data);
		LOG_TRACE("Inside of packet: login(?), key(?), pid(?)", received->login, received->login_key, received->pid);

		if (descs_.EraseByKey(received->pid)) {
			LOG_ERR("MobiDesc(pid: ?) existed & deleted in this core(port: ?, host: ?)", received->pid, mother_port, g_szPublicIP);
		}
		if (auto* desc_found = DESC_MANAGER::instance().FindByLoginKey(received->login_key)) {
			LOG_ERR("Desc(hid: ?, pid: ?) already exists in this core(port: ?, host: ?), for_mobile(?)", desc_found->GetHandle(), received->pid, mother_port, g_szPublicIP, desc_found->is_mobile_request);
			if (!desc_found->is_mobile_request) return;
			LOG_ERR("Desc(hid: ?, pid: ?) removing, will create newd", desc_found->GetHandle(), received->pid);
			DESC_MANAGER::instance().DestroyDesc(desc_found);
		}

		auto* newd = CreateDesc();
		if (!newd) {
			return;
		}

		LOG_TRACE("Desc(hid: ?) created for pid: ?", newd->GetHandle(), received->pid);
		descs_.Emplace(received->pid, TMobiLoginInfo{ received->pid, received->login }, newd->GetHandle());

		TPacketCGLogin2 pack{};
		pack.header = HEADER_CG_LOGIN2;
		pack.dwLoginKey = received->login_key;
		TMP_BUFFER::str_copy(pack.login, LOGIN_MAX_LEN, received->login);
		pack.login[LOGIN_MAX_LEN] = '\0';

		newd->SetPhase(PHASE_LOGIN);
		newd->m_inputLogin.LoginByKey(newd, reinterpret_cast<const char*>(&pack));
	}

	void CMobiCharManager::CharacterSelect(DESC* d) {
		if (g_bShutdown) {
			LOG_TRACE("Shutdown progress");
			return;
		}

		if (!d) return;
		if (!d->is_mobile_request) {
			LOG_TRACE("Character is not loaded for mobi, skipping");
			return;
		}

		LOG_TRACE("Selecting character for desc(hid: ?, login_key: ?)", d->GetHandle(), d->GetLoginKey());
		auto* ptr = MobiGetByHandleID(d->GetHandle());
		if (!ptr) {
			LOG_TRACE("MobiData is nullptr for handle_id(?)", d->GetHandle());
			return;
		}

		int ch_idx = d->GetAccountTable().GetIndexByPID(ptr->info.pid);
		if (ch_idx == -1) {
			LOG_TRACE("Index not found for pid(?)", ptr->info.pid);
			return;
		}

		TPacketCGPlayerSelect pack{}; //other members will initialize default
		pack.header = HEADER_CG_CHARACTER_SELECT;
		pack.index = ch_idx;

		d->last_activity = std::chrono::steady_clock::now();
		d->SetPhase(PHASE_SELECT);
		d->m_inputLogin.CharacterSelect(d, reinterpret_cast<const char*>(&pack));
	}

	class TScopeFlag {
		bool& flag_;
	public:
		TScopeFlag(bool& flag) : flag_(flag) {
			flag_ = !flag_;
		}
		~TScopeFlag() {
			flag_ = !flag_;
		}
	};
	void CMobiCharManager::Entergame(DESC* d) {
		if (!d) return;
		if (g_bShutdown) {
			LOG_TRACE("Shutdown progress");
			return;
		}
		if (!d->is_mobile_request) {
			LOG_TRACE("Character is not loaded for mobi, skipping");
			return;
		}
		auto* ch = d->GetCharacter();
		if (!ch) {
			LOG_WARN("Character not exists in mobiDesc(hid: ?)", d->GetHandle());
			return;
		}

		d->last_activity = std::chrono::steady_clock::now();
		d->SetPhase(PHASE_LOADING);
		TPacketCGEnterGame pack{};
		{
			TScopeFlag controller(ch->can_mobi_warp);
			d->m_inputLogin.Entergame(d, reinterpret_cast<const char*>(&pack));
		}

		LOG_TRACE("MobiCh(hid: ?) loggined to game", d->GetHandle());
		NotifyStatus(d, EMobiLoad::SUCCESS);
	}
	//====================================================== EOF LOGIN
#endif

	/*IMPORTANT: Bu func, sadece disaridan cagirilmalidir,
	* Db'den cevap alindiktan sonra silinir. Bu sayede core'a ait inputDb vs icerisinden cagirilmis olur.*/
	bool CMobiCharManager::HandleLogout(uint32_t pid) {
		if (!descs_.EraseByKey(pid)) {
			LOG_TRACE("Ch(?) already removed", pid);
			return false;
		}
		LOG_TRACE("Desc(pid: ?) removed from channel(id:?, port:?).", pid, g_bChannel, mother_port);
		mobileInstance.sendChLoadStatus(pid, EMobiLoad::LOGOUT);
		return true;
	}

	void CMobiCharManager::Process() {
		static constexpr auto ACTIVITY_TIMEOUT = std::chrono::minutes(15);
		auto now = std::chrono::steady_clock::now();
		if (now - last_cleanup < CH_CLEANER_INTERVAL) return;
		last_cleanup = now;
#if __BUILD_FOR_GAME__
		if (g_bShutdown) {
			LOG_TRACE("Shutdown progress");
			return;
		}
		size_t removed_count = 0;

		std::vector<uint32_t> remove_vec;

		descs_.ForEach([&remove_vec, &now](TMobiCharacter& mobi) -> void {
			bool need_remove{ true };
			uint32_t pid = mobi.info.pid;
			if (g_bShutdown) return;

			auto* desc = DESC_MANAGER::Instance().FindByHandle(mobi.handle_id);
			if (!desc) {
				LOG_TRACE("Desc not exists in core(pid: ?)", pid);
			}
			else if (now - desc->last_activity > ACTIVITY_TIMEOUT) {
				LOG_TRACE("Desc(hid: ?) will remove due to activity timeout", desc->GetHandle());
			}
			else {
				need_remove = false;
			}

			if (need_remove) {
				remove_vec.emplace_back(pid);
			}
		});

		for (const auto& pid : remove_vec) {
			if (!descs_.EraseByKey(pid)) continue;
			++removed_count;
			mobileInstance.sendChLoadStatus(pid, EMobiLoad::NO_ACTIVITY);
		}

		if (removed_count > 0) {
			LOG_TRACE("? mobi_ch removed", removed_count);
		}
#endif
	}
}

#endif