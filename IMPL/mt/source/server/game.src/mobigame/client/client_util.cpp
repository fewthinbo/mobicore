#include "mobi_base.h"

#include <vector>

#if __MOBICORE__
#include "war_map.h"
#endif

#include <Singletons/log_manager.h>
#include <Network/buffer.h>

#include "constants/packets.h"
#include "client_core.h"

using namespace network;

namespace mobi_game {
	void GameClientBase::SendSync() {
		if (sync_packet_sent_) return;
		if (!bridge_cache_ready_) return;

		std::vector<uint8_t> data{};
		GetSyncData(data);
		
		if (data.empty()) {
			LOG_TRACE("There is no data to sync");
			sync_packet_sent_ = true;
			return;
		}

		static uint8_t retry_count = 0;
		static constexpr uint8_t kMaxRetryCount = 3;
		if (retry_count >= kMaxRetryCount) {
			LOG_TRACE("Reached max failed attempt, skipping permanently.");
			retry_count = 0;
			sync_packet_sent_ = true;
			return;
		}

		//unprocessed olarak eklenmesi istenmedigi icin direkt Send() kullaniliyor.
		if (client_->Send(data) != ESendResult::SUCCESS) {
			LOG_TRACE("sync packet couldn't sent, will retry");
			++retry_count;
			return;
		}

		retry_count = 0;
		LOG_TRACE("sync packet sent successfully");
		sync_packet_sent_ = true;
	}

#if __MOBICORE__
	static void WritePids(TMP_BUFFER& buf) {
		std::vector<uint32_t> pids{};
		const auto& cl_desc = DESC_MANAGER::instance().GetClientSet();
		size_t desc_count = cl_desc.size();
		pids.resize(desc_count);

		for (const auto& d : cl_desc) {
			if (!d) continue;
			auto ch = d->GetCharacter();
			if (!ch) continue;
			uint32_t pid = ch->GetPlayerID();
			LOG_TRACE("pid(?) will be sync.", pid);
			pids.emplace_back(pid);
		}

		buf.write(pids.data(), sizeof(uint32_t) * desc_count);
	}

	static void WriteWars(TMP_BUFFER& buf) {
		auto get_list = [&buf](CWarMap* war) {
			if (!war) return;
			CGuild* gld1 = war->GetGuild(0);
			CGuild* gld2 = war->GetGuild(1);

			if (!gld1 || !gld2) return;

			const auto& team1 = war->m_TeamData[0];
			const auto& team2 = war->m_TeamData[1];

			TWarElem war_pack{};
			war_pack.gids[0] = team1.dwID;
			war_pack.gids[1] = team2.dwID;

			war_pack.scores[0] = team1.iScore;
			war_pack.scores[1] = team2.iScore;
#if __FIGHTER_SCORE_SYNC__
			war_pack.team_size[0] = team1.iMemberCount * sizeof(TWarFighter);
			war_pack.team_size[1] = team2.iMemberCount * sizeof(TWarFighter);
#endif
			buf.write(&war_pack, sizeof(TWarElem));

#if __FIGHTER_SCORE_SYNC__
			auto write_fighters = [&buf, war](uint32_t gid) {
				for (const auto& pair : war->map_MemberStats) {
					uint32_t pid = pair.first;
					const TMemberStats* stats = pair.second;
					if (!stats) continue;

					if (stats->dwGuildId != gid) continue;

					TWarFighter fighter_pack{};
					fighter_pack.pid = pid;
					fighter_pack.kills = stats->dwKills;
					fighter_pack.deaths = stats->dwDeaths;
					buf.write(&fighter_pack, sizeof(TWarFighter));
				}
			};
			//once team1'in fightlar ini yaz
			write_fighters(team1.dwID);
			write_fighters(team2.dwID);
#endif
		};

		war_manager.for_each(std::move(get_list));
	}
#endif

	/*optimizasyon dolayisiyla, paketler sadece oyuncunun bulundugu port'a gonderilir.
	**Bir** kez bu paket calistirilmalidir cunku:
	ara sunucu kapali, mt aciksa ve ara sunucu yeni/yeniden baslatiliyorsa, senkronizasyon ihtiyaci dogar:
	oyuncularin bulundugu portlari -db'de olmayan bir bilgi- initialize etmek onemlidir. Aksi halde oyuncular map degistirene kadar paket alamazlar.*/
	void GameClientBase::GetSyncData(std::vector<uint8_t>& data) const noexcept {
		data.clear();

#if __MOBICORE__
		const auto& cl_desc = DESC_MANAGER::instance().GetClientSet();
		LOG_TRACE("? desc found to sync", cl_desc.size());
		TSIZE player_size = cl_desc.size() * sizeof(uint32_t);

		auto& war_manager = CWarMapManager::instance();
		TSIZE war_size = war_manager.CountWarMap() * sizeof(TWarElem); //tahmini boyut

		MSReSync packet{};
		packet.header = HEADER_MS_SYNC;
		packet.size = player_size + war_size;

		TMP_BUFFER buf(sizeof(MSReSync) + packet.size);

		WritePids(buf);

		WriteWars(buf);

		data = std::move(buf.get());
#endif
	}
}