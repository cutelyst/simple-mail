/*
  Copyright (C) 2019-2023 Daniel Nicoletti <dantti12@gmail.com>

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

#include <QObject>

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

    int responseCode() const;
    QString responseText() const;

Q_SIGNALS:
    void finished();

protected:
    void finish(bool error, int responseCode, const QString &responseText);

private:
    friend class ServerPrivate;

    ServerReplyPrivate *d_ptr;
};

} // namespace SimpleMail
