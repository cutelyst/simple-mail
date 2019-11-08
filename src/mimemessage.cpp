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

#include <QtCore/QDebug>
#include "quotedprintable.h"
#include <typeinfo>

#include <QDateTime>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(SIMPLEMAIL_MIMEMSG, "simplemail.mimemessage")

using namespace SimpleMail;

MimeMessage::MimeMessage(bool createAutoMimeContent) : d(new MimeMessagePrivate)
{
//    Q_D(MimeMessage);
    qDebug() << Q_FUNC_INFO;
    if (createAutoMimeContent) {
        d->content = new MimeMultiPart();
        auto foo = d->content;
        qDebug() << Q_FUNC_INFO << "========" << (typeid(d->content) == typeid(MimeMultiPart)) << typeid(MimeMultiPart).name();
        qDebug() << Q_FUNC_INFO << "========" << (typeid(*foo) == typeid(MimeMultiPart)) << typeid(d->content).name();

    }

    d->autoMimeContentCreated = createAutoMimeContent;
}

MimeMessage::MimeMessage(const MimeMessage &other) : d(other.d)
{
    qDebug() << Q_FUNC_INFO << d->content << d->ref;
}

MimeMessage::~MimeMessage()
{
    qDebug() << Q_FUNC_INFO << d->content;
    if (d->content) {
        qDebug() << Q_FUNC_INFO << "CONTENT" << d->ref  << d->content->data();
    }
    //    delete d;
}

MimeMessage &MimeMessage::operator=(const MimeMessage &other)
{
    qDebug() << Q_FUNC_INFO << this << d->content;
    // TODO this is broken
    d = other.d;
    return *this;
}

MimePart& MimeMessage::getContent()
{
//    Q_D(MimeMessage);
    return *d->content;
}

void MimeMessage::setContent(MimePart *content)
{
//    Q_D(MimeMessage);
    if (d->autoMimeContentCreated) {
      d->autoMimeContentCreated = false;
      delete d->content;
    }
    d->content = content;
}

bool MimeMessage::write(QIODevice *device) const
{
    // Headers
    QByteArray data;

    if (!d->listExtraHeaders.isEmpty()) {
        const auto listExtraHeaders = d->listExtraHeaders;
        for (const QByteArray &header : listExtraHeaders) {
            data += MimeMessagePrivate::encodeData(d->encoding, QString::fromLatin1(header), true) + QByteArrayLiteral("\r\n");
        }
        if (device->write(data) != data.size()) {
            return false;
        }
    }

    data = MimeMessagePrivate::encode(QByteArrayLiteral("From: "), QList<EmailAddress>() << d->sender, d->encoding);
    if (device->write(data) != data.size()) {
        return false;
    }

    if (d->replyTo.address().isEmpty() == false) {
        data = MimeMessagePrivate::encode(QByteArrayLiteral("Reply-To: "), QList<EmailAddress>() << d->replyTo, d->encoding);
        if (device->write(data) != data.size()) {
            return false;
        }
    }

    data = MimeMessagePrivate::encode(QByteArrayLiteral("To: "), d->recipientsTo, d->encoding);
    if (device->write(data) != data.size()) {
        return false;
    }

    data = MimeMessagePrivate::encode(QByteArrayLiteral("Cc: "), d->recipientsCc, d->encoding);
    if (device->write(data) != data.size()) {
        return false;
    }

    data = QByteArrayLiteral("Date: ") + QDateTime::currentDateTime().toString(Qt::RFC2822Date).toLatin1() + QByteArrayLiteral("\r\n");
    if (device->write(data) != data.size()) {
        return false;
    }

    data = QByteArrayLiteral("Subject: ") + MimeMessagePrivate::encodeData(d->encoding, d->subject, true);
    if (device->write(data) != data.size()) {
        return false;
    }

    data = QByteArrayLiteral("\r\nMIME-Version: 1.0\r\n");
    if (device->write(data) != data.size()) {
        return false;
    }

    if (!d->content->write(device)) {
        return false;
    }

    // Send \r\n.\r\n to end the mail data
    device->write(QByteArrayLiteral("\r\n.\r\n"));

    return true;
}

void MimeMessage::setSender(const EmailAddress &sender)
{
//    Q_D(MimeMessage);
    d->sender = sender;
}

void MimeMessage::setToRecipients(const QList<EmailAddress> &toList)
{
//    Q_D(MimeMessage);
    d->recipientsTo = toList;
}

QList<EmailAddress> MimeMessage::toRecipients() const
{
//    Q_D(const MimeMessage);
    return d->recipientsTo;
}

void MimeMessage::addTo(const EmailAddress &rcpt)
{
//    Q_D(MimeMessage);
    d->recipientsTo.append(rcpt);
}

void MimeMessage::setCcRecipients(const QList<EmailAddress> &ccList)
{
//    Q_D(MimeMessage);
    d->recipientsCc = ccList;
}

void MimeMessage::addCc(const EmailAddress &rcpt)
{
//    Q_D(MimeMessage);
    d->recipientsCc.append(rcpt);
}

