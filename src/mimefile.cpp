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
    this->file = file;
    d_ptr->cType = QStringLiteral("application/octet-stream");
    d_ptr->cName = QFileInfo(*file).fileName();
    d_ptr->cEncoding = Base64;
}

MimeFile::MimeFile(const QByteArray &stream, const QString &fileName)
{
    d_ptr->cEncoding = Base64;
    d_ptr->cType = QStringLiteral("application/octet-stream");
    d_ptr->cName = fileName;
    d_ptr->content = stream;
}

MimeFile::~MimeFile()
{
    delete file;
}

void MimeFile::prepare()
{
    if (this->file) {
        file->open(QIODevice::ReadOnly);
        d_ptr->content = file->readAll();
        file->close();
    }

    /* !!! IMPORTANT !!!! */
    MimePart::prepare();
}
