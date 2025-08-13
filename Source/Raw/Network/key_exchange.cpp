#include "key_exchange.h"
#include <openssl_compat.h>

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/kdf.h>

#include <Singletons/log_manager.h>
namespace network {
    KeyExchange::~KeyExchange() noexcept = default;

    KeyExchange::KeyExchange()
        : key_ctx(nullptr, EVP_PKEY_CTX_free)
        , private_key(nullptr, EVP_PKEY_free) {

        OPENSSL_CLEAR_ERRORS();

        // X25519 parametrelerini oluştur - cross-version compatible
        auto* params_ctx = openssl_compat::create_x25519_context();
        if (!params_ctx) {
            LOG_FATAL("Failed to create X25519 params context: ?", OPENSSL_GET_ERROR());
            return;
        }

        // Key generation context'ini oluştur
        if (!openssl_compat::pkey_keygen_init(params_ctx)) {
            EVP_PKEY_CTX_free(params_ctx);
            LOG_FATAL("Failed to init keygen context: ?", OPENSSL_GET_ERROR());
            return;
        }

        // Key pair oluştur
        EVP_PKEY* pkey = nullptr;
        if (!openssl_compat::pkey_keygen(params_ctx, &pkey)) {
            EVP_PKEY_CTX_free(params_ctx);
            LOG_FATAL("Failed to generate keypair: ?", OPENSSL_GET_ERROR());
            return;
        }

        private_key.reset(pkey);
        EVP_PKEY_CTX_free(params_ctx);

        // Public key'i export et
        size_t pubkey_len = 32;  // X25519 public key size
        m_public_key.resize(pubkey_len);

        if (EVP_PKEY_get_raw_public_key(private_key.get(), m_public_key.data(), &pubkey_len) <= 0) {
            LOG_FATAL("Failed to export public key: ?", OPENSSL_GET_ERROR());
            return;
        }

        //LOG_TRACE("KeyExchange initialized successfully with ? version", OPENSSL_VERSION_INFO());
    }

    bool KeyExchange::computeSharedKey(const std::vector<uint8_t>& peer_public_key, std::vector<uint8_t>& _computed) {
        OPENSSL_CLEAR_ERRORS();

        // Log peer public key size
        LOG_INFO("Received peer public key size: ?", peer_public_key.size());

        // Peer public key'i import et
        auto* peer_key = EVP_PKEY_new_raw_public_key(EVP_PKEY_X25519, nullptr,
            peer_public_key.data(), peer_public_key.size());
        if (!peer_key) {
            LOG_FATAL("Failed to import peer public key: ?", OPENSSL_GET_ERROR());
            return false;
        }

        // Derive context olustur
        key_ctx.reset(EVP_PKEY_CTX_new(private_key.get(), nullptr));
        if (!key_ctx) {
            EVP_PKEY_free(peer_key);
            LOG_FATAL("Failed to create derive context: ?", OPENSSL_GET_ERROR());
            return false;
        }

        // Shared secret hesapla - compat functions kullan
        if (!openssl_compat::pkey_derive_init(key_ctx.get())) {
            EVP_PKEY_free(peer_key);
            LOG_FATAL("Failed to init derive operation: ?", OPENSSL_GET_ERROR());
            return false;
        }

        if (!openssl_compat::pkey_derive_set_peer(key_ctx.get(), peer_key)) {
            EVP_PKEY_free(peer_key);
            LOG_FATAL("Failed to set peer key: ?", OPENSSL_GET_ERROR());
            return false;
        }

        size_t shared_secret_len = 0;
        if (!openssl_compat::pkey_derive(key_ctx.get(), nullptr, &shared_secret_len)) {
            EVP_PKEY_free(peer_key);
            LOG_FATAL("Failed to determine shared secret length: ?", OPENSSL_GET_ERROR());
            return false;
        }

        LOG_INFO("Expected shared secret length: ?", shared_secret_len);

        std::vector<uint8_t> shared_secret(shared_secret_len);
        if (!openssl_compat::pkey_derive(key_ctx.get(), shared_secret.data(), &shared_secret_len)) {
            EVP_PKEY_free(peer_key);
            LOG_FATAL("Failed to derive shared secret: ?", OPENSSL_GET_ERROR());
            return false;
        }

        LOG_INFO("Successfully derived shared secret of length: ?", shared_secret_len);

        EVP_PKEY_free(peer_key);

        // Shared secret'tan AES anahtarı türet
        if (!deriveAESKey(shared_secret, _computed)) {
            LOG_ERR("Failed to derive AES key from shared secret");
            return false;
        }
        
        LOG_INFO("Derived AES key size: ?", _computed.size());
        return true;
    }

    bool KeyExchange::deriveAESKey(const std::vector<uint8_t>& shared_secret, std::vector<uint8_t>& _computed) {
        OPENSSL_CLEAR_ERRORS();

        std::vector<uint8_t> derived_key(32); // AES-256 için 32 byte

        LOG_INFO("Starting HKDF key derivation with shared secret size: ?", shared_secret.size());

        // HKDF parametreleri
        const unsigned char salt[] = "CryptoHelperSalt";
        const unsigned char info[] = "AES-256-Key";

        // Cross-version HKDF using EVP_KDF - compatible with OpenSSL 1.1.1 and 3.x
        if (!OPENSSL_HKDF_DERIVE(salt, sizeof(salt) - 1,
                               shared_secret.data(), shared_secret.size(),
                               info, sizeof(info) - 1,
                               derived_key.data(), derived_key.size())) {
            LOG_FATAL("HKDF operation failed: ?", OPENSSL_GET_ERROR());
            return false;
        }

        LOG_INFO("Successfully derived AES key with HKDF, key size: ?", derived_key.size());

        _computed = derived_key;
        return true;
    }
}
