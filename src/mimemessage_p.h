/*
 * Copyright (C) 2015 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "mimemessage.h"
#include "mimemultipart.h"

#ifndef MIMEMESSAGE_P_H
#define MIMEMESSAGE_P_H

namespace SimpleMail {

class MimeMessagePrivate : public QSharedData
{
public:
    MimeMessagePrivate() = default;
    ~MimeMessagePrivate();

    inline static QByteArray encode(const QByteArray &addressKind, const QList<EmailAddress> &emails, MimePart::Encoding codec);
    inline static QByteArray encodeData(MimePart::Encoding codec, const QString &data, bool autoencoding);

    QList<EmailAddress> recipientsTo;
    QList<EmailAddress> recipientsCc;
    QList<EmailAddress> recipientsBcc;
    QString subject;
    EmailAddress sender;
    MimePart *content = nullptr;
    MimePart::Encoding encoding = MimePart::_8Bit;
    bool autoMimeContentCreated;
    EmailAddress replyTo;
    QList<QByteArray> listExtraHeaders;
};

}

#endif // MIMEMESSAGE_P_H

