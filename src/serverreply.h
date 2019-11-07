/*
  Copyright (C) 2019 Daniel Nicoletti <dantti12@gmail.com>

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
#ifndef SERVERREPLY_H
#define SERVERREPLY_H

#include <QObject>

#include "smtpexports.h"

namespace SimpleMail {

class ServerReplyPrivate;
class SMTP_EXPORT ServerReply : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ServerReply)
public:
    explicit ServerReply(QObject *parent = nullptr);
    virtual ~ServerReply();

    bool error() const;
    QString errorText() const;

Q_SIGNALS:
    void finished();

private:
    ServerReplyPrivate *d_ptr;
};

}

#endif // SERVERREPLY_H
