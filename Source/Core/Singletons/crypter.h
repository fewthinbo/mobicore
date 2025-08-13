#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <mutex>

#include <singleton.h>

#include <openssl/evp.h>

namespace NSingletons {

	class CCrypter : public CSingleton<CCrypter> {
		std::vector<uint8_t> GenerateKey(const std::string& password) const noexcept;
		EVP_CIPHER_CTX* m_enc_ctx;
		EVP_CIPHER_CTX* m_dec_ctx;
		mutable std::mutex m_enc_mutex, m_dec_mutex;
	public:
		CCrypter();
		~CCrypter();
	public:
		bool DecryptData(std::string& encrypted_data, const std::string& password) const noexcept;
		std::string EncryptData(const std::string& data, const char* password) const noexcept;
	};
}

#define cryInstance NSingletons::CCrypter::getInstance()