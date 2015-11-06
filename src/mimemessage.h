/*
  Copyright (c) 2011-2012 - Tőkés Attila

  This file is part of SmtpClient for Qt.

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
#include <QList>

#include "smtpexports.h"

class MimeMessagePrivate;
class SMTP_EXPORT MimeMessage
{
    Q_DECLARE_PRIVATE(MimeMessage)
public:
    enum RecipientType {
        To,                 // primary
        Cc,                 // carbon copy
        Bcc                 // blind carbon copy
    };

    MimeMessage(bool createAutoMimeConent = true);
    ~MimeMessage();

    void setSender(const EmailAddress &sender);
    void addRecipient(const EmailAddress &rcpt, RecipientType type = To);
    void addTo(const EmailAddress &rcpt);
    void addCc(const EmailAddress &rcpt);
    void addBcc(const EmailAddress &rcpt);
    void setSubject(const QString &subject);
    void addPart(MimePart* part);

    void setHeaderEncoding(MimePart::Encoding);

    EmailAddress sender() const;
    QList<EmailAddress> getRecipients(RecipientType type = To) const;
    QString subject() const;
    const QList<MimePart*> & getParts() const;

    MimePart& getContent();
    void setContent(MimePart *content);

    virtual QString toString();

protected:
    MimeMessagePrivate *d_ptr;


};

#endif // MIMEMESSAGE_H
