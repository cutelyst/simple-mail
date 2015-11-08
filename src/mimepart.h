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

#ifndef MIMEPART_H
#define MIMEPART_H

#include <QtCore/QSharedDataPointer>

#include "mimecontentformatter.h"

#include "smtpexports.h"

class MimePartPrivate;
class SMTP_EXPORT MimePart
{
public:
    enum Encoding {        
        _7Bit,
        _8Bit,
        Base64,
        QuotedPrintable
    };

    MimePart();
    MimePart(const MimePart &other);
    virtual ~MimePart();

    MimePart &operator=(const MimePart &other);

    QString header() const;
    QByteArray content() const;

    void setContent(const QByteArray &content);
    void setHeader(const QString &header);

    void addHeaderLine(const QString &line);

    void setContentId(const QString &cId);
    QString contentId() const;

    void setContentName(const QString &name);
    QString contentName() const;

    void setContentType(const QString &cType);
    QString contentType() const;

    void setCharset(const QString &charset);
    QString charset() const;

    void setEncoding(Encoding enc);
    Encoding encoding() const;

    MimeContentFormatter *contentFormatter();

    virtual QByteArray data();

    virtual void prepare();

protected:
    MimePart(MimePartPrivate *d);

    QSharedDataPointer<MimePartPrivate> d_ptr;

    // Q_DECLARE_PRIVATE equivalent for shared data pointers
    MimePartPrivate* d_func();
    inline const MimePartPrivate* d_func() const
    {
        return d_ptr.constData();
    }
};

#endif // MIMEPART_H
