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

#include "mimeattachment.h"

#include "mimepart_p.h"

#include <QtCore/QFileInfo>

using namespace SimpleMail;

MimeAttachment::MimeAttachment(std::shared_ptr<QFile> &&file)
    : MimeFile(std::move(file))
{
    Q_D(MimePart);
    const QString filename = QFileInfo(*file).fileName();
    d->header.append("Content-Disposition: attachment; filename=\"=?UTF-8?B?" +
                     filename.toUtf8().toBase64(QByteArray::Base64Encoding) + "?=\"\r\n");
}

MimeAttachment::MimeAttachment(const QByteArray &stream, const QString &fileName)
    : MimeFile(stream, fileName)
{
    Q_D(MimePart);
    d->header.append("Content-Disposition: attachment; filename=\"=?UTF-8?B?" +
                     fileName.toUtf8().toBase64(QByteArray::Base64Encoding) + "?=\"\r\n");
}

MimeAttachment::~MimeAttachment()
{
}
