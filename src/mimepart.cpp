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

#include "mimepart_p.h"
#include "quotedprintable.h"

MimePart::MimePart() : d_ptr(new MimePartPrivate)
{
    Q_D(MimePart);
    d->cEncoding = _7Bit;
    d->prepared = false;
}

MimePart::MimePart(const MimePart &other)
{

}

MimePart::~MimePart()
{

}

void MimePart::setContent(const QByteArray &content)
{
    Q_D(MimePart);
    d->content = content;
}

void MimePart::setHeader(const QString &header)
{
    Q_D(MimePart);
    d->header = header;
}

void MimePart::addHeaderLine(const QString &line)
{
    Q_D(MimePart);
    d->header.append(line % QLatin1String("\r\n"));
}

QString MimePart::header() const
{
    Q_D(const MimePart);
    return d->header;
}

QByteArray MimePart::content() const
{
    Q_D(const MimePart);
    return d->content;
}

void MimePart::setContentId(const QString &cId)
{
    Q_D(MimePart);
    d->cId = cId;
}

QString MimePart::contentId() const
{
    Q_D(const MimePart);
    return d->cId;
}

void MimePart::setContentName(const QString &name)
{
    Q_D(MimePart);
    d->cName = name;
}

QString MimePart::contentName() const
{
    Q_D(const MimePart);
    return d->cName;
}

void MimePart::setContentType(const QString &cType)
{
    Q_D(MimePart);
    d->cType = cType;
}

QString MimePart::contentType() const
{
    Q_D(const MimePart);
    return d->cType;
}

void MimePart::setCharset(const QString &charset)
{
    Q_D(MimePart);
    d->cCharset = charset;
}

QString MimePart::charset() const
{
    Q_D(const MimePart);
    return d->cCharset;
}

void MimePart::setEncoding(Encoding enc)
{
    Q_D(MimePart);
    d->cEncoding = enc;
}

MimePart::Encoding MimePart::encoding() const
{
    Q_D(const MimePart);
    return d->cEncoding;
}

MimeContentFormatter *MimePart::contentFormatter()
{
    Q_D(MimePart);
    return &d->formatter;
}

QString MimePart::toString()
{
    Q_D(MimePart);

    if (!d->prepared) {
        prepare();
    }

    return d->mimeString;
}

void MimePart::prepare()
{
    Q_D(MimePart);

    QString mimeString;

    /* === Header Prepare === */

    /* Content-Type */
    mimeString.append(QLatin1String("Content-Type: ")).append(d->cType);

    if (!d->cName.isEmpty()) {
        mimeString.append(QLatin1String("; name=\"") % d->cName % QLatin1String("\""));
    }

    if (!d->cCharset.isEmpty()) {
        mimeString.append(QLatin1String("; charset=")).append(d->cCharset);
    }

    if (!d->cBoundary.isEmpty()) {
        mimeString.append(QLatin1String("; boundary=")).append(d->cBoundary);
    }

    mimeString.append(QLatin1String("\r\n"));
    /* ------------ */

    /* Content-Transfer-Encoding */
    mimeString.append(QLatin1String("Content-Transfer-Encoding: "));
    switch (d->cEncoding)
    {
    case _7Bit:
        mimeString.append(QLatin1String("7bit\r\n"));
        break;
    case _8Bit:
        mimeString.append(QLatin1String("8bit\r\n"));
        break;
    case Base64:
        mimeString.append(QLatin1String("base64\r\n"));
        break;
    case QuotedPrintable:
        mimeString.append(QLatin1String("quoted-printable\r\n"));
        break;
    }

    /* Content-Id */
    if (!d->cId.isNull()) {
        mimeString.append(QLatin1String("Content-ID: <")).append(d->cId).append(QLatin1String(">\r\n"));
    }

    /* Addition header lines */

    mimeString.append(d->header).append(QLatin1String("\r\n"));

    /* === End of Header Prepare === */

    /* === Content === */
    switch (d->cEncoding)
    {
    case _7Bit:
        mimeString.append(QString::fromLatin1(d->content));
        break;
    case _8Bit:
        mimeString.append(QString::fromLatin1(d->content));
        break;
    case Base64:
        mimeString.append(d->formatter.format(QString::fromLatin1(d->content.toBase64())));
        break;
    case QuotedPrintable:
        mimeString.append(d->formatter.format(QuotedPrintable::encode(d->content), true));
        break;
    }
    mimeString.append(QLatin1String("\r\n"));
    /* === End of Content === */

    d->prepared = true;
    d->mimeString = mimeString;
}

MimePart::MimePart(MimePartPrivate *d) : d_ptr(d)
{

}

MimePartPrivate *MimePart::d_func()
{
    return d_ptr.data();
}
