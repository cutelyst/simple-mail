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

#include <QtCore/QIODevice>
#include <QtCore/QBuffer>

MimePart::MimePart() : d_ptr(new MimePartPrivate)
{
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
    if (d->contentDevice) {
        delete d->contentDevice;
    }
    d->contentDevice = new QBuffer;
    d->contentDevice->open(QBuffer::ReadWrite);
    d->contentDevice->write(content);
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
    if (d->contentDevice) {
        return d->contentDevice->readAll();
    }
    return QByteArray();
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

void MimePart::setData(const QString &data)
{
    Q_D(MimePart);

    if (d->contentDevice) {
        delete d->contentDevice;
    }
    d->contentDevice = new QBuffer;
    d->contentDevice->open(QBuffer::ReadWrite);

    switch (d->cEncoding) {
    case _7Bit:
        d->contentDevice->write(data.toLatin1());
        break;
    case _8Bit:
        d->contentDevice->write(data.toUtf8());
        break;
    case Base64:
        d->contentDevice->write(d->formatter.format(data.toUtf8().toBase64()));
        break;
    case QuotedPrintable:
        d->contentDevice->write(d->formatter.format(QuotedPrintable::encode(data.toUtf8()), true));
        break;
    }
}

QString MimePart::data() const
{
    Q_D(const MimePart);

    if (!d->contentDevice || !d->contentDevice->seek(0)) {
        return QString();
    }

    QString ret;
    switch (d->cEncoding) {
    case _7Bit:
        ret = QString::fromLatin1(d->contentDevice->readAll());
    case _8Bit:
        ret = QString::fromUtf8(d->contentDevice->readAll());
    case Base64:
        ret = QString::fromUtf8(QByteArray::fromBase64(d->contentDevice->readAll()));
    case QuotedPrintable:
        ret = QString::fromUtf8(QuotedPrintable::decode(d->contentDevice->readAll()));
    }
    return ret;
}

MimeContentFormatter *MimePart::contentFormatter()
{
    Q_D(MimePart);
    return &d->formatter;
}

bool MimePart::write(QIODevice *device)
{
    Q_D(const MimePart);

    QByteArray headers;

    // Content-Type
    headers.append("Content-Type: " + d->cType);
    if (!d->cName.isEmpty()) {
        headers.append("; name=\"" + d->cName);
    }
    if (!d->cCharset.isEmpty()) {
        headers.append("; charset=" + d->cCharset);
    }
    if (!d->cBoundary.isEmpty()) {
        headers.append("; boundary=" + d->cBoundary);
    }
    headers.append("\r\n");

    // Content-Transfer-Encoding
    switch (d->cEncoding) {
    case _7Bit:
        headers.append("Content-Transfer-Encoding: 7bit\r\n");
        break;
    case _8Bit:
        headers.append("Content-Transfer-Encoding: 8bit\r\n");
        break;
    case Base64:
        headers.append("Content-Transfer-Encoding: base64\r\n");
        break;
    case QuotedPrintable:
        headers.append("Content-Transfer-Encoding: quoted-printable\r\n");
        break;
    }

    // Content-Id
    if (!d->cId.isNull()) {
        headers.append("Content-ID: <" + d->cId + ">\r\n");
    }

    // Addition header lines
    headers.append(d->header + "\r\n");

    // Write headers
    if (device->write(headers) != headers.size()) {
        return false;
    }

    // Write content data
    return writeData(device);
}

MimePart::MimePart(MimePartPrivate *d) : d_ptr(d)
{

}

bool MimePart::writeData(QIODevice *device)
{
    Q_D(MimePart);

    /* === Content === */
    QIODevice *input = d->contentDevice;
    if (!input->isOpen()) {
        if (!input->open(QIODevice::ReadOnly)) {
            return false;
        }
    } else if (!input->seek(0)) {
        return false;
    }

    char block[4096];
    qint64 totalRead = 0;
    while (!input->atEnd()) {
        qint64 in = input->read(block, sizeof(block));
        if (in <= 0) {
            break;
        }

        totalRead += in;
        if (in != device->write(block, in)) {
            return true;
        }
    }

    if (device->write("\r\n", 2) != 2) {
        return false;
    }

    return true;
}

MimePartPrivate *MimePart::d_func()
{
    return d_ptr.data();
}
