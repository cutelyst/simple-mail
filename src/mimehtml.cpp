/*
  Copyright (c) 2011-2012 - Tőkés Attila

  This file is part of SmtpClient for Qt.

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

#include "mimehtml.h"
#include "mimepart_p.h"

using namespace SimpleMail;

MimeHtml::MimeHtml(const QString &html) : MimeText(html)
{
    Q_D(MimePart);
    d->contentType = QByteArrayLiteral("text/html");
}

MimeHtml::~MimeHtml()
{

}

void MimeHtml::setHtml(const QString &html)
{
    setText(html);
}

QString MimeHtml::html() const
{
    return text();
}
