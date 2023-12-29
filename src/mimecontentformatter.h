/*
  Copyright (c) 2011-2012 - Tőkés Attila
  Copyright (C) 2015-2023 Daniel Nicoletti <dantti12@gmail.com>

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
#pragma once

#include "smtpexports.h"

#include <QByteArray>

namespace SimpleMail {

class SMTP_EXPORT MimeContentFormatter
{
public:
    explicit MimeContentFormatter(int max_length = 76);

    void setMaxLength(int l);
    int maxLength() const;

    QByteArray format(const QByteArray &content, int &chars) const;
    QByteArray formatQuotedPrintable(const QByteArray &content, int &chars) const;

protected:
    int max_length;
};

} // namespace SimpleMail
