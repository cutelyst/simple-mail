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
#ifndef SERVER_P_H
#define SERVER_P_H

#include "server.h"
#include "mimemessage.h"

#include <QPointer>

class QTcpSocket;

namespace SimpleMail {

class ServerReply;
class ServerReplyContainer {
public:
    QPointer<ServerReply> reply;
    MimeMessage msg;
    bool sent = false;
};

class ServerPrivate
{
//    Q_DECLARE_PUBLIC(Serder)
public:
    QTcpSocket *socket = nullptr;
    QString host;
    QString hostname;
    QString username;
    QString password;
    quint16 port = 25;
    Server::ConnectionType ct = Server::TcpConnection;
    Server::AuthMethod authMethod = Server::AuthNone;
    QList<ServerReplyContainer> queue;
};

}

#endif // SERVER_P_H
