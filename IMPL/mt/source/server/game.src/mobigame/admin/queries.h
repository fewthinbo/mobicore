#pragma once
#ifdef MOBICORE
#include <memory>
#include "db.h"
#endif
namespace mobi_game {
#if !defined(ENABLE_MT_DB_INFO)
	namespace query {
		static constexpr const char* ACCOUNT_WITH_EMPIRE =
			"SELECT a.id, a.login, pi.empire, a.email, "
			"COALESCE(CAST(gl.mAuthority AS UNSIGNED), 0) AS authority "
			"FROM account.account a LEFT JOIN "
			"player.player_index "
			"pi ON a.id = pi.id "
			"LEFT JOIN common.gmlist gl ON gl.mID = a.id";

		static constexpr const char* PLAYER =
			"SELECT "
			"player.id, "
			"player.account_id, "
			"player.name, "
			"player.job, "
			"player.map_index, player.level, player.playtime, "
			"player.last_play, "
			"COALESCE(guild_member.guild_id, 0) as guild_id, "
			"COALESCE(guild_member.is_general, 0) as is_guild_leader "
			"FROM player.player "
			"LEFT JOIN guild_member ON player.id = guild_member.pid";

		static constexpr const char* GUILD = "SELECT id, name, master, level, win, draw, loss, ladder_point FROM player.guild";

		static constexpr const char* GUILD_MEMBER = "SELECT pid, guild_id, grade, is_general, offer FROM player.guild_member";

		static constexpr const char* MESSENGER_LIST = "SELECT account, companion FROM player.messenger_list";
	}
#if defined(MOBICORE)
	namespace utils {
		inline std::unique_ptr<SQLMsg> GetResultOfQuery(const char* query) {
			auto& db_inst = DBManager::instance();

			std::unique_ptr<SQLMsg> ret(db_inst.DirectQuery(query));
			if (!ret) {
				LOG_TRACE("Sql response of query(?) is nullptr", query);
				return {};
			}

			SQLResult* sql_res = ret->Get();
			if (!sql_res) {
				LOG_TRACE("Weird stuff happened. Line(?), query(?)", __LINE__, query);
				return {};
			}

			if (sql_res->uiNumRows == 0) {
				LOG_TRACE("Query(?) result is empty.", query);
				return {};
			}

			return ret;
		}
	}
#endif
#endif
}