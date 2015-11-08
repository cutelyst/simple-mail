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

#include <QFileInfo>

MimeFile::MimeFile(QFile *file)
{
    Q_D(MimePart);
    d->contentFile = file;
    d->cType = QStringLiteral("application/octet-stream");
    d->cName = QFileInfo(*file).fileName();
    d->cEncoding = Base64;
}

MimeFile::MimeFile(const QByteArray &stream, const QString &fileName)
{
    Q_D(MimePart);
    d->cEncoding = Base64;
    d->cType = QStringLiteral("application/octet-stream");
    d->cName = fileName;
    d->content = stream;
}

MimeFile::~MimeFile()
{
}

void MimeFile::prepare()
{
    Q_D(MimePart);

    if (d->contentFile) {
        d->contentFile->open(QIODevice::ReadOnly);
        d->content = d->contentFile->readAll();
        d->contentFile->close();
    }

    /* !!! IMPORTANT !!!! */
    MimePart::prepare();
}