QList<EmailAddress> MimeMessage::ccRecipients() const
{
//    Q_D(const MimeMessage);
    return d->recipientsCc;
}

void MimeMessage::setBccRecipients(const QList<EmailAddress> &bccList)
{
//    Q_D(MimeMessage);
    d->recipientsBcc = bccList;
}

QList<EmailAddress> MimeMessage::bccRecipients() const
{
//    Q_D(const MimeMessage);
    return d->recipientsBcc;
}

void MimeMessage::addBcc(const EmailAddress &rcpt)
{
//    Q_D(MimeMessage);
    d->recipientsBcc.append(rcpt);
}

void MimeMessage::setSubject(const QString & subject)
{
//    Q_D(MimeMessage);
    d->subject = subject;
}

void MimeMessage::addPart(MimePart *part)
{
//    Q_D(MimeMessage);
    qDebug() << "ADD PART" << part;

    auto content = d->content;
    if (typeid(*content) == typeid(MimeMultiPart)) {
        qDebug() << "ADD PART" << part;
        static_cast<MimeMultiPart*>(d->content)->addPart(part);
    }
}

void MimeMessage::setHeaderEncoding(MimePart::Encoding hEnc)
{
//    Q_D(MimeMessage);
    d->encoding = hEnc;
}

void MimeMessage::addHeader(const QByteArray &headerName, const QByteArray &headerValue)
{
//    Q_D(MimeMessage);
    d->listExtraHeaders.append(headerName + ":" + headerValue);
}

QList<QByteArray> MimeMessage::getHeaders() const
{
//    Q_D(const MimeMessage);
    return d->listExtraHeaders;
}

EmailAddress MimeMessage::sender() const
{
//    Q_D(const MimeMessage);
    return d->sender;
}

void MimeMessage::setReplyto(const EmailAddress &replyTo)
{
//    Q_D(MimeMessage);
    d->replyTo = replyTo;
}

QString MimeMessage::subject() const
{
//    Q_D(const MimeMessage);
    return d->subject;
}

QList<MimePart*> MimeMessage::parts() const
{
//    Q_D(const MimeMessage);

    QList<MimePart*> ret;
    auto content = d->content;
    if (typeid(*content) == typeid(MimeMultiPart)) {
        ret = static_cast<MimeMultiPart*>(d->content)->parts();
    } else {
        ret.append(d->content);
    }
    qDebug() << Q_FUNC_INFO << "parts" << (typeid(d->content) == typeid(MimeMultiPart)) << d.data();

    return ret;
}

//MimeMessagePrivate::MimeMessagePrivate(const MimeMessagePrivate &other) : QSharedData(other), content(other.content)
//{
//    qDebug() << Q_FUNC_INFO << this << content << other.content;
//}

MimeMessagePrivate::MimeMessagePrivate()
{
    qDebug() << Q_FUNC_INFO << this;
}

MimeMessagePrivate::~MimeMessagePrivate()
{
    qDebug() << Q_FUNC_INFO << this;
//    delete content;
}

QByteArray MimeMessagePrivate::encode(const QByteArray &addressKind, const QList<EmailAddress> &emails, MimePart::Encoding codec)
{
    if (emails.isEmpty()) {
        return QByteArray();
    }

    QByteArray mime = addressKind;
    bool first = true;
    for (const EmailAddress &email : emails) {
        if (!first) {
            mime.append(',');
        } else {
            first = false;
        }

        const QString name = email.name();
        if (!name.isEmpty()) {
            mime.append(MimeMessagePrivate::encodeData(codec, name, true));
            mime.append(" <" + email.address().toLatin1() + '>');
        } else {
            mime.append('<' + email.address().toLatin1() + '>');
        }
    }
    mime.append(QByteArrayLiteral("\r\n"));

    return mime;
}

QByteArray MimeMessagePrivate::encodeData(MimePart::Encoding codec, const QString &data, bool autoencoding)
{
    const QString simpleData = data.simplified();
    const QByteArray simple = simpleData.toUtf8();
    if (autoencoding) {
        if (simpleData.toLatin1() == simple) {
            return simple;
        }

        int printable = 0;
        int encoded = 0;
        const QByteArray result = QuotedPrintable::encode(simple, true, &printable, &encoded);
        int sum = printable + encoded;
        qCDebug(SIMPLEMAIL_MIMEMSG) << data << result << printable << encoded << sum << (double(printable)/sum) << (encoded/sum);
        if (sum != 0 && (double(printable)/sum) >= 0.8) {
            return " =?utf-8?Q?" + result + "?=";
        } else {
            return " =?utf-8?B?" + data.toUtf8().toBase64() + "?=";
        }
    } else {
        switch (codec) {
        case MimePart::Base64:
            return " =?utf-8?B?" + simple.toBase64() + "?=";
        case MimePart::QuotedPrintable:
            return " =?utf-8?Q?" + QuotedPrintable::encode(simple, true) + "?=";
        default:
            return ' ' + data.toLatin1();
        }
    }
}
