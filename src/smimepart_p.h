#ifndef SMIMEPART_P_H
#define SMIMEPART_P_H

#include "mimepart_p.h"

#include <openssl/bio.h>
#include <openssl/pkcs12.h>

template <>
class std::default_delete<BIO>
{
public:
    void operator()(BIO *ptr) const { BIO_free(ptr); }
};

template <>
class std::default_delete<EVP_PKEY>
{
public:
    void operator()(EVP_PKEY *ptr) const { EVP_PKEY_free(ptr); }
};

template <>
class std::default_delete<X509>
{
public:
    void operator()(X509 *ptr) const { X509_free(ptr); }
};

template <>
class std::default_delete<STACK_OF(X509)>
{
public:
    void operator()(STACK_OF(X509) * ptr) const { sk_X509_pop_free(ptr, X509_free); }
};

namespace SimpleMail {
class SMimePartPrivate : public MimePartPrivate
{
public:
    virtual ~SMimePartPrivate();

    QByteArray _message;
    QString _keyfile;
    QString _publicKeyfile;
    QString _password;

    // private key for signing
    std::unique_ptr<EVP_PKEY> _privateKey = nullptr;

    // certificate from private Key
    std::unique_ptr<X509> _certificate             = nullptr;
    std::unique_ptr<STACK_OF(X509)> _certificateCA = nullptr;
    // certificate from public key, used for encrypting
    std::unique_ptr<STACK_OF(X509)> _recipsReceiver = nullptr;

    // buffer to be signed/encrypted
    std::unique_ptr<BIO> _input;
};

} // namespace SimpleMail
#endif // SMIMEPART_P_H
