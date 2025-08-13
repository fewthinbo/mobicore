// crypto_helper.cpp
#include "crypto_helper.h"
#include "openssl_compat.h"

#include <openssl/err.h>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
//#define CRYPTO_DEBUG

#include <Singletons/log_manager.h>
#include "common.h"

namespace network {
    CryptoHelper::CryptoHelper()
        : enc_ctx(EVP_CIPHER_CTX_new())
        , dec_ctx(EVP_CIPHER_CTX_new()) {
    }

    CryptoHelper::~CryptoHelper() {
        EVP_CIPHER_CTX_free(enc_ctx);
        EVP_CIPHER_CTX_free(dec_ctx);
    }

    bool CryptoHelper::initialize(const std::vector<uint8_t>& new_key) {
        if (new_key.size() != KEY_SIZE) return false;

        key = new_key;
        iv.resize(IV_SIZE);

        if (RAND_bytes(iv.data(), IV_SIZE) != 1) return false;

        // Her initialize'da context'leri yenile - cross-version compatible
        OPENSSL_RESET_CTX(enc_ctx);
        OPENSSL_RESET_CTX(dec_ctx);

        if (EVP_EncryptInit_ex(enc_ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) return false;
        if (EVP_DecryptInit_ex(dec_ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) return false;
        //EVP_CIPHER_CTX_set_padding(enc_ctx, 0);
        //EVP_CIPHER_CTX_set_padding(dec_ctx, 0); // Padding'i kapat
        return true;
    }

    bool CryptoHelper::encrypt(std::vector<uint8_t>& data) {
        if (data.empty()) return false;

#if defined(_DEBUG) && defined(CRYPTO_DEBUG)
        printf("[ENCRYPT] Input data size: %zu\n", data.size());
        printf("[ENCRYPT] Original data bytes: ");
        for (size_t i = 0; i < data.size(); i++) {
            printf("%02X ", data[i]);
        }
        printf("\n");
#endif

        // Her şifreleme işlemi için yeni bir IV oluştur - cross-version compatible
        std::vector<uint8_t> current_iv(IV_SIZE);
        if (!OPENSSL_RAND_BYTES(current_iv.data(), IV_SIZE)) {
#if defined(_DEBUG) && defined(CRYPTO_DEBUG)
            printf("[ENCRYPT] Failed to generate IV: %s\n", OPENSSL_GET_ERROR().c_str());
#endif
            return false;
        }

#if defined(_DEBUG) && defined(CRYPTO_DEBUG)
        printf("[ENCRYPT] Generated IV (%zu bytes): ", IV_SIZE);
        for (size_t i = 0; i < IV_SIZE; i++) {
            printf("%02X ", current_iv[i]);
        }
        printf("\n");
#endif

        // Şifrelenecek veriyi hazırla
        std::vector<uint8_t> encrypted;
        encrypted.resize(data.size() + EVP_MAX_BLOCK_LENGTH * 2); // Doubled buffer size for safety

        // Encryption context'i yeni IV ile güncelle
        if (EVP_EncryptInit_ex(enc_ctx, EVP_aes_256_cbc(), nullptr, key.data(), current_iv.data()) != 1) {
#if defined(_DEBUG) && defined(CRYPTO_DEBUG)
            printf("[ENCRYPT] Failed to initialize encryption context: %s\n", OPENSSL_GET_ERROR().c_str());
#endif
            return false;
        }

        int len = 0;
        // Veriyi şifrele
        if (EVP_EncryptUpdate(enc_ctx, encrypted.data(), &len,
            data.data(), data.size()) != 1) {
#if defined(_DEBUG) && defined(CRYPTO_DEBUG)
            printf("[ENCRYPT] Failed during EncryptUpdate: %s\n", OPENSSL_GET_ERROR().c_str());
#endif
            return false;
        }

#if defined(_DEBUG) && defined(CRYPTO_DEBUG)
        printf("[ENCRYPT] After EncryptUpdate len: %d\n", len);
        printf("[ENCRYPT] Partial encrypted data: ");
        for (int i = 0; i < len; i++) {
            printf("%02X ", encrypted[i]);
        }
        printf("\n");
#endif

        int final_len = 0;
        // Şifrelemeyi tamamla ve padding ekle
        if (EVP_EncryptFinal_ex(enc_ctx, encrypted.data() + len, &final_len) != 1) {
#if defined(_DEBUG) && defined(CRYPTO_DEBUG)
            printf("[ENCRYPT] Failed during EncryptFinal: %s (len=%d, data_size=%zu)\n",
                OPENSSL_GET_ERROR().c_str(), len, data.size());
#endif
            // Error recovery - cross-version compatible
            OPENSSL_RESET_CTX(enc_ctx);
            EVP_EncryptInit_ex(enc_ctx, EVP_aes_256_cbc(), nullptr, key.data(), current_iv.data()); // Reinitialize
            return false;
        }

#if defined(_DEBUG) && defined(CRYPTO_DEBUG)
        printf("[ENCRYPT] After EncryptFinal final_len: %d\n", final_len);
        printf("[ENCRYPT] Final block encrypted data: ");
        for (int i = len; i < len + final_len; i++) {
            printf("%02X ", encrypted[i]);
        }
        printf("\n");
#endif

        // Gerçek boyuta küçült
        encrypted.resize(len + final_len);

        // Final paket formatı: [SIZE][IV][ENCRYPTED_DATA]
        TSIZE total_size = IV_SIZE + encrypted.size();

#if defined(_DEBUG) && defined(CRYPTO_DEBUG)
        printf("[ENCRYPT] Encrypted size: %zu, Total size with IV: %u\n", encrypted.size(), total_size);
#endif

        std::vector<uint8_t> final_packet;
        final_packet.reserve(SIZE_SIZE + total_size);

        // Boyut bilgisini ekle
        final_packet.resize(SIZE_SIZE);
        std::memcpy(final_packet.data(), &total_size, SIZE_SIZE);

#if defined(_DEBUG) && defined(CRYPTO_DEBUG)
        printf("[ENCRYPT] Size bytes: [%02X %02X]\n",
            final_packet[0], final_packet[1]);
#endif

        // IV'yi ekle
        final_packet.insert(final_packet.end(), current_iv.begin(), current_iv.end());

        // Şifrelenmiş veriyi ekle
        final_packet.insert(final_packet.end(), encrypted.begin(), encrypted.end());

#if defined(_DEBUG) && defined(CRYPTO_DEBUG)
        printf("[ENCRYPT] Final packet (%zu bytes):\n", final_packet.size());
        printf("Size  [2]: ");
        for (size_t i = 0; i < SIZE_SIZE; i++) printf("%02X ", final_packet[i]);
        printf("\nIV   [16]: ");
        for (size_t i = SIZE_SIZE; i < SIZE_SIZE + IV_SIZE; i++) printf("%02X ", final_packet[i]);
        printf("\nData [%zu]: ", encrypted.size());
        for (size_t i = SIZE_SIZE + IV_SIZE; i < final_packet.size(); i++) printf("%02X ", final_packet[i]);
        printf("\n");
#endif

        data = std::move(final_packet);
        return true;
    }

    bool CryptoHelper::decrypt(std::vector<uint8_t>& data) {
        // Data should be at least IV size + some encrypted data
        if (data.size() <= IV_SIZE) {
#if defined(_DEBUG) && defined(CRYPTO_DEBUG)
            printf("[DECRYPT] Data too small, size: %zu, minimum needed: %zu\n",
                data.size(), IV_SIZE);
#endif
            return false;
        }

        // Extract IV from the beginning of data
        std::vector<uint8_t> received_iv(data.begin(), data.begin() + IV_SIZE);
        std::vector<uint8_t> encrypted_data(data.begin() + IV_SIZE, data.end());

#if defined(_DEBUG) && defined(CRYPTO_DEBUG)
        printf("[DECRYPT] Extracted IV (%zu bytes): ", received_iv.size());
        for (size_t i = 0; i < IV_SIZE; i++) {
            printf("%02X ", received_iv[i]);
        }
        printf("\n[DECRYPT] Encrypted data (%zu bytes): ", encrypted_data.size());
        for (size_t i = 0; i < encrypted_data.size(); i++) {
            printf("%02X ", encrypted_data[i]);
        }
        printf("\n");
#endif

        // Initialize decryption context with received IV
        if (EVP_DecryptInit_ex(dec_ctx, EVP_aes_256_cbc(), nullptr, key.data(), received_iv.data()) != 1) {
#if defined(_DEBUG) && defined(CRYPTO_DEBUG)
            printf("[DECRYPT] Failed to initialize decryption context\n");
#endif
            return false;
        }

        // Prepare buffer for decrypted data
        std::vector<uint8_t> decrypted;
        decrypted.resize(encrypted_data.size() + EVP_MAX_BLOCK_LENGTH);

        int len = 0;
        // Decrypt data
        if (EVP_DecryptUpdate(dec_ctx, decrypted.data(), &len,
            encrypted_data.data(), encrypted_data.size()) != 1) {
#if defined(_DEBUG) && defined(CRYPTO_DEBUG)
            printf("[DECRYPT] Failed during DecryptUpdate\n");
#endif
            return false;
        }

#if defined(_DEBUG) && defined(CRYPTO_DEBUG)
        printf("[DECRYPT] After DecryptUpdate len: %d\n", len);
        printf("[DECRYPT] Partial decrypted data: ");
        for (int i = 0; i < len; i++) {
            printf("%02X ", decrypted[i]);
        }
        printf("\n");
#endif

        int final_len = 0;
        // Finalize decryption and remove padding
        if (EVP_DecryptFinal_ex(dec_ctx, decrypted.data() + len, &final_len) != 1) {
#if defined(_DEBUG) && defined(CRYPTO_DEBUG)
            unsigned long err = ERR_get_error();
            char err_msg[256];
            ERR_error_string_n(err, err_msg, sizeof(err_msg));
            printf("[DECRYPT] Failed during DecryptFinal: %s\n", err_msg);
#endif
            return false;
        }

#if defined(_DEBUG) && defined(CRYPTO_DEBUG)
        printf("[DECRYPT] After DecryptFinal final_len: %d\n", final_len);
        printf("[DECRYPT] Final block decrypted data: ");
        for (int i = len; i < len + final_len; i++) {
            printf("%02X ", decrypted[i]);
        }
        printf("\n");
#endif

        // Resize to actual decrypted size
        decrypted.resize(len + final_len);

#if defined(_DEBUG) && defined(CRYPTO_DEBUG)
        printf("[DECRYPT] Final decrypted data (%zu bytes): ", decrypted.size());
        for (size_t i = 0; i < decrypted.size(); i++) {
            printf("%02X ", decrypted[i]);
        }
        printf("\n");
#endif

        data = std::move(decrypted);
        return true;
    }

    std::vector<uint8_t> CryptoHelper::generateKey() {
        std::vector<uint8_t> key(KEY_SIZE);
        if (!OPENSSL_RAND_BYTES(key.data(), KEY_SIZE)) {
            LOG_ERR("Failed to generate random key: ?", OPENSSL_GET_ERROR());
            // Return empty key on failure
            return std::vector<uint8_t>{};
        }
        return key;
    }
}

