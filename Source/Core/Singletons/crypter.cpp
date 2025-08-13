#include "crypter.h"

#include <vector>
#include <cstdint>
#include <string>
#include <cstring>

#include <openssl_compat.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/opensslv.h>


#include "log_manager.h"

namespace NSingletons {

	CCrypter::CCrypter() : m_enc_ctx(EVP_CIPHER_CTX_new()), m_dec_ctx(EVP_CIPHER_CTX_new()) {
		// Cross-version OpenSSL initialization
		OPENSSL_INITIALIZE();
	}

	CCrypter::~CCrypter() {
		// Cleanup OpenSSL contexts
		EVP_CIPHER_CTX_free(m_enc_ctx);
		EVP_CIPHER_CTX_free(m_dec_ctx);
		
		// Cross-version OpenSSL cleanup
		OPENSSL_CLEANUP();
	}

	std::vector<uint8_t> CCrypter::GenerateKey(const std::string& password) const noexcept {
		std::vector<uint8_t> key(32); // 256 bit key

		if (!EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha256(), nullptr,
			reinterpret_cast<const uint8_t*>(password.c_str()),
			static_cast<int>(password.length()), 1, key.data(), nullptr)) {
			LOG_FATAL("Failed to generate key");
		}
		return key;
	}

	bool CCrypter::DecryptData(std::string& encrypted_data, const std::string& password) const noexcept {
		std::lock_guard<std::mutex> mute(m_dec_mutex);
		if (encrypted_data.length() < AES_BLOCK_SIZE) {
			LOG_FATAL("Encrypted data too short");
			return false;
		}

		LOG_TRACE("Decrypting file - total size: ? bytes", encrypted_data.length());

		// İlk 16 byte IV'dir
		std::vector<uint8_t> iv(encrypted_data.begin(), encrypted_data.begin() + AES_BLOCK_SIZE);
		auto key = GenerateKey(password);

		// Debug: IV ve ilk birkaç byte'ı göster
		std::string iv_hex;
		for (size_t i = 0; i < std::min<size_t>(16, iv.size()); ++i) {
			char hex[3];
			sprintf(hex, "%02X", iv[i]);
			iv_hex += hex;
		}
		LOG_TRACE("Extracted IV (16 bytes): ?", iv_hex);

		std::string data_preview;
		size_t preview_size = std::min<size_t>(32, encrypted_data.length() - AES_BLOCK_SIZE);
		for (size_t i = AES_BLOCK_SIZE; i < AES_BLOCK_SIZE + preview_size; ++i) {
			char hex[3];
			sprintf(hex, "%02X", static_cast<uint8_t>(encrypted_data[i]));
			data_preview += hex;
		}
		LOG_TRACE("Encrypted data preview (? bytes): ?", preview_size, data_preview);

		// Context'i her decrypt işleminden önce temizle/reset et - cross-version compatible
		OPENSSL_RESET_CTX(m_dec_ctx);

		if (EVP_DecryptInit_ex(m_dec_ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
			LOG_FATAL("Failed to initialize decryption: ?", OPENSSL_GET_ERROR());
			return false;
		}

		// Decryption buffer hazırla
		std::vector<uint8_t> decrypted;
		decrypted.resize(encrypted_data.length() - AES_BLOCK_SIZE + AES_BLOCK_SIZE);

		int len = 0;
		const unsigned char* input_data = reinterpret_cast<const unsigned char*>(encrypted_data.data() + AES_BLOCK_SIZE);
		int input_len = static_cast<int>(encrypted_data.length() - AES_BLOCK_SIZE);

		if (EVP_DecryptUpdate(m_dec_ctx, decrypted.data(), &len, input_data, input_len) != 1) {
			LOG_FATAL("Failed during DecryptUpdate: ?", OPENSSL_GET_ERROR());
			return false;
		}

		int final_len = 0;
		if (EVP_DecryptFinal_ex(m_dec_ctx, decrypted.data() + len, &final_len) != 1) {
			LOG_FATAL("Failed during DecryptFinal: ?", OPENSSL_GET_ERROR());
			return false;
		}

		// Başarılı decryption
		int total_decrypted_len = len + final_len;
		LOG_TRACE("Decryption successful - decrypted size: ? bytes", total_decrypted_len);

		encrypted_data.assign(reinterpret_cast<const char*>(decrypted.data()), total_decrypted_len);
		return true;
	}

	std::string CCrypter::EncryptData(const std::string& data, const char* password) const noexcept {
		std::lock_guard<std::mutex> mute(m_enc_mutex);

		if (data.empty() || !password) {
			LOG_FATAL("Invalid input data or password");
			return "";
		}

		LOG_TRACE("Encrypting data - input size: ? bytes", data.length());

		auto key = GenerateKey(std::string(password));

		// Random IV oluştur - cross-version compatible
		std::vector<uint8_t> iv(AES_BLOCK_SIZE);
		if (!OPENSSL_RAND_BYTES(iv.data(), AES_BLOCK_SIZE)) {
			LOG_FATAL("Failed to generate IV: ?", OPENSSL_GET_ERROR());
			return "";
		}

		// Debug: IV'yi göster
		std::string iv_hex;
		for (size_t i = 0; i < iv.size(); ++i) {
			char hex[3];
			sprintf(hex, "%02X", iv[i]);
			iv_hex += hex;
		}
		LOG_TRACE("Generated IV (16 bytes): ?", iv_hex);

		// Context'i reset et - cross-version compatible
		OPENSSL_RESET_CTX(m_enc_ctx);

		if (EVP_EncryptInit_ex(m_enc_ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
			LOG_FATAL("Failed to initialize encryption: ?", OPENSSL_GET_ERROR());
			return "";
		}

		// Encryption buffer hazırla
		std::vector<uint8_t> encrypted;
		encrypted.resize(data.length() + AES_BLOCK_SIZE * 2); // Extra space for padding

		int len = 0;
		const unsigned char* input_data = reinterpret_cast<const unsigned char*>(data.data());
		int input_len = static_cast<int>(data.length());

		if (EVP_EncryptUpdate(m_enc_ctx, encrypted.data(), &len, input_data, input_len) != 1) {
			LOG_FATAL("Failed during EncryptUpdate: ?", OPENSSL_GET_ERROR());
			return "";
		}

		int final_len = 0;
		if (EVP_EncryptFinal_ex(m_enc_ctx, encrypted.data() + len, &final_len) != 1) {
			LOG_FATAL("Failed during EncryptFinal: ?", OPENSSL_GET_ERROR());
			return "";
		}

		int total_encrypted_len = len + final_len;
		LOG_TRACE("Encryption successful - encrypted size: ? bytes", total_encrypted_len);

		// Final result: IV + encrypted data
		std::string result;
		result.reserve(AES_BLOCK_SIZE + total_encrypted_len);
		
		// IV'yi ekle
		result.append(reinterpret_cast<const char*>(iv.data()), AES_BLOCK_SIZE);
		// Encrypted data'yı ekle
		result.append(reinterpret_cast<const char*>(encrypted.data()), total_encrypted_len);

		LOG_TRACE("Final encrypted result size: ? bytes", result.length());
		return result;
	}
}