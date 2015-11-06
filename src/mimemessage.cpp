/*
  Copyright (c) 2011-2012 - Tőkés Attila
  Copyright (C) 2015 Daniel Nicoletti <dantti12@gmail.com>

x
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

#include "mimemessage_p.h"

#include <QDateTime>
#include <QtCore/QStringBuilder>
#include "quotedprintable.h"
#include <typeinfo>

MimeMessage::MimeMessage(bool createAutoMimeContent) :
    d_ptr(new MimeMessagePrivate)
{
    Q_D(MimeMessage);
    if (createAutoMimeContent) {
        d->content = new MimeMultiPart();
    }
    
    d->autoMimeContentCreated = createAutoMimeContent;
}

MimeMessage::~MimeMessage()
{
    Q_D(MimeMessage);

    if (d->autoMimeContentCreated) {
      d->autoMimeContentCreated = false;
      delete (d->content);
    }
    delete d_ptr;
}

MimePart& MimeMessage::getContent()
{
    Q_D(MimeMessage);
    return *d->content;
}

void MimeMessage::setContent(MimePart *content)
{
    Q_D(MimeMessage);
    if (d->autoMimeContentCreated) {
      d->autoMimeContentCreated = false;
      delete (d->content);
    }
    d->content = content;
}

void MimeMessage::setSender(const EmailAddress &sender)
{
    Q_D(MimeMessage);
    d->sender = sender;
}

void MimeMessage::addRecipient(const EmailAddress &rcpt, RecipientType type)
{
    Q_D(MimeMessage);
    switch (type) {
    case To:
        d->recipientsTo.append(rcpt);
        break;
    case Cc:
        d->recipientsCc.append(rcpt);
        break;
    case Bcc:
        d->recipientsBcc.append(rcpt);
        break;
    }
}

void MimeMessage::addTo(const EmailAddress &rcpt)
{
    Q_D(MimeMessage);
    d->recipientsTo.append(rcpt);
}

void MimeMessage::addCc(const EmailAddress &rcpt)
{
    Q_D(MimeMessage);
    d->recipientsCc.append(rcpt);
}

void MimeMessage::addBcc(const EmailAddress &rcpt)
{
    Q_D(MimeMessage);
    d->recipientsBcc.append(rcpt);
}

void MimeMessage::setSubject(const QString & subject)
{
    Q_D(MimeMessage);
    d->subject = subject;
}

void MimeMessage::addPart(MimePart *part)
{
    Q_D(MimeMessage);
    if (typeid(*d->content) == typeid(MimeMultiPart)) {
        ((MimeMultiPart*) d->content)->addPart(part);
    }
}

void MimeMessage::setHeaderEncoding(MimePart::Encoding hEnc)
{
    Q_D(MimeMessage);
    d->encoding = hEnc;
}

EmailAddress MimeMessage::sender() const
{
    Q_D(const MimeMessage);
    return d->sender;
}

QList<EmailAddress> MimeMessage::getRecipients(MimeMessage::RecipientType type) const
{
    Q_D(const MimeMessage);
    switch (type)
    {
    default:
    case To:
        return d->recipientsTo;
    case Cc:
        return d->recipientsCc;
    case Bcc:
        return d->recipientsBcc;
    }
}

QString MimeMessage::subject() const
{
    Q_D(const MimeMessage);
    return d->subject;
}

const QList<MimePart*> & MimeMessage::getParts() const
{
    Q_D(const MimeMessage);
    if (typeid(*d->content) == typeid(MimeMultiPart)) {
        return ((MimeMultiPart*) d->content)->getParts();
    } else {
        QList<MimePart*> *res = new QList<MimePart*>();
        res->append(d->content);
        return *res;
    }
}

QString MimeMessage::toString()
{
    Q_D(const MimeMessage);

    QString mime;

    /* =========== MIME HEADER ============ */

    /* ---------- Sender / From ----------- */
    mime = QStringLiteral("From:");
    if (!d->sender.name().isEmpty())
    {
        mime.append(MimeMessagePrivate::encode(d->encoding, d->sender.name()));
    }
    mime += QStringLiteral(" <") % d->sender.address() % QStringLiteral(">\r\n");
    /* ---------------------------------- */


    /* ------- Recipients / To ---------- */
    mime.append(QStringLiteral("To:"));
    QList<EmailAddress>::const_iterator it;  int i;
    for (i = 0, it = d->recipientsTo.constBegin(); it != d->recipientsTo.constEnd(); ++it, ++i)
    {
        if (i != 0) {
            mime.append(QLatin1Char(','));
        }

        if (!(*it).name().isEmpty())
        {
            mime.append(MimeMessagePrivate::encode(d->encoding, (*it).name()));
        }
        mime += QStringLiteral(" <") % (*it).address() % QLatin1Char('>');
    }
    mime += QStringLiteral("\r\n");
    /* ---------------------------------- */

    /* ------- Recipients / Cc ---------- */
    if (d->recipientsCc.size() != 0) {
        mime += QStringLiteral("Cc:");
    }

    for (i = 0, it = d->recipientsCc.constBegin(); it != d->recipientsCc.constEnd(); ++it, ++i) {
        if (i != 0) {
            mime.append(QLatin1Char(','));
        }

        if (!(*it).name().isEmpty()) {
            mime.append(MimeMessagePrivate::encode(d->encoding, (*it).name()));
        }
        mime += QLatin1String(" <") % (*it).address() % QLatin1Char('>');
    }
    if (!d->recipientsCc.isEmpty()) {
        mime += QStringLiteral("\r\n");
    }
    /* ---------------------------------- */

    /* ------------ Subject ------------- */
    mime += QStringLiteral("Subject: ");

    // TODO subject on previous implementation didn't prepend
    // an empty space at the beggining, maybe add a conditional
    // subject bool to encode()?
    mime.append(MimeMessagePrivate::encode(d->encoding, d->subject));

    mime += QStringLiteral("\r\n");
    mime += QStringLiteral("MIME-Version: 1.0\r\n");

    mime += d->content->toString();
    return mime;
}

QString MimeMessagePrivate::encode(MimePart::Encoding codec, const QString &data)
{
    switch (codec) {
    case MimePart::Base64:
        return QLatin1String(" =?utf-8?B?") % QString::fromLatin1(data.toLatin1().toBase64()) % QLatin1String("?=");
    case MimePart::QuotedPrintable:
        return QLatin1String(" =?utf-8?Q?") % QuotedPrintable::encode(data.toLatin1())
                .replace(QLatin1Char(' '), QLatin1String("_"))
                .replace(QLatin1Char(':'), QLatin1String("=3A")) % QLatin1String("?=");
    default:
        return QLatin1Char(' ') % data;
    }
}
