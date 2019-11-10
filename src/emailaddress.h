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

#ifndef EMAILADDRESS_H
#define EMAILADDRESS_H

#include <QtCore/QSharedDataPointer>
#include <QString>

#include "smtpexports.h"

namespace SimpleMail {

class EmailAddressPrivate;
class SMTP_EXPORT EmailAddress
{
public:
    EmailAddress();
    EmailAddress(const EmailAddress &other);
    EmailAddress(const QString &nameAndAddress);
    EmailAddress(const QString &address, const QString &name);
    virtual ~EmailAddress();

    EmailAddress &operator=(const EmailAddress &other);

    QString name() const;
    void setName(const QString &name);

    QString address() const;
    void setAddress(const QString &address);

protected:
    QSharedDataPointer<EmailAddressPrivate> d_ptr;

private:
    // Q_DECLARE_PRIVATE equivalent for shared data pointers
    EmailAddressPrivate* d_func();
    inline const EmailAddressPrivate* d_func() const
    {
        return d_ptr.constData();
    }
};

}

#endif // EMAILADDRESS_H
