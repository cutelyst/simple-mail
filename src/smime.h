#ifndef SMIMEPART_H
#define SMIMEPART_H

#include "mimemessage.h"
#include "mimepart.h"

#include <openssl/pkcs12.h>

#include <QString>

namespace SimpleMail {
class SMTP_EXPORT SMimePart : public MimePart
{
public:
    SMimePart(MimeMessage *message);
    virtual ~SMimePart();

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
};
} // namespace SimpleMail

#endif // SMIME_H
