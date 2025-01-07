#include "smimepart.h"

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
#include <QScopeGuard>

Q_LOGGING_CATEGORY(SIMPLEMAIL_SMIMEPART, "simplemail.smimepart", QtInfoMsg)

using namespace SimpleMail;

SMimePart::SMimePart()
    : MimePart(new SMimePartPrivate)
{
    initOpenSSL();
}

SMimePart::~SMimePart()
{
}

void SMimePart::setKeyFile(const QString &filename, const QString &password)
{
    SMimePartPrivate *d = static_cast<SMimePartPrivate *>(d_ptr.data());
    d->_keyfile         = filename;
    d->_password        = password;
    loadPKCS12PrivateKey();
}

void SMimePart::setPublicKey(const QString &filename)
{
    SMimePartPrivate *d = static_cast<SMimePartPrivate *>(d_ptr.data());
    d->_publicKeyfile   = filename;
    loadPKCS12PublicKey();
}

bool SMimePart::sign()
{
    qCDebug(SIMPLEMAIL_SMIMEPART) << "signing message";
    SMimePartPrivate *d = static_cast<SMimePartPrivate *>(d_ptr.data());
    PKCS7 *p7           = nullptr;

    if (!d->_certificate || !d->_privateKey) {
        qCDebug(SIMPLEMAIL_SMIMEPART) << "no certificate or private key";
        return false;
    }

    setSignedHeader();

    int flags = PKCS7_DETACHED | PKCS7_STREAM | PKCS7_BINARY;

    BIO *out = NULL;

    auto cleanup = qScopeGuard([&]() {
        PKCS7_free(p7);
        BIO_free(out);
    });

    if (!writeInputBuffer()) {
        qCDebug(SIMPLEMAIL_SMIMEPART) << "Error writing input buffer";
        return false;
    }

    /* Sign content */
    p7 = PKCS7_sign(d->_certificate.get(), d->_privateKey.get(), NULL, d->_input.get(), flags);
    if (!p7) {
        qCDebug(SIMPLEMAIL_SMIMEPART) << "Error signing data";
        return false;
    }

    out = BIO_new(BIO_s_mem());
    if (!out) {
        qCDebug(SIMPLEMAIL_SMIMEPART) << "Error creating output buffer";
        return false;
    }
    /* Write out S/MIME message */
    if (!SMIME_write_PKCS7(out,
                           p7,
                           d->_input.get(),
                           flags |
                               PKCS7_CRLFEOL)) { // needed for intializing/finalizing SMIME structure
            qCDebug(SIMPLEMAIL_SMIMEPART) << "Error finalizing S/MIME message structure";
            return false;
    }

    if (!handleData(p7, nullptr, 0)){
        qCDebug(SIMPLEMAIL_SMIMEPART) << "Error finishing S/MIME message";
        return false;
    }

    return true;
}

bool SMimePart::encrypt()
{
    qCDebug(SIMPLEMAIL_SMIMEPART) << "encrypting message";
    SMimePartPrivate *d = static_cast<SMimePartPrivate *>(d_ptr.data());
    PKCS7 *p7           = nullptr;

    if (!d->_recipsReceiver) {
        qCDebug(SIMPLEMAIL_SMIMEPART) << "no public key";
        return false;
    }

    setEncryptionHeader();

    int flags = PKCS7_STREAM;

    auto cleanup = qScopeGuard([&]() {
        PKCS7_free(p7);
    });

    if (!writeInputBuffer()) {
        qCDebug(SIMPLEMAIL_SMIMEPART) << "Error writing input buffer";
        return false;
    }

    /* encrypt content */
    p7 = PKCS7_encrypt(d->_recipsReceiver.get(), d->_input.get(), EVP_des_ede3_cbc(), flags);

    if (!p7) {
        qCDebug(SIMPLEMAIL_SMIMEPART) << "Error encrypting data";
        return false;
    }

    if (!handleData(p7, d->_input.get(), flags)) {
        qCDebug(SIMPLEMAIL_SMIMEPART) << "Error finishing S/MIME message";
        return false;
    }

    return true;
}

bool SMimePart::signAndEncrypt()
{
    qCDebug(SIMPLEMAIL_SMIMEPART) << "signing and encrypting message";
    SMimePartPrivate *d = static_cast<SMimePartPrivate *>(d_ptr.data());
    PKCS7 *p7           = nullptr;
    BIO *signedContent  = nullptr;
    if (!d->_certificate || !d->_privateKey) {
        qCDebug(SIMPLEMAIL_SMIMEPART) << "no certificate or private key";
        return false;
    }
    if (!d->_recipsReceiver) {
        qCDebug(SIMPLEMAIL_SMIMEPART) << "no public key";
        return false;
    }

    setEncryptionHeader();

    int flags = PKCS7_STREAM;

    auto cleanup = qScopeGuard([&]() {
        PKCS7_free(p7);
        BIO_free(signedContent);
    });

    if (!writeInputBuffer()) {
        qCDebug(SIMPLEMAIL_SMIMEPART) << "Error writing input buffer";
        return false;
    }

    /* Sign content */
    p7 = PKCS7_sign(d->_certificate.get(), d->_privateKey.get(), NULL, d->_input.get(), flags);
    if (!p7) {
        qCDebug(SIMPLEMAIL_SMIMEPART) << "Error signing input buffer";
        return false;
    }

    signedContent = BIO_new(BIO_s_mem());
    if (!SMIME_write_PKCS7(signedContent, p7, d->_input.get(), flags | PKCS7_CRLFEOL)){
        qCDebug(SIMPLEMAIL_SMIMEPART) << "Error finalizing S/MIME message structure";
        return false;
    }

    PKCS7_free(p7);
    p7 = NULL;
    p7 = PKCS7_encrypt(d->_recipsReceiver.get(), signedContent, EVP_des_ede3_cbc(), flags);

    if (!p7) {
        qCDebug(SIMPLEMAIL_SMIMEPART) << "Error encrypting signed data";
        return false;
    }

    if (!handleData(p7, signedContent, flags)) {
        qCDebug(SIMPLEMAIL_SMIMEPART) << "Error finishing S/MIME message";
        return false;
    }

    return true;
}

