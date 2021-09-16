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

#ifndef MIMEFILE_H
#define MIMEFILE_H

#include "mimepart.h"
#include <QFile>

#include "smtpexports.h"

namespace SimpleMail {

class SMTP_EXPORT MimeFile : public MimePart
{
public:
    MimeFile(const QByteArray &stream, const QString &fileName);
    MimeFile(const QByteArray &stream, const QString &fileName, const QByteArray &mimeType);
    MimeFile(QFile *f);
    virtual ~MimeFile();
};

}

#endif // MIMEFILE_H
