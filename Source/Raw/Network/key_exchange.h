// key_exchange.h
#pragma once

#include <vector>
#include <memory>

#include <openssl/evp.h>

namespace network {
    class KeyExchange final {
        std::unique_ptr<EVP_PKEY_CTX, void(*)(EVP_PKEY_CTX*)> key_ctx;
        std::unique_ptr<EVP_PKEY, void(*)(EVP_PKEY*)> private_key;
        std::vector<uint8_t> m_public_key;
    public:
        KeyExchange();
        ~KeyExchange() noexcept;

        // Public key: karsi tarafa gönderilecek
        const std::vector<uint8_t>& getPublicKey() const noexcept { return m_public_key; }

        // Karşı tarafın public key'i ile shared secret oluştur
        bool computeSharedKey(const std::vector<uint8_t>& peer_public_key, std::vector<uint8_t>& _computed);

        // HKDF ile shared secret'tan AES anahtarı türet
        bool deriveAESKey(const std::vector<uint8_t>& shared_secret, std::vector<uint8_t>& _computed);
    };

}
