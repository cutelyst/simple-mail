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

#include "mimemessage.h"

#include <QDateTime>
#include <QtCore/QStringBuilder>
#include "quotedprintable.h"
#include <typeinfo>

/* [1] Constructors and Destructors */
MimeMessage::MimeMessage(bool createAutoMimeContent) :
    hEncoding(MimePart::_8Bit)
{
    if (createAutoMimeContent)
        this->content = new MimeMultiPart();
    
    autoMimeContentCreated = createAutoMimeContent;
}

MimeMessage::~MimeMessage()
{
    if (this->autoMimeContentCreated)
    {
      this->autoMimeContentCreated = false;
      delete (this->content);
    }
}

/* [1] --- */


/* [2] Getters and Setters */
MimePart& MimeMessage::getContent() {
    return *content;
}

void MimeMessage::setContent(MimePart *content) {
    if (this->autoMimeContentCreated)
    {
      this->autoMimeContentCreated = false;
      delete (this->content);
    }
    this->content = content;
}

void MimeMessage::setSender(EmailAddress* e)
{
    this->sender = e;
}

void MimeMessage::addRecipient(EmailAddress* rcpt, RecipientType type)
{
    switch (type)
    {
    case To:
        recipientsTo << rcpt;
        break;
    case Cc:
        recipientsCc << rcpt;
        break;
    case Bcc:
        recipientsBcc << rcpt;
        break;
    }
}

void MimeMessage::addTo(EmailAddress* rcpt) {
    this->recipientsTo << rcpt;
}

void MimeMessage::addCc(EmailAddress* rcpt) {
    this->recipientsCc << rcpt;
}

void MimeMessage::addBcc(EmailAddress* rcpt) {
    this->recipientsBcc << rcpt;
}

void MimeMessage::setSubject(const QString & subject)
{
    this->subject = subject;
}

void MimeMessage::addPart(MimePart *part)
{
    if (typeid(*content) == typeid(MimeMultiPart)) {
        ((MimeMultiPart*) content)->addPart(part);
    };
}

void MimeMessage::setHeaderEncoding(MimePart::Encoding hEnc)
{
    this->hEncoding = hEnc;
}

const EmailAddress & MimeMessage::getSender() const
{
    return *sender;
}

const QList<EmailAddress*> & MimeMessage::getRecipients(RecipientType type) const
{
    switch (type)
    {
    default:
    case To:
        return recipientsTo;
    case Cc:
        return recipientsCc;
    case Bcc:
        return recipientsBcc;
    }
}

const QString & MimeMessage::getSubject() const
{
    return subject;
}

const QList<MimePart*> & MimeMessage::getParts() const
{
    if (typeid(*content) == typeid(MimeMultiPart)) {
        return ((MimeMultiPart*) content)->getParts();
    }
    else {
        QList<MimePart*> *res = new QList<MimePart*>();
        res->append(content);
        return *res;
    }
}

/* [2] --- */


/* [3] Public Methods */

QString MimeMessage::toString()
{
    QString mime;

    /* =========== MIME HEADER ============ */

    /* ---------- Sender / From ----------- */
    mime = QStringLiteral("From:");
    if (!sender->name().isEmpty())
    {
        mime.append(encode(hEncoding, sender->name()));
    }
    mime += QStringLiteral(" <") % sender->address() % QStringLiteral(">\r\n");
    /* ---------------------------------- */


    /* ------- Recipients / To ---------- */    
    mime.append(QStringLiteral("To:"));
    QList<EmailAddress*>::iterator it;  int i;
    for (i = 0, it = recipientsTo.begin(); it != recipientsTo.end(); ++it, ++i)
    {
        if (i != 0) {
            mime.append(QLatin1Char(','));
        }

        if (!(*it)->name().isEmpty())
        {
            mime.append(encode(hEncoding, (*it)->name()));
        }
        mime += QStringLiteral(" <") % (*it)->address() % QLatin1Char('>');
    }
    mime += QStringLiteral("\r\n");
    /* ---------------------------------- */

    /* ------- Recipients / Cc ---------- */
    if (recipientsCc.size() != 0) {
        mime += QStringLiteral("Cc:");
    }

    for (i = 0, it = recipientsCc.begin(); it != recipientsCc.end(); ++it, ++i) {
        if (i != 0) {
            mime.append(QLatin1Char(','));
        }

        if (!(*it)->name().isEmpty()) {
            mime.append(encode(hEncoding, (*it)->name()));
        }
        mime += QLatin1String(" <") % (*it)->address() % QLatin1Char('>');
    }
    if (recipientsCc.size() != 0) {
        mime += QStringLiteral("\r\n");
    }
    /* ---------------------------------- */

    /* ------------ Subject ------------- */
    mime += QStringLiteral("Subject: ");

    // TODO subject on previous implementation didn't prepend
    // an empty space at the beggining, maybe add a conditional
    // subject bool to encode()?
    mime.append(encode(hEncoding, subject));

    mime += QStringLiteral("\r\n");
    mime += QStringLiteral("MIME-Version: 1.0\r\n");

    mime += content->toString();
    return mime;
}

QString MimeMessage::encode(MimePart::Encoding codec, const QString &data)
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

/* [3] --- */
