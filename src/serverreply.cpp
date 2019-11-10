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
#include "serverreply.h"
#include "serverreply_p.h"

using namespace SimpleMail;

ServerReply::ServerReply(QObject *parent) : QObject(parent)
  , d_ptr(new ServerReplyPrivate)
{

}

ServerReply::~ServerReply()
{
    delete d_ptr;
}

bool ServerReply::error() const
{
    Q_D(const ServerReply);
    return d->error;
}

int ServerReply::responseCode() const
{
    Q_D(const ServerReply);
    return d->responseCode;
}

QString ServerReply::responseText() const
{
    Q_D(const ServerReply);
    return d->responseText;
}

void ServerReply::finish(bool error, int responseCode, const QString &responseText)
{
    Q_D(ServerReply);
    d->error = error;
    d->responseCode = responseCode;
    d->responseText = responseText;
    Q_EMIT finished();
}

#include "moc_serverreply.cpp"
