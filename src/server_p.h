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
    enum State {
        Initial,
        SendingCommands,
        SendingData,
    };

    ServerReplyContainer(const MimeMessage &email) : msg(email) {}

    MimeMessage msg;
    QPointer<ServerReply> reply;
    QByteArrayList commands;
    QList<int> awaitedCodes;
    State state = Initial;
};

class ServerPrivate
{
    Q_DECLARE_PUBLIC(Server)
public:
    enum State {
        Disconnected,
        Closing,
        Connecting,
        WaitingForServiceReady220,
        WaitingForServerCaps250,
        WaitingForAuthPlain235,
        WaitingForAuthLogin334_step1,
        WaitingForAuthLogin334_step2,
        WaitingForAuthLogin235_step3,
        WaitingForAuthCramMd5_334_step1,
        WaitingForAuthCramMd5_334_step2,
        Ready,
        SendingMail,
    };

    ServerPrivate(Server *srv) : q_ptr(srv) { }
    inline void createSocket();
    void login();
    void processNextMail();

    bool parseResponseCode(int expectedCode);
    int parseCaps();
    inline void commandReset();
    inline void commandNoop();
    inline void commandQuit();

    QList<ServerReplyContainer> queue;
    Server *q_ptr;
    QTcpSocket *socket = nullptr;
    QStringList caps;
    QString host;
    QString hostname;
    QString username;
    QString password;
    quint16 port = 25;
    Server::ConnectionType ct = Server::TcpConnection;
    Server::AuthMethod authMethod = Server::AuthNone;
    State state = Disconnected;
    QList<int> awaitedCodes;
    bool capPipelining = false;
};

}

#endif // SERVER_P_H
