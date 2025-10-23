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

namespace mobi_game {

	TMobiCharacter::~TMobiCharacter() noexcept {
		if (this->desc) {
			const auto& desc_manager = DESC_MANAGER::instance();
			desc_manager.DestroyLoginKey(desc);
			desc_manager.DestroyDesc(desc);
		}
	};

	CMobiCharManager::~CMobiCharManager() noexcept = default;

	bool CMobiCharManager::IsLoadingForMobi(DESC* d) const {
		if (!d) return false;
		uint32_t handle_id = d->GetHandle();
		
		auto found = std::find_if(character_vec_.begin(), character_vec_.end(), [handle_id](const auto& mobi) {
			if (!mobi) return false;
			auto* desc = mobi->desc;
			if (!desc) return false;
			return handle_id == desc->GetHandle();
		});

		if (found == character_vec_.end()) {
			LOG_TRACE("Desc(hid:?) is not loading for mobi", handle_id);
			return false;
		}

		auto* ptr = found->get();
		if (!ptr) {
			LOG_TRACE("Data is nullptr (hid:?)", handle_id);
			return false;
		}

		if (ptr->status != EMobiChStatus::LOADING) {
			LOG_TRACE("Data found but status doesn't match.");
		}

		return true;
	}

	TMobiCharacter* CMobiCharManager::MobiGetByPID(uint32_t pid) {
		auto found = character_map_.find(pid);
		if (found == character_map_.end()) return nullptr;
		return found->second;
	}

	bool CMobiCharManager::CharacterRemove(uint32_t pid) {
		size_t removed = character_map_.erase(pid);

		auto found = std::find_if(character_vec_.begin(), character_vec_.end(), [pid](const auto& mobi_ch) {
			if (!mobi_ch) return false;
			return mobi_ch->pid == pid;
		});

		bool found_in_vec = found != character_vec_.end();

		if (removed != found_in_vec) {
			LOG_ERR("Map(size: ?) and vector(size: ?) is not synced. Pid(?)", character_map_.size(), character_vec_.size(), pid);
			return false;
		}

		if (!found_in_vec) {
			LOG_TRACE("Character not found to remove(pid: ?)", pid);
			return false;
		}

		if (found != character_vec_.end() - 1) {
			*found = std::move(character_vec_.back());
		}
		character_vec_.pop_back();
		return true;
	}

	ELoadChResult CMobiCharManager::CharacterLoad(TMobiLoginInfo&& info, const std::string& pw) {
		/*
		Su an auth core'undayiz fakat asil veriler game core'larina dagitilmalidir.
		*/
		if (auto* mobi_ptr = MobiGetByPID(info.pid)) {
			if (mobi_ptr->status == EMobiChStatus::LOADING) {
				LOG_TRACE("Same request received for pid(?), already loading.", info.pid);
				return ELoadChResult::LOADING;
			}
			else {
				LOG_TRACE("Ptr found(pid: ?) but status(?) is not loading, will try to load", info.pid, mobi_ptr->status);
			}
		}
		//FIXME: LoginSuccess olmadan once zaten oyunda mi diye kontrol edilen yeri bul ve dogrudan cikis yap.
		//Hatta o kontrolu buraya ekle.
		//SORU: PlayerLoad'dan once auth core'undan veri cikabiliyor mu?

		//TODO: cci yapisina is_mobile eklenmesi gerekebilir.

		//Auth core'u icin sorgula:
		//Client zaten authenticate olmaya mi calisiyor?
		if (LPDESC d = DESC_MANAGER::instance().FindByLoginName(info.login)) {
			if (d->GetMobiInfo().is_loading_for_mobi) {
				//DESC'ler ortak degil, peki nasil senkronize oluyorlar.??
			}
		}

		//Zaten basarisiz girisler umrumuzda degil.
		//LoginSuccess'dan sonraki islemler icin tum core'lar arasinda arama yap.
		if (CCI* p2p_found = P2P_MANAGER::instance().FindByPID(info.pid)) {
			//Tum login islemleri sirasinda desc'te bilgi bulunmalidir.
			//Su an auth core'undayiz desc manager ile dogrudan bulabiliyorsak karakter yukleniyor demektir.
		}

		if (auto ch = CHARACTER_MANAGER::instance().FindByPID(info.pid)) {
			LOG_TRACE("Character(pid:?) already exists", info.pid);

			if (ch->is_mobile_request) {
				LOG_TRACE("Character(pid:?) already loaded for mobile.", info.pid);
				return ELoadChResult::ALREADY_FOR_MOBILE;
			}

			LOG_TRACE("Character(pid:?) is in game cannot load for mobile.", info.pid);
			return ELoadChResult::IN_GAME;
		}

		const auto& desc_manager = DESC_MANAGER::instance();

		//=============================== SETUP
		auto* newd = M2_NEW DESC;
		if (!newd) {
			LOG_TRACE("Memory allocation error for desc(pid:?).", info.pid);
			return ELoadChResult::SYSTEM_ERR;
		}

		uint32_t handle_id = ++desc_manager.m_iHandleCount;

		//IMPORTANT: db'ye haber sal, bu id mobil icin yuklenecek.
		TGDHandleData gd_pack{ handle_id , false };
		db_clientdesc->DBPacket(HEADER_GD_MOBI_LOAD, 0, gd_pack);

		newd->m_dwHandle = handle_id;
		newd->SetPong(true);
		newd->is_mobile_request = true;

		desc_manager.m_map_handle.emplace(handle_id, newd);
		desc_manager.m_set_pkDesc.emplace(newd);
		//++desc_manager.m_iSocketsConnected; //diger kullanici girislerini etkilememelidir.
		//=============================== EOF SETUP


		//=============================== LOGIN
		TPacketCGLogin3 LoginPacket{}; //it will also initialize adwClientKey with zero
		LoginPacket.header = HEADER_CG_LOGIN3;
		strncpy(LoginPacket.login, info.login, ID_MAX_NUM);
		strncpy(LoginPacket.passwd, pw, PASS_MAX_NUM);
		LoginPacket.login[ID_MAX_NUM] = '\0';
		LoginPacket.passwd[PASS_MAX_NUM] = '\0';

		newd->SetPhase(PHASE_AUTH);
		newd->m_inputAuth.Login(newd, LoginPacket);
		//=============================== EOF LOGIN

		CharacterEmplace(std::move(info), newd, nullptr);
		/*
			TPlayerLoadPacket pack{};
			pack.player_id = pid;
			pack.account_id = acc_id;
			pack.account_index = 0;

			LOG_TRACE("Load request sent to db, pid(?), acc_id(?)", pid, acc_id);

			db_clientdesc->DBPacket(HEADER_GD_PLAYER_LOAD, 0, &pack, sizeof(TPlayerLoadPacket));	
		*/

		return ELoadChResult::LOADING;
	}

