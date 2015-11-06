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

#include "mimetext.h"
#include "mimepart_p.h"

MimeText::MimeText(const QString &txt)
{
    this->m_text = txt;
    d_ptr->cType = QStringLiteral("text/plain");
    d_ptr->cCharset = QStringLiteral("utf-8");
    d_ptr->cEncoding = _8Bit;
}

MimeText::~MimeText()
{

}

void MimeText::setText(const QString &text)
{
    this->m_text = text;
}

QString MimeText::text() const
{
    return m_text;
}

void MimeText::prepare()
{
    d_ptr->content = m_text.toUtf8();

    /* !!! IMPORTANT !!! */
    MimePart::prepare();
}
