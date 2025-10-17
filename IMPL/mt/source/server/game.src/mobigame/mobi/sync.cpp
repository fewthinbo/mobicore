#if __MOBICORE__
#if __BUILD_FOR_GAME__
#include "stdafx.h"
#endif
#include "mobi_client.h"

#if __BUILD_FOR_GAME__
#include "war_map.h"
#include "desc_manager.h"
#include "desc.h"
#include "char.h"
#endif

#include <Singletons/log_manager.h>

#include "client/client_base.h"

#include "constants/packets.h"

using namespace network;

namespace mobi_game {
	using namespace consts;

	void MobiClient::SendSync() {
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
		if (client_impl_->Send(data) != ESendResult::SUCCESS) {
			LOG_TRACE("sync packet couldn't sent, will retry");
			++retry_count;
			return;
		}

		retry_count = 0;
		LOG_TRACE("sync packet sent successfully");
		sync_packet_sent_ = true;
	}

#if __BUILD_FOR_GAME__
	std::pair<TSIZE, uint32_t> MobiClient::WritePids(TMP_BUFFER& buf) const noexcept {
		std::vector<uint32_t> pids{};
		const auto& cl_desc = DESC_MANAGER::instance().GetClientSet();
		for (const auto& d : cl_desc) {
			if (!d) continue;
			auto ch = d->GetCharacter();
			if (!ch) continue;
			uint32_t pid = ch->GetPlayerID();
			LOG_TRACE("pid(?) will be sync.", pid);
			pids.emplace_back(pid);
		}

		auto pid_count = pids.size();
		auto total_size = sizeof(uint32_t) * pid_count;
		buf.write(pids.data(), total_size);
		return { total_size, pid_count };
	}

	std::pair<TSIZE, uint32_t> MobiClient::WriteWars(TMP_BUFFER& buf) const noexcept {
		size_t total_size{};
		uint32_t war_count{};
		auto get_list = [&buf, &total_size, &war_count](CWarMap* war) {
			if (!war) return;
			CGuild* gld1 = war->GetGuild(0);
			CGuild* gld2 = war->GetGuild(1);

			if (!gld1 || !gld2) return;

			const auto& team1 = war->GetTeamData(0);
			const auto& team2 = war->GetTeamData(1);

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

			total_size += sizeof(TWarElem);
			war_count++;

#if __FIGHTER_SCORE_SYNC__
			auto write_fighters = [&buf, war, &total_size](uint32_t gid) {
				for (const auto& pair : war) {
					uint32_t pid = pair.first;
					const TMemberStats* stats = pair.second;
					if (!stats) continue;

					if (stats->dwGuildId != gid) continue;

					TWarFighter fighter_pack{};
					fighter_pack.pid = pid;
					fighter_pack.kills = stats->dwKills;
					fighter_pack.deaths = stats->dwDeaths;
					buf.write(&fighter_pack, sizeof(TWarFighter));
					total_size += sizeof(TWarFighter);
				}
			};
			//once team1'in fightlar ini yaz
			write_fighters(team1.dwID);
			write_fighters(team2.dwID);
#endif
		};

		CWarMapManager::instance().for_each(std::move(get_list));
		return { total_size, war_count };
	}
#endif

	/*optimizasyon dolayisiyla, paketler sadece oyuncunun bulundugu port'a gonderilir.
	**Bir** kez bu paket calistirilmalidir cunku:
	ara sunucu kapali, mt aciksa ve ara sunucu yeni/yeniden baslatiliyorsa, senkronizasyon ihtiyaci dogar:
	oyuncularin bulundugu portlari -db'de olmayan bir bilgi- initialize etmek onemlidir. Aksi halde oyuncular map degistirene kadar paket alamazlar.*/
	void MobiClient::GetSyncData(std::vector<uint8_t>& data) const noexcept {
		data.clear();

#if __BUILD_FOR_GAME__
		TMP_BUFFER buf_data{};
		std::pair<TSIZE, uint32_t> pid_data = WritePids(buf_data);
		std::pair<TSIZE, uint32_t> war_data = WriteWars(buf_data);
		if (buf_data.get().empty()) return;

		MSReSync packet{};
		packet.header = HEADER_MS_SYNC;
		packet.size = pid_data.first + war_data.first;
		packet.count_sync = pid_data.second;
		packet.count_war = war_data.second;

		TMP_BUFFER buf(packet.size);
		buf.write(&packet, sizeof(MSReSync));
		buf.write(buf_data.get().data(), packet.size);

		data = std::move(buf.get());
#endif
	}
}
#endif