	bool CMobiCharManager::CharacterSetPhase(uint32_t pid, int phase) {
		TMobiCharacter* ptr = MobiGetByPID(pid);
		if (!ptr) {
			LOG_TRACE("Mobi character(?) not exists", pid);
			return false;
		}

		if (!ptr->desc) {
			LOG_TRACE("Character(?) desc not exists", pid);
			return false;
		}

		const DESC& desc = *ptr->desc;

		if (desc.IsPhase(phase)) {
			LOG_TRACE("Character(?) is already in phase(?)", pid, phase);
			return false;
		}

		desc.SetPhase(phase);
		//TODO: playerload vs islemlerinden sonra status'u guncelle.

		if (ptr->status != EMobiChStatus::LOADING) return true;

		switch (phase) {
		case PHASE_LOGIN: {
			LOG_TRACE("Login key is ?" desc.GetLoginKey());

			TPacketCGLogin2 pack{};
			pack.header = HEADER_CG_LOGIN2;
			pack.dwLoginKey = desc.GetLoginKey();
			strncpy(pack.login, ptr->info.login, sizeof(pack.name) - 1);
			pack.login[ID_MAX_NUM] = '\0';

			desc.m_inputLogin.LoginByKey(desc, pack);
			break;
		}

		case PHASE_SELECT: {
			int ch_idx = desc.GetAccountTable().GetIndexByPID(ptr->info.pid);
			if (ch_idx == -1) {
				LOG_TRACE("Index not found for pid(?)", ptr->info.pid);
				break;
			}

			TPacketCGPlayerSelect pack{}; //other members will initialize default
			pack.header = HEADER_CG_CHARACTER_SELECT;
			pack.index = ch_idx;
			desc.m_inputLogin.CharacterSelect(desc, pack);
			break;
		}
		case PHASE_LOADING: {
			desc.m_inputLogin.Entergame(desc, nullptr);
			break;
		}
		default:
			break;
		}

		return true;
	}

	bool CharacterSetStatus(uint32_t pid, EMobiChStatus status) {
		TMobiCharacter* ptr = MobiGetByPID(pid);
		if (!ptr) {
			LOG_TRACE("Mobi character(?) not exists", pid);
			return false;
		}

		if (ptr->status == status) {
			LOG_TRACE("Ch has same status(?)", status);
			return false;
		}

		ptr->status = status;
		return true;
	}

	void CMobiCharManager::CharacterAdd(CHARACTER* ch) {
		if (!ch) return;

		uint32_t pid = ch->GetPlayerID();

		auto* ptr = MobiGetByPID(pid);
		if (!ptr) {
			LOG_TRACE("Pid(?) doesn't exists as mobi skipping", pid);
			return;
		}

		if (ptr->ch) {
			LOG_TRACE("Mobi_ch has already character data overwriting, pid(?)", pid);
		}

		ch->is_mobile_request = true;
		ptr->ch = ch;
		ptr->status = EMobiChStatus::READY;

		//handle id sil.
		TGDHandleData gd_pack{ handle_id , true };
		db_clientdesc->DBPacket(HEADER_GD_MOBI_LOAD, 0, gd_pack);
	}

	void CMobiCharManager::Process() {
		auto now = std::chrono::steady_clock::now();
		if (now - last_cleanup < CH_CLEANER_INTERVAL) return;

		size_t removed_count = 0;

		auto it = character_vec_.begin();
		while (it != character_vec_.end()) {
			bool need_remove{ false };
			auto* ptr = it->get();

			if (!ptr){
				need_remove = true;
			}
			else if(ptr->IsActivityTimeout() || ptr->status == EMobiChStatus::NONE) {
				need_remove = true;
			}
			else if (ptr->IsLoadTimeout()) {
				//notify to db
				if (auto* desc = ptr->desc) {
					TGDHandleData gd_pack{ desc->GetHandle() , true};
					db_clientdesc->DBPacket(HEADER_GD_MOBI_LOAD, 0, gd_pack);
				}
				else {
					LOG_TRACE("Desc not adjusted yet for pid(?)", ptr->info.pid);
				}

				need_remove = true;
			}

			if (need_remove) {
				it = character_vec_.erase(it);
				removed_count++;
				continue;
			}
			++it;
		}

		if (removed_count > 0) {
			LOG_TRACE("? mobi_ch removed", removed_count);
		}
	}
}

#endif