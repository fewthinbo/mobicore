#pragma once
#include <string>
#include <vector>
#include <memory>

#include <openssl/opensslv.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/hmac.h>

// OpenSSL 1.1.1+ için KDF desteği
#if OPENSSL_VERSION_NUMBER >= 0x10101000L
#include <openssl/kdf.h>
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
// OpenSSL 3.x için yeni API
#include <openssl/core_names.h>
#include <openssl/params.h>
#endif
#endif

// OpenSSL Version Compatibility Layer
// Ensures code works with both OpenSSL 1.1.x and 3.x

namespace openssl_compat {
    
// Version detection
constexpr bool is_openssl_1_1() {
    return OPENSSL_VERSION_NUMBER >= 0x10101000L && OPENSSL_VERSION_NUMBER < 0x30000000L;
}

constexpr bool is_openssl_3_x() {
    return OPENSSL_VERSION_NUMBER >= 0x30000000L;
}

// Context management compatibility
inline void reset_cipher_context(EVP_CIPHER_CTX* ctx) {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    // OpenSSL 1.0.x: Use cleanup
    EVP_CIPHER_CTX_cleanup(ctx);
#else
    // OpenSSL 1.1.0+, 1.1.1+, 3.x+: All use reset
    EVP_CIPHER_CTX_reset(ctx);
#endif
}

// Error handling compatibility
inline std::string get_openssl_error() {
    unsigned long err = ERR_get_error();
    if (err == 0) return "No OpenSSL error";
    
    char err_msg[256];
    ERR_error_string_n(err, err_msg, sizeof(err_msg));
    return std::string(err_msg);
}

inline void clear_openssl_errors() {
    while (ERR_get_error() != 0) {
        // Clear error queue
    }
}

// Initialization compatibility
inline void initialize_openssl() {
    // Only OpenSSL 1.0.x requires explicit initialization
    // OpenSSL 1.1.1+ initializes automatically and these calls are no-ops or harmful
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();
#endif
    // OpenSSL 1.1.1+ and 3.x: Do nothing, automatic initialization
}

// Cleanup compatibility  
inline void cleanup_openssl() {
    // Only OpenSSL 1.0.x requires explicit cleanup
    // OpenSSL 1.1.1+ cleans up automatically and these calls are no-ops or harmful
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    EVP_cleanup();
    ERR_free_strings();
    CRYPTO_cleanup_all_ex_data();
#endif
    // OpenSSL 1.1.1+ and 3.x: Do nothing, automatic cleanup
}

// Key Exchange specific compatibility
inline EVP_PKEY_CTX* create_x25519_context() {
    return EVP_PKEY_CTX_new_id(EVP_PKEY_X25519, nullptr);
}

// PKEY operations with error logging
inline bool pkey_keygen_init(EVP_PKEY_CTX* ctx) {
    int ret = EVP_PKEY_keygen_init(ctx);
    return ret > 0;
}

inline bool pkey_keygen(EVP_PKEY_CTX* ctx, EVP_PKEY** pkey) {
    int ret = EVP_PKEY_keygen(ctx, pkey);
    return ret > 0;
}

inline bool pkey_derive_init(EVP_PKEY_CTX* ctx) {
    int ret = EVP_PKEY_derive_init(ctx);
    return ret > 0;
}

inline bool pkey_derive_set_peer(EVP_PKEY_CTX* ctx, EVP_PKEY* peer) {
    int ret = EVP_PKEY_derive_set_peer(ctx, peer);
    return ret > 0;
}

inline bool pkey_derive(EVP_PKEY_CTX* ctx, unsigned char* key, size_t* keylen) {
    int ret = EVP_PKEY_derive(ctx, key, keylen);
    return ret > 0;
}

// Manual HKDF implementation for OpenSSL 1.1.1 compatibility
#if OPENSSL_VERSION_NUMBER >= 0x10101000L && OPENSSL_VERSION_NUMBER < 0x30000000L
// HKDF-Extract: PRK = HMAC-Hash(salt, IKM)
inline bool hkdf_extract(const unsigned char* salt, size_t salt_len,
                        const unsigned char* ikm, size_t ikm_len,
                        unsigned char* prk, unsigned int* prk_len) {
    return HMAC(EVP_sha256(), salt, salt_len, ikm, ikm_len, prk, prk_len) != nullptr;
}

// HKDF-Expand: OKM = HMAC-Hash(PRK, info || 0x01)
inline bool hkdf_expand(const unsigned char* prk, size_t prk_len,
                       const unsigned char* info, size_t info_len,
                       unsigned char* okm, size_t okm_len) {
    if (okm_len > 255 * EVP_MD_size(EVP_sha256())) {
        return false; // Output too long
    }
    
    unsigned char counter = 1;
    size_t done = 0;
    
    while (done < okm_len) {
        // Create T(counter) = HMAC-Hash(PRK, info || counter)
        std::vector<unsigned char> t_input;
        t_input.reserve(info_len + 1);
        t_input.insert(t_input.end(), info, info + info_len);
        t_input.push_back(counter);
        
        unsigned char t_output[32]; // SHA256 hash size
        unsigned int t_len;
        
        if (!HMAC(EVP_sha256(), prk, prk_len, t_input.data(), t_input.size(), t_output, &t_len)) {
            return false;
        }
        
        // Copy needed bytes to output
        size_t to_copy = std::min(static_cast<size_t>(t_len), okm_len - done);
        memcpy(okm + done, t_output, to_copy);
        done += to_copy;
        counter++;
    }
    
    return true;
}
#endif

// Cross-version HKDF implementation
inline bool hkdf_derive(const unsigned char* salt, size_t salt_len,
                       const unsigned char* key, size_t key_len,
                       const unsigned char* info, size_t info_len,
                       unsigned char* output, size_t output_len) {
#if OPENSSL_VERSION_NUMBER < 0x10101000L
    // OpenSSL 1.1.0 and earlier: HKDF not available
    (void)salt; (void)salt_len; (void)key; (void)key_len; 
    (void)info; (void)info_len; (void)output; (void)output_len;
    return false;
#elif OPENSSL_VERSION_NUMBER >= 0x30000000L
    // OpenSSL 3.x: Use new EVP_KDF API with OSSL_PARAM
    
    // RAII wrapper for EVP_KDF
    struct KDFDeleter {
        void operator()(EVP_KDF* kdf) const { if (kdf) EVP_KDF_free(kdf); }
    };
    struct KDFCtxDeleter {
        void operator()(EVP_KDF_CTX* ctx) const { if (ctx) EVP_KDF_CTX_free(ctx); }
    };
    
    std::unique_ptr<EVP_KDF, KDFDeleter> kdf_ptr;
    std::unique_ptr<EVP_KDF_CTX, KDFCtxDeleter> ctx_ptr;

    // Fetch HKDF KDF (OpenSSL 3.x only)
    kdf_ptr.reset(EVP_KDF_fetch(nullptr, "HKDF", nullptr));
    if (!kdf_ptr) {
        return false;
    }

    ctx_ptr.reset(EVP_KDF_CTX_new(kdf_ptr.get()));
    if (!ctx_ptr) {
        return false;
    }

    // Setup parameters - OSSL_PARAM (OpenSSL 3.x)
    OSSL_PARAM params[5];
    params[0] = OSSL_PARAM_construct_utf8_string("digest", const_cast<char*>("SHA256"), 0);
    params[1] = OSSL_PARAM_construct_octet_string("salt", const_cast<void*>(static_cast<const void*>(salt)), salt_len);
    params[2] = OSSL_PARAM_construct_octet_string("key", const_cast<void*>(static_cast<const void*>(key)), key_len);
    params[3] = OSSL_PARAM_construct_octet_string("info", const_cast<void*>(static_cast<const void*>(info)), info_len);
    params[4] = OSSL_PARAM_construct_end();

    // EVP_KDF_derive (OpenSSL 3.x)
    return EVP_KDF_derive(ctx_ptr.get(), output, output_len, params) > 0;
#else
    // OpenSSL 1.1.1: Use manual HKDF implementation
    unsigned char prk[32]; // SHA256 hash size
    unsigned int prk_len;
    
    // Step 1: Extract
    if (!hkdf_extract(salt, salt_len, key, key_len, prk, &prk_len)) {
        return false;
    }
    
    // Step 2: Expand
    bool result = hkdf_expand(prk, prk_len, info, info_len, output, output_len);
    
    // Clear PRK from memory
    OPENSSL_cleanse(prk, sizeof(prk));
    
    return result;
#endif
}

// Random number generation
inline bool generate_random_bytes(unsigned char* buf, int num) {
    return RAND_bytes(buf, num) == 1;
}

} // namespace openssl_compat

// Convenience macros for easier migration
#define OPENSSL_RESET_CTX(ctx) openssl_compat::reset_cipher_context(ctx)
#define OPENSSL_INITIALIZE() openssl_compat::initialize_openssl()
#define OPENSSL_CLEANUP() openssl_compat::cleanup_openssl()
#define OPENSSL_GET_ERROR() openssl_compat::get_openssl_error()
#define OPENSSL_CLEAR_ERRORS() openssl_compat::clear_openssl_errors()
#define OPENSSL_RAND_BYTES(buf, num) openssl_compat::generate_random_bytes(buf, num)
#define OPENSSL_HKDF_DERIVE(salt, salt_len, key, key_len, info, info_len, output, output_len) \
    openssl_compat::hkdf_derive(salt, salt_len, key, key_len, info, info_len, output, output_len)

// Version info for debugging
#define OPENSSL_VERSION_INFO() \
    "OpenSSL Version: " OPENSSL_VERSION_TEXT " (Number: " STRINGIFY(OPENSSL_VERSION_NUMBER) ")"

#define STRINGIFY(x) STRINGIFY_(x)
#define STRINGIFY_(x) #x 