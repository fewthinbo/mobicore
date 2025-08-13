#pragma once

#include <vector>
#include <string>

#include <openssl/evp.h>

namespace network {
    class CryptoHelper final {
    public:
        static constexpr size_t KEY_SIZE = 32;  // 256 bit
        static constexpr size_t IV_SIZE = 16;   // 128 bit
        static constexpr size_t BLOCK_SIZE = 16; // AES block size

        CryptoHelper();
        ~CryptoHelper();

        bool initialize(const std::vector<uint8_t>& key);
        bool encrypt(std::vector<uint8_t>& data);
        bool decrypt(std::vector<uint8_t>& data);

        static std::vector<uint8_t> generateKey();
    private:
        EVP_CIPHER_CTX* enc_ctx;
        EVP_CIPHER_CTX* dec_ctx;
        std::vector<uint8_t> key;
        std::vector<uint8_t> iv;
    };
}
