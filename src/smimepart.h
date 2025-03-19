#ifndef SMIMEPART_H
#define SMIMEPART_H

#include "mimemessage.h"
#include "mimepart.h"

#include <openssl/pkcs12.h>

#include <QString>

namespace SimpleMail {
class SMimePart : public MimePart
{
public:
    SMimePart();
    virtual ~SMimePart();

    void setKeyFile(const QString &filename, const QString &password);

    void setPublicKey(const QString &filename);

    bool sign();
    bool encrypt();
    bool signAndEncrypt();

    bool writeMimeMessageBuffer(const std::shared_ptr<SimpleMail::MimePart> &mimeParts);

private:
    void setSignedHeader();
    void setEncryptionHeader();

    void loadPKCS12PrivateKey();
    void loadPKCS12PublicKey();

    void initOpenSSL();

    bool writeInputBuffer();
    bool handleData(PKCS7 *p7, BIO *dataIn, int flags);
};
} // namespace SimpleMail

#endif // SMIME_H
