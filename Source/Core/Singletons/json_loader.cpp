#include "json_loader.h"

#include <fstream>

#include "log_manager.h"
#include "json_manager.h"
#include "crypter.h"


void TJsonFile::Clear() noexcept {
	if (!json_converted) return;
	json_converted->clear();
}

bool TJsonFile::CheckRequiredFiles() const {
	if (!json_converted) return false;
	if (v_required_fields.empty()) return true;

	bool ret = true;
	for (const auto& elem : v_required_fields) {
		if (!jsonInstance.checkField(*json_converted, elem)) {
			LOG_TRACE("Field(?) missing, file(?)", elem, file_path);
			ret = false;
		}
	}
	return ret;
}

namespace NSingletons
{
	const NLOHMANN_JSON_NAMESPACE::json* CJsonLoader::GetJsonValue(const TJsonFile& jFile, const std::string& path) const noexcept {
		if (!jFile.json_converted || jFile.json_converted->is_null()) return nullptr;

		std::vector<std::string> parts{};
		std::stringstream ss(path);
		std::string part{};

		while (std::getline(ss, part, '.')) {
			parts.push_back(part);
		}

		//sirayla tüm alanları kontrol et ve ic ice degerleri al
		const type_json* current = jFile.json_converted.get();

		for (const auto& p : parts) {
			if (!current->contains(p)) {
				return nullptr;
			}
			current = &current->at(p);
		}

		return current;
	}

	bool CJsonLoader::LoadFile(TJsonFile& jFile) const noexcept {
		LOG_INFO("File(?) loading", jFile.file_path);
		try
		{
			int mode = std::ios::in;

			if (!jFile.password.empty()) {
				mode = std::ios::binary;
			}

			std::ifstream file(jFile.file_path, mode);

			if (!file) {
				LOG_INFO("File not found: ?", jFile.file_path);
				return false;
			}

			if (!file.is_open()) {
				LOG_INFO("File couldn't be opened: ?", jFile.file_path);
				return false;
			}

			jFile.Clear();

			std::stringstream buffer;
			buffer << file.rdbuf();

			auto data_str = buffer.str();

			if (data_str.empty()) {
				LOG_INFO("File(?) is empty.", jFile.file_path);
				return false;
			}

			if (!jFile.password.empty()) {
				LOG_INFO("Decrypting data of file: ?", jFile.file_path);
				if (!cryInstance.DecryptData(data_str, jFile.password)) {
					LOG_INFO("Decryption failed: ?", jFile.file_path);
					return false;
				}
			}

			jFile.json_converted = std::make_unique<NLOHMANN_JSON_NAMESPACE::json>(jsonInstance.toJson(data_str));

			if (jFile.json_converted->is_null()) {
				LOG_INFO("Json(?) parsing failed.", jFile.file_path);
				return false;
			}

			if (!jFile.CheckRequiredFiles()) {
				LOG_INFO("That Json(?) has some missing fields.", jFile.file_path);
				return false;
			}

			return true;
		}
		catch (const std::exception& e)
		{
			LOG_ERR("JSON reading failed: " + std::string(e.what()));
			return false;
		}
	}
}
