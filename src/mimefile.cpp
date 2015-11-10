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

#include "mimefile.h"
#include "mimepart_p.h"

#include <QtCore/QBuffer>
#include <QtCore/QMimeDatabase>
#include <QtCore/QFileInfo>

using namespace SimpleMail;

MimeFile::MimeFile(QFile *file)
{
    Q_D(MimePart);
    d->contentEncoding = Base64;
    d->contentDevice = file;

    const QString filename = QFileInfo(*file).fileName();
    d->contentName = filename.toLatin1();

    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForFile(file->fileName());
    d->contentType = mime.name().toLatin1();
    if (d->contentType.isEmpty()) {
        d->contentType = QByteArrayLiteral("application/octet-stream");
    }
}

MimeFile::MimeFile(const QByteArray &stream, const QString &fileName)
{
    Q_D(MimePart);
    d->contentEncoding = Base64;
    d->contentName = fileName.toLatin1();
    d->contentType = QByteArrayLiteral("application/octet-stream");
    setContent(stream);
}

MimeFile::~MimeFile()
{
}
