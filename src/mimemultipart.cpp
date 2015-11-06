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

#include "mimemultipart.h"
#include "mimepart_p.h"

#include <QTime>
#include <QUuid>

const QString MULTI_PART_NAMES[] = {
    QStringLiteral("multipart/mixed"),         //    Mixed
    QStringLiteral("multipart/digest"),        //    Digest
    QStringLiteral("multipart/alternative"),   //    Alternative
    QStringLiteral("multipart/related"),       //    Related
    QStringLiteral("multipart/report"),        //    Report
    QStringLiteral("multipart/signed"),        //    Signed
    QStringLiteral("multipart/encrypted")      //    Encrypted
};

MimeMultiPart::MimeMultiPart(MultiPartType type)
{
    Q_D(MimePart);
    this->type = type;
    d->cType = MULTI_PART_NAMES[type];
    d->cEncoding = _8Bit;

    d->cBoundary = QString::fromLatin1(QUuid::createUuid().toRfc4122().toHex());
}

MimeMultiPart::~MimeMultiPart()
{

}

void MimeMultiPart::addPart(MimePart *part)
{
    parts.append(part);
}

const QList<MimePart*> & MimeMultiPart::getParts() const
{
    return parts;
}

void MimeMultiPart::prepare()
{
    Q_D(MimePart);

    QList<MimePart*>::iterator it;

    d->content = QByteArray();
    for (it = parts.begin(); it != parts.end(); it++) {
        d->content += "--" + d->cBoundary.toLatin1() + "\r\n";
        (*it)->prepare();
        d->content += (*it)->toString().toUtf8();
    };

    d->content += "--" + d->cBoundary.toLatin1() + "--\r\n";

    MimePart::prepare();
}

void MimeMultiPart::setMimeType(const MultiPartType type)
{
    Q_D(MimePart);
    d->cType = MULTI_PART_NAMES[type];
    this->type = type;
}

MimeMultiPart::MultiPartType MimeMultiPart::mimeType() const
{
    return type;
}
