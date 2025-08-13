#pragma once
#include <string>
#include <vector>
#include <memory>

#include <nlohmann/json_fwd.hpp>

#include "json_manager.h"
#include "singleton.h"


struct TJsonFile {
	std::unique_ptr<NLOHMANN_JSON_NAMESPACE::json> json_converted{}; //file data as json
	std::string file_path;
	std::vector<std::string> v_required_fields;
	std::string password;
	TJsonFile(const std::string& _path, std::vector<std::string>&& _fields = {}, const std::string& _password = "")
		: file_path(_path), v_required_fields(std::move(_fields)), password(_password) {
	}

	void Clear() noexcept;

	bool CheckRequiredFiles() const;
};

namespace NSingletons {

	class CJsonLoader : public CSingleton<CJsonLoader> {
	public:
		const NLOHMANN_JSON_NAMESPACE::json* GetJsonValue(const TJsonFile& jFile, const std::string& path) const noexcept;
		bool LoadFile(TJsonFile& jFile) const noexcept;
	};

}

#define jsonLoaderInstance NSingletons::CJsonLoader::getInstance()