void SMimePart::setSignedHeader()
{
    Q_D(MimePart);
    d->contentType = QByteArrayLiteral("application/pkcs7-signature; name=\"smime.p7s\"");
    d->contentCharset.clear();
    d->contentEncoding = Base64;
    d->contentIsBase64 = true;

    d->header.append("Content-Disposition: attachment; filename=\"smime.p7s\"\r\n");
}

void SMimePart::setEncryptionHeader()
{
    Q_D(MimePart);
    d->contentType = QByteArrayLiteral(
        "application/x-pkcs7-mime; smime-type=enveloped-data; name=\"smime.p7m\"");
    d->contentCharset.clear();
    d->contentEncoding = Base64;
    d->contentIsBase64 = true;

    d->header.append("Content-Disposition: attachment; filename=\"smime.p7m\"\r\n");
}

void SMimePart::loadPKCS12PrivateKey()
{
    SMimePartPrivate *d = static_cast<SMimePartPrivate *>(d_ptr.data());
    QFile file(d->_keyfile);
    if (!file.exists())
        return;
    file.open(QFile::ReadOnly);
    QByteArray buffer = file.readAll();
    file.close();

    BIO *keyBuffer = BIO_new(BIO_s_mem());
    if (keyBuffer == NULL) {
        qCDebug(SIMPLEMAIL_SMIMEPART) << "Error opening file " << d->_keyfile;
        return;
    }

    BIO_write(keyBuffer, (void *) buffer.data(), buffer.length());
    PKCS12 *p12 = d2i_PKCS12_bio(keyBuffer, NULL);
    BIO_free(keyBuffer);

    if (p12 == NULL) {
        qCDebug(SIMPLEMAIL_SMIMEPART) << "Error reading PKCS#12 file";
    }

    EVP_PKEY *privateKey          = nullptr;
    X509 *certificate             = nullptr;
    STACK_OF(X509) *certificateCA = nullptr;
    if (!PKCS12_parse(
            p12, d->_password.toStdString().c_str(), &privateKey, &certificate, &certificateCA)) {
        OSSL_PROVIDER_try_load(nullptr, "legacy", 1);

        if (!PKCS12_parse(p12,
                          d->_password.toStdString().c_str(),
                          &privateKey,
                          &certificate,
                          &certificateCA)) {
            qCDebug(SIMPLEMAIL_SMIMEPART) << "Error parsing PKCS#12 file";
        }
    }
    d->_privateKey.reset(privateKey);
    d->_certificate.reset(certificate);
    d->_certificateCA.reset(certificateCA);

    PKCS12_free(p12);
}

void SMimePart::loadPKCS12PublicKey()
{
    SMimePartPrivate *d = static_cast<SMimePartPrivate *>(d_ptr.data());
    QFile file(d->_publicKeyfile);
    if (!file.exists())
        return;
    file.open(QFile::ReadOnly);
    QByteArray buffer = file.readAll();
    file.close();

    BIO *keyBuffer = BIO_new(BIO_s_mem());
    if (keyBuffer == NULL) {
        qCDebug(SIMPLEMAIL_SMIMEPART) << "Error opening file " << d->_publicKeyfile;
        return;
    }
    BIO_write(keyBuffer, (void *) buffer.data(), buffer.length());

    X509 *publicrcert = PEM_read_bio_X509(keyBuffer, NULL, 0, NULL);
    BIO_free(keyBuffer);

    STACK_OF(X509) *recipsReceiver = sk_X509_new_null();

    if (!recipsReceiver || !sk_X509_push(recipsReceiver, publicrcert)) {
        return;
    }
    d->_recipsReceiver.reset(recipsReceiver);
    publicrcert = nullptr;
}

void SMimePart::initOpenSSL()
{
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();
}

bool SMimePart::writeInputBuffer()
{
    SMimePartPrivate *d = static_cast<SMimePartPrivate *>(d_ptr.data());
    BIO *input          = BIO_new(BIO_s_mem());
    d->_input           = std::unique_ptr<BIO>(input);
    if (!d->_input)
        return false;
    if (!BIO_write(d->_input.get(), (void *) d->_message.data(), d->_message.length()))
        return false;
    return true;
}

bool SMimePart::writeMimeMessageBuffer(const std::shared_ptr<SimpleMail::MimePart> &mimeParts)
{
    SMimePartPrivate *d = static_cast<SMimePartPrivate *>(d_ptr.data());
    QBuffer buffer;
    buffer.open(QBuffer::ReadWrite);

    mimeParts->write(&buffer);
    buffer.close();

    d->_message = buffer.data();

    return true;
}

bool SMimePart::handleData(PKCS7 *p7, BIO *dataIn, int flags)
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
        qCDebug(SIMPLEMAIL_SMIMEPART) << QString::fromLatin1(buffer);
        if (strncmp(&buffer[0], "-----", strlen("-----")) != 0) {
            contentBuffer += QByteArray(&buffer[0], strlen(buffer) - 1);
            contentBuffer += QByteArray("\r\n");
        }
    }

    setContent(contentBuffer);
    BIO_free(encryptedData);
    return true;
}
