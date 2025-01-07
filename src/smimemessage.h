#ifndef SMIMEMESSAGE_H
#define SMIMEMESSAGE_H

#include "mimemessage.h"
#include "smtpexports.h"

class QIODevice;
namespace SimpleMail {

class SMimeMessagePrivate;
class SMTP_EXPORT SMimeMessage : public MimeMessage
{
public:
    explicit SMimeMessage();
    virtual ~SMimeMessage();

    void setKeyFile(const QString &filename, const QString &password);

    void setPublicKey(const QString &filename);

    bool sign();
    bool encrypt();
    bool signAndEncrypt();
};

} // namespace SimpleMail

#endif // SMIMEMESSAGE_H
