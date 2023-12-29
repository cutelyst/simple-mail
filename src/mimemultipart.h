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

#include "mimepart.h"
#include "smtpexports.h"

#include <memory>

#include <QtCore/QList>

namespace SimpleMail {

class SMTP_EXPORT MimeMultiPart : public MimePart
{
public:
    enum MultiPartType {
        Mixed       = 0, // RFC 2046, section 5.1.3
        Digest      = 1, // RFC 2046, section 5.1.5
        Alternative = 2, // RFC 2046, section 5.1.4
        Related     = 3, // RFC 2387
        Report      = 4, // RFC 6522
        Signed      = 5, // RFC 1847, section 2.1
        Encrypted   = 6  // RFC 1847, section 2.2
    };

    MimeMultiPart(const MultiPartType type = Related);
    virtual ~MimeMultiPart();

    void setMimeType(const MultiPartType type);
    MultiPartType mimeType() const;

    QList<std::shared_ptr<MimePart>> parts() const;
    void addPart(const std::shared_ptr<MimePart> &part);

protected:
    virtual bool writeData(QIODevice *device) Q_DECL_FINAL;
};

} // namespace SimpleMail
