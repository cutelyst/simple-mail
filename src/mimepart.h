/*
  Copyright (c) 2011-2012 - Tőkés Attila
  Copyright (C) 2015-2023 Daniel Nicoletti <dantti12@gmail.com>

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
#pragma once

#include "mimecontentformatter.h"
#include "smtpexports.h"

#include <QtCore/QMetaType>
#include <QtCore/QSharedDataPointer>

class QIODevice;
namespace SimpleMail {

class MimePartPrivate;
class SMTP_EXPORT MimePart
{
public:
    enum Encoding { _7Bit, _8Bit, Base64, QuotedPrintable };

    MimePart();
    MimePart(const MimePart &other);
    virtual ~MimePart();

    MimePart &operator=(const MimePart &other);

    QByteArray header() const;
    QByteArray content() const;

    void setContent(const QByteArray &content);
    void setHeader(const QByteArray &header);

    void addHeaderLine(const QByteArray &line);

    void setContentId(const QByteArray &cId);
    QByteArray contentId() const;

    void setContentName(const QByteArray &contentName);
    QByteArray contentName() const;

    void setContentType(const QByteArray &contentType);
    QByteArray contentType() const;

    void setCharset(const QByteArray &charset);
    QByteArray charset() const;

    void setEncoding(Encoding enc);
    Encoding encoding() const;

    void setData(const QString &data);
    QString data() const;

    MimeContentFormatter *contentFormatter();

    bool write(QIODevice *device);

protected:
    MimePart(MimePartPrivate *d);
    virtual bool writeData(QIODevice *device);

    QSharedDataPointer<MimePartPrivate> d_ptr;

    // Q_DECLARE_PRIVATE equivalent for shared data pointers
    MimePartPrivate *d_func();
    inline const MimePartPrivate *d_func() const { return d_ptr.constData(); }
};

} // namespace SimpleMail

Q_DECLARE_METATYPE(SimpleMail::MimePart *)
