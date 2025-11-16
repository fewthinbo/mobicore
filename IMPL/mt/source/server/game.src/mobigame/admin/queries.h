#pragma once
#if __MOBICORE__
#include <string>
#include <sstream>

namespace mobi_game {
#if __OFFSHOP__ == 1
	static constexpr const char* SCHEMA_ACCOUNT = "srv1_account";
	static constexpr const char* SCHEMA_PLAYER = "srv1_player";
	static constexpr const char* SCHEMA_COMMON = "srv1_common";
#else
	static constexpr const char* SCHEMA_ACCOUNT = "account";
	static constexpr const char* SCHEMA_PLAYER = "player";
	static constexpr const char* SCHEMA_COMMON = "common";
#endif

	namespace query {
#if !__MT_DB_INFO__ //@deprecated
		static const std::string QUERY_ACCOUNT_WITH_EMPIRE = []() -> std::string {
			std::stringstream ss;
			ss << "SELECT a.id, a.login, pi.empire, a.email, ";
			ss << "COALESCE(CAST(gl.mAuthority AS UNSIGNED), 0) AS authority ";
			ss << "FROM " << SCHEMA_ACCOUNT << ".account a ";
			ss << "LEFT JOIN " << SCHEMA_PLAYER << ".player_index pi ON a.id = pi.id ";
			ss << "LEFT JOIN " << SCHEMA_COMMON << ".gmlist gl ON a.id = gl.mID";
			return ss.str();
			}();

		static const std::string QUERY_PLAYER = []() -> std::string {
			std::stringstream ss;
			ss << "SELECT p.id, p.account_id, p.name, p.job, ";
			ss << "p.map_index, p.playtime, p.level, p.last_play, ";
			ss << "COALESCE(g.guild_id, 0) as guild_id, ";
			ss << "COALESCE(g.is_general, 0) as is_guild_leader ";
			ss << "FROM " << SCHEMA_PLAYER << ".player p ";
			ss << "LEFT JOIN " << SCHEMA_PLAYER << ".guild_member g ON p.id = g.pid";
			return ss.str();
			}();

		static const std::string QUERY_GUILD = []() -> std::string {
			std::stringstream ss;
			ss << "SELECT id, name, master, level, win, draw, loss, ladder_point FROM ";
			ss << SCHEMA_PLAYER << ".guild";
			return ss.str();
			}();

		static const std::string QUERY_GUILD_MEMBER = []() -> std::string {
			std::stringstream ss;
			ss << "SELECT pid, guild_id, grade, is_general, offer FROM ";
			ss << SCHEMA_PLAYER << ".guild_member";
			return ss.str();
			}();

		static const std::string QUERY_MESSENGER_LIST = []() -> std::string {
			std::stringstream ss;
			ss << "SELECT account, companion FROM ";
			ss << SCHEMA_PLAYER << ".messenger_list";
			return ss.str();
			}();
#endif
	}
}
#endif