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
#include "mimetext_p.h"

MimeHtml::MimeHtml(const QString &html) : MimeText(html)
{
    Q_D(MimePart);
    d->cType = QStringLiteral("text/html");
}

MimeHtml::~MimeHtml()
{

}

void MimeHtml::setHtml(const QString &html)
{
    Q_D(MimePart);
    static_cast<MimeTextPrivate*>(d)->text = html;
}

QString MimeHtml::html() const
{
    Q_D(const MimePart);
    return static_cast<const MimeTextPrivate*>(d)->text;
}

void MimeHtml::prepare()
{
    /* !!! IMPORTANT !!! */
    MimeText::prepare();
}
