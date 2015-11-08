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

#include "mimemultipart_p.h"
#include <QTime>
#include <QUuid>

const QByteArray MULTI_PART_NAMES[] = {
    QByteArrayLiteral("multipart/mixed"),         //    Mixed
    QByteArrayLiteral("multipart/digest"),        //    Digest
    QByteArrayLiteral("multipart/alternative"),   //    Alternative
    QByteArrayLiteral("multipart/related"),       //    Related
    QByteArrayLiteral("multipart/report"),        //    Report
    QByteArrayLiteral("multipart/signed"),        //    Signed
    QByteArrayLiteral("multipart/encrypted")      //    Encrypted
};

MimeMultiPart::MimeMultiPart(MultiPartType type) : MimePart(new MimeMultiPartPrivate)
{
    Q_D(MimePart);
    static_cast<MimeMultiPartPrivate*>(d)->type = type;
    d->cType = MULTI_PART_NAMES[type];
    d->cEncoding = _8Bit;

    d->cBoundary = QUuid::createUuid().toRfc4122().toHex();
}

MimeMultiPart::~MimeMultiPart()
{

}

void MimeMultiPart::addPart(MimePart *part)
{
    Q_D(MimePart);
    static_cast<MimeMultiPartPrivate*>(d)->parts.append(part);
}

QList<MimePart*> MimeMultiPart::parts() const
{
    Q_D(const MimePart);
    return static_cast<const MimeMultiPartPrivate*>(d)->parts;
}

void MimeMultiPart::prepare()
{
    Q_D(MimePart);
    auto parts = static_cast<MimeMultiPartPrivate*>(d)->parts;
    d->content = QByteArray();
    Q_FOREACH (MimePart *part, parts) {
        d->content.append("--" + d->cBoundary + "\r\n");
        part->prepare();
        d->content.append(part->data());
    }
    d->content.append("--" + d->cBoundary + "--\r\n");

    MimePart::prepare();
}

void MimeMultiPart::setMimeType(const MultiPartType type)
{
    Q_D(MimePart);
    d->cType = MULTI_PART_NAMES[type];
    static_cast<MimeMultiPartPrivate*>(d)->type = type;
}

MimeMultiPart::MultiPartType MimeMultiPart::mimeType() const
{
    Q_D(const MimePart);
    return static_cast<const MimeMultiPartPrivate*>(d)->type;
}
