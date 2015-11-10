/*
  Copyright (c) 2011-2012 - Tőkés Attila
  Copyright (C) 2015 Daniel Nicoletti <dantti12@gmail.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  See the LICENSE file for more details.
*/

#ifndef MIMEMESSAGE_H
#define MIMEMESSAGE_H

#include "mimepart.h"
#include "mimemultipart.h"
#include "emailaddress.h"

#include "smtpexports.h"

class QIODevice;
namespace SimpleMail {

class MimeMessagePrivate;
class SMTP_EXPORT MimeMessage
{
    Q_DECLARE_PRIVATE(MimeMessage)
public:
    MimeMessage(bool createAutoMimeConent = true);
    virtual ~MimeMessage();

    void setSender(const EmailAddress &sender);
    EmailAddress sender() const;

    void setToRecipients(const QList<EmailAddress> &toList);
    QList<EmailAddress> toRecipients() const;
    void addTo(const EmailAddress &rcpt);

    void setCcRecipients(const QList<EmailAddress> &ccList);
    QList<EmailAddress> ccRecipients() const;
    void addCc(const EmailAddress &rcpt);

    void setBccRecipients(const QList<EmailAddress> &bccList);
    QList<EmailAddress> bccRecipients() const;
    void addBcc(const EmailAddress &rcpt);

    void setSubject(const QString &subject);
    void addPart(MimePart* part);

    void setHeaderEncoding(MimePart::Encoding);

    QString subject() const;
    QList<MimePart *> parts() const;

    MimePart& getContent();
    void setContent(MimePart *content);

    bool write(QIODevice *device);

protected:
    MimeMessagePrivate *d_ptr;
};

}

#endif // MIMEMESSAGE_H
