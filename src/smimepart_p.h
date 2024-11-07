#ifndef SMIMEPART_P_H
#define SMIMEPART_P_H

#include "mimepart_p.h"
#include <openssl/pkcs12.h>

namespace SimpleMail {

class SMimePrivate : public MimePartPrivate
{
public:
    virtual ~SMimePrivate();

    QByteArray _message;
    QString _keyfile;
    QString _publicKeyfile;
    QString _password;

    // private key for signing
    EVP_PKEY *_privateKey = nullptr;

    // certificate from private Key
    X509 *_certificate = nullptr;
    STACK_OF(X509) * _certificateCA = nullptr;
    // certificate from public key, used for encrypting
    STACK_OF(X509) * _recipsReceiver = nullptr;
    // buffer to be signed/encrypted
    BIO *_input = nullptr;
};

} // namespace SimpleMail

#endif // SMIMEPART_P_H
