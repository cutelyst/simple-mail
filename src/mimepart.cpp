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

void MimePart::setHeader(const QByteArray &header)
{
    Q_D(MimePart);
    d->header = header;
}

void MimePart::addHeaderLine(const QByteArray &line)
{
    Q_D(MimePart);
    d->header.append(line + "\r\n");
}

QByteArray MimePart::header() const
{
    Q_D(const MimePart);
    return d->header;
}

QByteArray MimePart::content() const
{
    Q_D(const MimePart);
    return d->content;
}

void MimePart::setContentId(const QByteArray &cId)
{
    Q_D(MimePart);
    d->cId = cId;
}

QByteArray MimePart::contentId() const
{
    Q_D(const MimePart);
    return d->cId;
}

void MimePart::setContentName(const QByteArray &name)
{
    Q_D(MimePart);
    d->cName = name;
}

QByteArray MimePart::contentName() const
{
    Q_D(const MimePart);
    return d->cName;
}

void MimePart::setContentType(const QByteArray &cType)
{
    Q_D(MimePart);
    d->cType = cType;
}

QByteArray MimePart::contentType() const
{
    Q_D(const MimePart);
    return d->cType;
}

void MimePart::setCharset(const QByteArray &charset)
{
    Q_D(MimePart);
    d->cCharset = charset;
}

QByteArray MimePart::charset() const
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

QByteArray MimePart::data()
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

    QByteArray mimeString;

    /* === Header Prepare === */

    /* Content-Type */
    mimeString.append("Content-Type: " + d->cType);

    if (!d->cName.isEmpty()) {
        mimeString.append("; name=\"" + d->cName + "\"");
    }

    if (!d->cCharset.isEmpty()) {
        mimeString.append("; charset=" + d->cCharset);
    }

    if (!d->cBoundary.isEmpty()) {
        mimeString.append("; boundary=" + d->cBoundary);
    }

    mimeString.append("\r\n");

    // Content-Transfer-Encoding
    switch (d->cEncoding)
    {
    case _7Bit:
        mimeString.append("Content-Transfer-Encoding: 7bit\r\n");
        break;
    case _8Bit:
        mimeString.append("Content-Transfer-Encoding: 8bit\r\n");
        break;
    case Base64:
        mimeString.append("Content-Transfer-Encoding: base64\r\n");
        break;
    case QuotedPrintable:
        mimeString.append("Content-Transfer-Encoding: quoted-printable\r\n");
        break;
    }

    // Content-Id
    if (!d->cId.isNull()) {
        mimeString.append("Content-ID: <" + d->cId + ">\r\n");
    }

    // Addition header lines
    mimeString.append(d->header + "\r\n");

    /* === End of Header Prepare === */

    /* === Content === */
    switch (d->cEncoding) {
    case _7Bit:
        mimeString.append(d->content);
        break;
    case _8Bit:
        mimeString.append(d->content);
        break;
    case Base64:
        mimeString.append(d->formatter.format(d->content.toBase64()));
        break;
    case QuotedPrintable:
        mimeString.append(d->formatter.format(QuotedPrintable::encode(d->content), true));
        break;
    }
    mimeString.append("\r\n");

    // === End of Content ===
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
