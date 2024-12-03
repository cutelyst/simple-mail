#include "smime.h"

#include "mimemultipart.h"
#include "mimepart_p.h"
#include "smimepart_p.h"

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/provider.h>

#include <QBuffer>
#include <QFile>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(SIMPLEMAIL_SMIME, "simplemail.smime", QtInfoMsg)

using namespace SimpleMail;

SMime::SMime(MimeMessage *message)
    : MimePart(new SMimePrivate)
{
    initOpenSSL();
    _mimeMessage = message;
    writeMimeMessageBuffer();
}

SMime::~SMime()
{
    _mimeMessage = nullptr;
}

void SMime::setKeyFile(const QString &filename, const QString &password)
{
    SMimePrivate *d = static_cast<SMimePrivate*>(d_ptr.data());
    d->_keyfile  = filename;
    d->_password = password;
    loadPKCS12PrivateKey();
}

void SMime::setPublicKey(const QString &filename)
{
    SMimePrivate *d = static_cast<SMimePrivate*>(d_ptr.data());
    d->_publicKeyfile = filename;
    loadPKCS12PublicKey();
}

bool SMime::sign()
{
    SMimePrivate *d = static_cast<SMimePrivate*>(d_ptr.data());
    bool ret  = false;
    PKCS7 *p7 = nullptr;

    if (!d->_certificate || !d->_privateKey) {
        qCDebug(SIMPLEMAIL_SMIME) << "no certificate or private key";
        return ret;
    }

    wrapMimeMultiPart();
    setSignedHeader();

    int flags = PKCS7_DETACHED | PKCS7_STREAM | PKCS7_BINARY;

    BIO *out = NULL;

    if (!writeInputBuffer())
        goto err;

    /* Sign content */
    p7 = PKCS7_sign(d->_certificate, d->_privateKey, NULL, d->_input, flags);
    if (!p7)
        goto err;

    out = BIO_new(BIO_s_mem());
    if (!out)
        goto err;

    /* Write out S/MIME message */
    if (!SMIME_write_PKCS7(out,
                           p7,
                           d->_input,
                           flags |
                               PKCS7_CRLFEOL)) // needed for intializing/finalizing SMIME structure
        goto err;

    if (!handleData(p7, nullptr, 0))
        goto err;

    _mimeMessage->addPart(std::shared_ptr<SMime>(this));

    ret = true;
err:
    if (!ret) {
        qCDebug(SIMPLEMAIL_SMIME) << "Error Signing Data";
    }
    PKCS7_free(p7);
    BIO_free(out);
    return ret;
}

bool SMime::encrypt()
{
    SMimePrivate *d = static_cast<SMimePrivate*>(d_ptr.data());
    bool ret  = false;
    PKCS7 *p7 = nullptr;

    if (!d->_recipsReceiver) {
        qCDebug(SIMPLEMAIL_SMIME) << "no public key";
        return ret;
    }

    setEncryptionHeader();

    int flags = PKCS7_STREAM;

    if (!writeInputBuffer())
        goto err;

    /* encrypt content */
    p7 = PKCS7_encrypt(d->_recipsReceiver, d->_input, EVP_des_ede3_cbc(), flags);

    if (!p7)
        goto err;

    if (!handleData(p7, d->_input, flags))
        goto err;

    _mimeMessage->setContent(std::shared_ptr<SMime>(this));

    ret = true;
err:
    if (!ret) {
        qCDebug(SIMPLEMAIL_SMIME) << "Error Encrypting Data";
    }
    PKCS7_free(p7);
    return ret;
}

bool SMime::signAndEncrypt()
{
    SMimePrivate *d = static_cast<SMimePrivate*>(d_ptr.data());
    bool ret           = false;
    PKCS7 *p7          = nullptr;
    BIO *signedContent = nullptr;
    if (!d->_certificate || !d->_privateKey) {
        qCDebug(SIMPLEMAIL_SMIME) << "no certificate or private key";
        return ret;
    }
    if (!d->_recipsReceiver) {
        qCDebug(SIMPLEMAIL_SMIME) << "no public key";
        return ret;
    }

    setEncryptionHeader();

    int flags = PKCS7_STREAM;

    if (!writeInputBuffer())
        goto err;

    /* Sign content */
    p7 = PKCS7_sign(d->_certificate, d->_privateKey, NULL, d->_input, flags);
    if (!p7)
        goto err;

    signedContent = BIO_new(BIO_s_mem());
    if (!SMIME_write_PKCS7(signedContent, p7, d->_input, flags | PKCS7_CRLFEOL))
        goto err;

    PKCS7_free(p7);
    p7 = NULL;
    p7 = PKCS7_encrypt(d->_recipsReceiver, signedContent, EVP_des_ede3_cbc(), flags);

    if (!p7)
        goto err;

    if (!handleData(p7, signedContent, flags))
        goto err;

    _mimeMessage->setContent(std::shared_ptr<SMime>(this));

    ret = true;
err:
    if (!ret) {
        qCDebug(SIMPLEMAIL_SMIME) << "Error Signing/Encrypting Data";
    }
    PKCS7_free(p7);
    BIO_free(signedContent);
    return ret;
}

void SMime::setSignedHeader()
{
    Q_D(MimePart);
    d->contentType = QByteArrayLiteral("application/pkcs7-signature; name=\"smime.p7s\"");
    d->contentCharset.clear();
    d->contentEncoding = Base64;
    d->contentIsBase64 = true;

    d->header.append("Content-Disposition: attachment; filename=\"smime.p7s\"\r\n");
}

