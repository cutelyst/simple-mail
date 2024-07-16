#ifndef SMIME_H
#define SMIME_H

#include "mimemessage.h"
#include "mimepart.h"

#include <openssl/pkcs12.h>

#include <QString>

namespace SimpleMail {
class SMTP_EXPORT SMime : public MimePart
{
public:
    SMime(MimeMessage *message);
    virtual ~SMime();

    void setKeyFile(const QString &filename, const QString &password);

    void setPublicKey(const QString &filename);

    bool sign();
    bool encrypt();
    bool signAndEncrypt();

private:
    void setSignedHeader();
    void setEncryptionHeader();

    void loadPKCS12PrivateKey();
    void loadPKCS12PublicKey();

    void initOpenSSL();

    void wrapMimeMultiPart();
    bool writeInputBuffer();
    bool writeMimeMessageBuffer();
    bool handleData(PKCS7 *p7, BIO *dataIn, int flags);

    MimeMessage *_mimeMessage;

    QByteArray _message;
    QString _keyfile;
    QString _publicKeyfile;
    QString _password;

    // private key for signing
    EVP_PKEY *_privateKey;

    // certificate from private Key
    X509 *_certificate;
    STACK_OF(X509) * _certificateCA;
    // certificate from public key, used for encrypting
    STACK_OF(X509) * _recipsReceiver;
    // buffer to be signed/encrypted
    BIO *_input;
};
} // namespace SimpleMail

#endif // SMIME_H
