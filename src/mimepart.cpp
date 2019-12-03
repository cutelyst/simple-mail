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
#include <QtCore/QDebug>

using namespace SimpleMail;

MimePart::MimePart() : d_ptr(new MimePartPrivate)
{
}

MimePart::MimePart(const MimePart &other)
{
    Q_D(MimePart);
    d->contentCharset = other.charset();

    if (d->contentDevice) {
        delete d->contentDevice;
    }
    d->contentDevice = new QBuffer;
    d->contentDevice->open(QBuffer::ReadWrite);
    d->contentDevice->write(other.content());

    d->contentId = other.contentId();
    d->contentName = other.contentName();
    d->contentType = other.contentType();
    d->contentEncoding = other.encoding();
    d->header = other.header();
}

MimePart::~MimePart()
{
}

MimePart &MimePart::operator=(const MimePart &other)
{
    Q_D(MimePart);
    d->contentCharset = other.charset();

    if (d->contentDevice) {
        delete d->contentDevice;
    }
    d->contentDevice = new QBuffer;
    d->contentDevice->open(QBuffer::ReadWrite);
    d->contentDevice->write(other.content());

    d->contentId = other.contentId();
    d->contentName = other.contentName();
    d->contentType = other.contentType();
    d->contentEncoding = other.encoding();
    d->header = other.header();

    return *this;
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
    if (d->contentDevice && d->contentDevice->seek(0)) {
        return d->contentDevice->readAll();
    }
    return QByteArray();
}

void MimePart::setContentId(const QByteArray &cId)
{
    Q_D(MimePart);
    d->contentId = cId;
}

QByteArray MimePart::contentId() const
{
    Q_D(const MimePart);
    return d->contentId;
}

void MimePart::setContentName(const QByteArray &contentName)
{
    Q_D(MimePart);
    d->contentName = contentName;
}

QByteArray MimePart::contentName() const
{
    Q_D(const MimePart);
    return d->contentName;
}

void MimePart::setContentType(const QByteArray &contentType)
{
    Q_D(MimePart);
    d->contentType = contentType;
}

QByteArray MimePart::contentType() const
{
    Q_D(const MimePart);
    return d->contentType;
}

void MimePart::setCharset(const QByteArray &charset)
{
    Q_D(MimePart);
    d->contentCharset = charset;
}

QByteArray MimePart::charset() const
{
    Q_D(const MimePart);
    return d->contentCharset;
}

void MimePart::setEncoding(Encoding enc)
{
    Q_D(MimePart);
    d->contentEncoding = enc;
}

MimePart::Encoding MimePart::encoding() const
{
    Q_D(const MimePart);
    return d->contentEncoding;
}

void MimePart::setData(const QString &data)
{
    Q_D(MimePart);

    if (d->contentDevice) {
        delete d->contentDevice;
    }
    d->contentDevice = new QBuffer;
    d->contentDevice->open(QBuffer::ReadWrite);

    switch (d->contentEncoding) {
    case _7Bit:
        d->contentDevice->write(data.toLatin1());
        break;
    case _8Bit:
    case Base64:
    case QuotedPrintable:
        d->contentDevice->write(data.toUtf8());
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
    switch (d->contentEncoding) {
    case _7Bit:
        ret = QString::fromLatin1(d->contentDevice->readAll());
        break;
    case _8Bit:
        ret = QString::fromUtf8(d->contentDevice->readAll());
        break;
    case Base64:
        ret = QString::fromUtf8(QByteArray::fromBase64(d->contentDevice->readAll()));
        break;
    case QuotedPrintable:
        ret = QString::fromUtf8(QuotedPrintable::decode(d->contentDevice->readAll()));
        break;
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
    headers.append("Content-Type: " + d->contentType);
    if (!d->contentName.isEmpty()) {
        headers.append("; name=\"" + d->contentName + "\"");
    }
    if (!d->contentCharset.isEmpty()) {
        headers.append("; charset=" + d->contentCharset);
    }
    if (!d->contentBoundary.isEmpty()) {
        headers.append("; boundary=" + d->contentBoundary);
    }
    headers.append("\r\n");

    // Content-Transfer-Encoding
    switch (d->contentEncoding) {
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
    if (!d->contentId.isNull()) {
        headers.append("Content-ID: <" + d->contentId + ">\r\n");
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

    switch (d->contentEncoding) {
    case MimePart::_7Bit:
    case MimePart::_8Bit:
        if (!d->writeRaw(input, device)) {
            return false;
        }
        break;
    case MimePart::Base64:
        if (!d->writeBase64(input, device)) {
            return false;
        }
        break;
    case MimePart::QuotedPrintable:
        if (!d->writeQuotedPrintable(input, device)) {
            return false;
        }
        break;
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

MimePartPrivate::~MimePartPrivate()
{
    delete contentDevice;
}

bool MimePartPrivate::writeRaw(QIODevice *input, QIODevice *out)
{
    char block[4096];
    qint64 totalRead = 0;
    while (!input->atEnd()) {
        qint64 in = input->read(block, sizeof(block));
        if (in <= 0) {
            break;
        }

        totalRead += in;
        if (in != out->write(block, in)) {
            return false;
        }
    }
    return true;
}

bool MimePartPrivate::writeBase64(QIODevice *input, QIODevice *out)
{
    char block[6000]; // Must be powers of 6
    qint64 totalRead = 0;
    int chars = 0;
    while (!input->atEnd()) {
        qint64 in = input->read(block, sizeof(block));
        if (in <= 0) {
            break;
        }

        totalRead += in;
      
        // removed QByteArray::OmitTrailingEquals flag to generate ending == to ensure compatability with Amazon SES
        QByteArray encoded = QByteArray(block, int(in)).toBase64(QByteArray::Base64Encoding);
        encoded = formatter.format(encoded, chars);
        if (encoded.size() != out->write(encoded)) {
            return false;
        }
    }
    return true;
}

bool MimePartPrivate::writeQuotedPrintable(QIODevice *input, QIODevice *out)
{
    char block[4096];
    qint64 totalRead = 0;
    int chars = 0;
    while (!input->atEnd()) {
        qint64 in = input->read(block, sizeof(block));
        if (in <= 0) {
            break;
        }

        totalRead += in;
        QByteArray encoded = QuotedPrintable::encode(QByteArray(block, int(in)), false);
        encoded = formatter.formatQuotedPrintable(encoded, chars);
        if (encoded.size() != out->write(encoded)) {
            return false;
        }
    }
    return true;
}
