#pragma once
#include <unordered_map>
#include <memory>
#include <cstdint>
#include <chrono>

#include <singleton.h>
#include <Utility/container.h>

#include "constants/packets.h"

#if __BUILD_FOR_GAME__
class CHARACTER;
class DESC;
#endif

enum class ELoadChResult : uint8_t {
	LOADING, //loading for mobile
	ALREADY_FOR_MOBILE, //the ch is already loaded for mobile request
	IN_GAME, //character is in-game actively
	SYSTEM_ERR,
};
struct TMobiEnterGame;

namespace mobi_game {

	//using as temporary parameter
	struct TMobiLoginInfo {
		uint32_t pid{};
		std::string login{};
	};

	//Bu depolama sinifi core bazli calisacak yani
	//Bu core'dan cikildiginda kaynaklar otomatik temizlenmelidir. HEADER_GG gonderilen yerler vb(ben simdilik gondermeyecegim)
	//Cunku oyuna sokup isinlamiyoruz.
	//Ek olarak process icerisinde otomatik kontrol edilip temizlenecektir.

	//Auth'a ait veri
	struct TMobiCharacter final {
		TMobiLoginInfo info{};
		uint32_t handle_id{};

		explicit TMobiCharacter(TMobiLoginInfo&& info, uint32_t hid) 
			: info(std::move(info)), handle_id(hid) {
		}
		~TMobiCharacter() noexcept;
	};

	class CMobiCharManager : public CSingleton<CMobiCharManager> {
	public:
		~CMobiCharManager() noexcept;
		void Process(); //cleanups etc.
#if __BUILD_FOR_GAME__
		void NotifyStatus(DESC* d, EMobiLoad code);
#endif
	private:
		TMobiCharacter* MobiGetByHandleID(uint32_t hid) const;
#if __BUILD_FOR_GAME__
		DESC* CreateDesc() const;
	public: //Auth
		void HandleLoginResult(DESC* d, bool res);
		ELoadChResult CharacterLoad(TMobiLoginInfo&& info, const std::string& pw);
	private:
		void SendMobiLogin(LPDESC d);

	public: //game
		void SendLoginRequest(const char* data);
		void CharacterSelect(DESC* d);
		void Entergame(DESC* d);
		bool HandleLogout(uint32_t pid);
		void SendMobiWarp(LPDESC d, int32_t addr, uint16_t port);
#endif
	private:
		improved::Container<uint32_t/*pid*/, TMobiCharacter> descs_{};
		
		static constexpr auto CH_CLEANER_INTERVAL = std::chrono::minutes(15); //15 dakikada bir cache temizlenir
		std::chrono::steady_clock::time_point last_cleanup{};
	};
}

#define mobileChInstance mobi_game::CMobiCharManager::getInstance()