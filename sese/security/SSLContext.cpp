#include <openssl/ssl.h>

#include <sese/security/SSLContext.h>

using namespace sese::security;

SSLContext::SSLContext(const void *method) noexcept {
    context = SSL_CTX_new((const SSL_METHOD *) method);
}

SSLContext::~SSLContext() noexcept {
    SSL_CTX_free((SSL_CTX *) context);
}

void *SSLContext::getContext() const noexcept {
    return context;
}

bool SSLContext::importCertFile(const char *file) noexcept {
    return 1 == SSL_CTX_use_certificate_file((SSL_CTX *) context, file, SSL_FILETYPE_PEM);
}

bool SSLContext::importPrivateKeyFile(const char *file) noexcept {
    return 1 == SSL_CTX_use_PrivateKey_file((SSL_CTX *) context, file, SSL_FILETYPE_PEM);
}

bool SSLContext::authPrivateKey() noexcept {
    return 1 == SSL_CTX_check_private_key((SSL_CTX *) context);
}

// bool SSLContext::verifyAndLoad(const char *file) noexcept {
//     SSL_CTX_set_verify((SSL_CTX *) context, SSL_VERIFY_PEER, nullptr);
//     return 1 == SSL_CTX_load_verify_locations((SSL_CTX *) context, file, nullptr);
// }

sese::net::Socket::Ptr SSLContext::newSocketPtr(Socket::Family family, int32_t flags) {
    return std::make_shared<SecuritySocket>(shared_from_this(), family, flags);
}