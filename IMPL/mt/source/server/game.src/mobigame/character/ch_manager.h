#pragma once
#include <unordered_map>
#include <memory>
#include <cstdint>
#include <chrono>

#include <singleton.h>

class CHARACTER;
class DESC;

enum class ELoadChResult : uint8_t {
	LOADING, //loading for mobile
	ALREADY_FOR_MOBILE, //the ch is already loaded for mobile request
	IN_GAME, //character is in-game actively
	SYSTEM_ERR,
};

namespace mobi_game {

	//using as temporary parameter
	struct TMobiLoginInfo {
		uint32_t pid{}, acc_id{};
		std::string login{};
	};

	enum class EMobiChStatus {
		LOADING,
		READY,
		NONE,
	};

	//Bu depolama sinifi core bazli calisacak yani
	//Bu core'dan cikildiginda kaynaklar otomatik temizlenmelidir. HEADER_GG gonderilen yerler vb(ben simdilik gondermeyecegim)
	//Cunku oyuna sokup isinlamiyoruz.

	struct TMobiCharacter {
		TMobiLoginInfo info{};
		DESC* desc{ nullptr };
		CHARACTER* ch{ nullptr };
		EMobiChStatus status{ EMobiChStatus::LOADING };
		std::chrono::steady_clock::time_point last_activity{};
		std::chrono::steady_clock::time_point load_begin{};

		explicit TMobiCharacter(TMobiLoginInfo&& info, DESC* d, CHARACTER* ch) : pid(pid), desc(d), ch(ch) {
			load_begin = std::chrono::steady_clock::now();
		}

		~TMobiCharacter() noexcept;
		
		//TODO: herhangi bir islem gerceklestiginde bunu calistir.
		void UpdateActivity() {
			last_activity = std::chrono::steady_clock::now();
		}

		static constexpr auto ACTIVITY_TIMEOUT_MIN = std::chrono::minutes(20);
		static constexpr auto LOAD_TIMEOUT_MIN = std::chrono::minutes(5);
		bool IsTimeout(const std::chrono::steady_clock::time_point& now) const {
			bool is_activity_timeout = now - last_activity > ACTIVITY_TIMEOUT_MIN;
			bool is_load_timeout = status == EMobiChStatus::LOADING && (now - load_begin > LOAD_TIMEOUT_MIN);
			return is_activity_timeout || is_load_timeout;
		}
	};

	class CMobiCharP2P {
		bool P2PAdd(uint32_t hid, uint32_t pid) {
			return handles_.try_emplace(hid, pid).second;
		}
	private:
		std::unordered_map<uint32_t, uint32_t> handles_;//handle_id to pid
	};


	class CMobiCharManager : public CSingleton<CMobiCharManager> {
	public:
		~CMobiCharManager() noexcept override;
	public:
		ELoadChResult CharacterLoad(TMobiLoginInfo&& info, const std::string& pw);

		bool CharacterSetPhase(uint32_t pid, int phase);
		bool CharacterSetStatus(uint32_t pid, EMobiChStatus status);
		TMobiCharacter* MobiGetByPID(uint32_t pid);

		bool IsLoadingForMobi(DESC* d) const;

		void CharacterAdd(CHARACTER* ch);
	public:
		void Process(); //cleanups etc.
	private:
		template<typename ... Args>
		TMobiCharacter* CharacterEmplace(Args&& ... args) {
			auto& added = character_vec_.emplace_back(std::make_unique<TMobiCharacter>(std::forward<Args>(args)...));
			TMobiCharacter* ptr = added.get();
			character_map_.try_emplace(added->pid, ptr);
			return ptr;
		}
		bool CharacterRemove(uint32_t pid);
	private:
		std::unordered_map<uint32_t, TMobiCharacter*> character_map_; //once map tanimlanmalidir.
		std::vector<std::unique_ptr<TMobiCharacter>> character_vec_; //iterasyonlarin mapi invalidate etmemesi icin unique.
		
		static constexpr auto CH_CLEANER_INTERVAL = std::chrono::minutes(15); //15 dakikada bir cache temizlenir
		std::chrono::steady_clock::time_point last_cleanup{};
	};
}

#define mobileChInstance mobi_game::CMobiCharManager::getInstance()