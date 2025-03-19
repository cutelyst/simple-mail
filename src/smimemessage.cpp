#include "smimemessage.h"

#include "mimemultipart.h"
#include "smimemessage_p.h"

using namespace SimpleMail;

SMimeMessage::SMimeMessage()
    : MimeMessage(new SMimeMessagePrivate())
{
}

SMimeMessage::~SMimeMessage()
{
}

void SMimeMessage::setKeyFile(const QString &filename, const QString &password)
{
    SMimeMessagePrivate *dPtr = static_cast<SMimeMessagePrivate *>(d.data());
    dPtr->_smimePart->setKeyFile(filename, password);
}

void SMimeMessage::setPublicKey(const QString &filename)
{
    SMimeMessagePrivate *dPtr = static_cast<SMimeMessagePrivate *>(d.data());
    dPtr->_smimePart->setPublicKey(filename);
}

bool SMimeMessage::sign()
{
    SMimeMessagePrivate *dPtr = static_cast<SMimeMessagePrivate *>(d.data());
    dPtr->_smimePart->writeMimeMessageBuffer(this->getContent());

    MimeMultiPart *multiPartSigned = new MimeMultiPart(MimeMultiPart::Signed);
    multiPartSigned->addPart(getContent());
    setContent(std::shared_ptr<MimeMultiPart>(multiPartSigned));

    bool ret = dPtr->_smimePart->sign();
    addPart(dPtr->_smimePart);
    return ret;
}

bool SMimeMessage::encrypt()
{
    SMimeMessagePrivate *dPtr = static_cast<SMimeMessagePrivate *>(d.data());
    dPtr->_smimePart->writeMimeMessageBuffer(this->getContent());
    bool ret = dPtr->_smimePart->encrypt();
    setContent(dPtr->_smimePart);
    return ret;
}

bool SMimeMessage::signAndEncrypt()
{
    SMimeMessagePrivate *dPtr = static_cast<SMimeMessagePrivate *>(d.data());
    dPtr->_smimePart->writeMimeMessageBuffer(this->getContent());
    bool ret = dPtr->_smimePart->signAndEncrypt();
    setContent(dPtr->_smimePart);
    return ret;
}