void SMime::setEncryptionHeader()
{
    Q_D(MimePart);
    d->contentType = QByteArrayLiteral(
        "application/x-pkcs7-mime; smime-type=enveloped-data; name=\"smime.p7m\"");
    d->contentCharset.clear();
    d->contentEncoding = Base64;
    d->contentIsBase64 = true;

    d->header.append("Content-Disposition: attachment; filename=\"smime.p7m\"\r\n");
}

void SMime::loadPKCS12PrivateKey()
{
    SMimePrivate *d = static_cast<SMimePrivate*>(d_ptr.data());
    QFile file(d->_keyfile);
    if (!file.exists())
        return;
    file.open(QFile::ReadOnly);
    QByteArray buffer = file.readAll();
    file.close();

    BIO *keyBuffer = BIO_new(BIO_s_mem());
    if (keyBuffer == NULL) {
        qCDebug(SIMPLEMAIL_SMIME) << "Error opening file " << d->_keyfile;
        return;
    }

    BIO_write(keyBuffer, (void *) buffer.data(), buffer.length());
    PKCS12 *p12 = d2i_PKCS12_bio(keyBuffer, NULL);
    BIO_free(keyBuffer);

    if (p12 == NULL) {
        qCDebug(SIMPLEMAIL_SMIME) << "Error reading PKCS#12 file";
    }

    if (!PKCS12_parse(
            p12, d->_password.toStdString().c_str(), &d->_privateKey, &d->_certificate, &d->_certificateCA)) {
        OSSL_PROVIDER_try_load(nullptr, "legacy", 1);

        if (!PKCS12_parse(p12,
                          d->_password.toStdString().c_str(),
                          &d->_privateKey,
                          &d->_certificate,
                          &d->_certificateCA)) {
            qCDebug(SIMPLEMAIL_SMIME) << "Error parsing PKCS#12 file";
        }
    }

    PKCS12_free(p12);
}

void SMime::loadPKCS12PublicKey()
{
    SMimePrivate *d = static_cast<SMimePrivate*>(d_ptr.data());
    QFile file(d->_publicKeyfile);
    if (!file.exists())
        return;
    file.open(QFile::ReadOnly);
    QByteArray buffer = file.readAll();
    file.close();

    BIO *keyBuffer = BIO_new(BIO_s_mem());
    if (keyBuffer == NULL) {
        qCDebug(SIMPLEMAIL_SMIME) << "Error opening file " << d->_publicKeyfile;
        return;
    }
    BIO_write(keyBuffer, (void *) buffer.data(), buffer.length());

    X509 *publicrcert = PEM_read_bio_X509(keyBuffer, NULL, 0, NULL);
    BIO_free(keyBuffer);

    d->_recipsReceiver = sk_X509_new_null();

    if (!d->_recipsReceiver || !sk_X509_push(d->_recipsReceiver, publicrcert)) {
        return;
    }
    publicrcert = nullptr;
}

void SMime::initOpenSSL()
{
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();
}

void SMime::wrapMimeMultiPart()
{
    auto &mimeMultiPart = *_mimeMessage->getContent();

    if (typeid(mimeMultiPart) == typeid(MimeMultiPart)) {
        MimeMultiPart *multiPartSigned = new MimeMultiPart(MimeMultiPart::Signed);
        multiPartSigned->addPart(_mimeMessage->getContent());
        _mimeMessage->setContent(std::shared_ptr<MimeMultiPart>(multiPartSigned));
    }
}

bool SMime::writeInputBuffer()
{
    SMimePrivate *d = static_cast<SMimePrivate*>(d_ptr.data());
    d->_input = BIO_new(BIO_s_mem());
    if (!d->_input)
        return false;
    if (!BIO_write(d->_input, (void *) d->_message.data(), d->_message.length()))
        return false;
    return true;
}

bool SMime::writeMimeMessageBuffer()
{
    SMimePrivate *d = static_cast<SMimePrivate*>(d_ptr.data());
    QBuffer buffer;
    buffer.open(QBuffer::ReadWrite);

    _mimeMessage->getContent()->write(&buffer);
    buffer.close();

    d->_message = buffer.data();

    return true;
}

bool SMime::handleData(PKCS7 *p7, BIO *dataIn, int flags)
{
    QByteArray contentBuffer;

    // Buffer for signed and/or encrypted data;
    BIO *encryptedData = BIO_new(BIO_s_mem());
    if (!encryptedData)
        return false;

    BIO *b64 = BIO_new(BIO_f_base64());
    BIO *tmp = BIO_push(b64, encryptedData);

    i2d_ASN1_bio_stream(tmp, (ASN1_VALUE *) p7, dataIn, flags, ASN1_ITEM_rptr(PKCS7));

    BIO_flush(tmp);
    BIO_pop(tmp);
    BIO_free(b64);

    char buffer[1024];
    while (BIO_get_line(encryptedData, &buffer[0], sizeof(buffer))) {
        qCDebug(SIMPLEMAIL_SMIME) << QString::fromLatin1(buffer);
        if (strncmp(&buffer[0], "-----", strlen("-----")) != 0) {
            contentBuffer += QByteArray(&buffer[0], strlen(buffer) - 1);
            contentBuffer += QByteArray("\r\n");
        }
    }

    setContent(contentBuffer);
    BIO_free(encryptedData);
    return true;
}
