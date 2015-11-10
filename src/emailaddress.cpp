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

#include "emailaddress_p.h"

using namespace SimpleMail;

EmailAddress::EmailAddress() : d_ptr(new EmailAddressPrivate)
{

}

EmailAddress::EmailAddress(const EmailAddress &other) : d_ptr(other.d_ptr)
{

}

EmailAddress::EmailAddress(const QString &address, const QString &name) : d_ptr(new EmailAddressPrivate)
{
    Q_D(EmailAddress);
    d->address = address;
    d->name = name;
}

EmailAddress::~EmailAddress()
{
}

EmailAddress &EmailAddress::operator=(const EmailAddress &other)
{
    d_ptr = other.d_ptr;
    return *this;
}

void EmailAddress::setName(const QString &name)
{
    Q_D(EmailAddress);
    d->name = name;
}
void EmailAddress::setAddress(const QString &address)
{
    Q_D(EmailAddress);
    d->address = address;
}

EmailAddressPrivate *EmailAddress::d_func()
{
    return d_ptr.data();
}

QString EmailAddress::name() const
{
    Q_D(const EmailAddress);
    return d->name;
}

QString EmailAddress::address() const
{
    Q_D(const EmailAddress);
    return d->address;
}
