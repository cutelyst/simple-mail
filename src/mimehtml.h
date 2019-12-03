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

#ifndef MIMEHTML_H
#define MIMEHTML_H

#include "mimetext.h"

#include "smtpexports.h"

namespace SimpleMail {

class SMTP_EXPORT MimeHtml : public MimeText
{
public:
    MimeHtml(const QString &html = QString());
    virtual ~MimeHtml();

    void setHtml(const QString &html);
    QString html() const;
};

}

#endif // MIMEHTML_H
