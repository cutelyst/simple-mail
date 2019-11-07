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
#include "server_p.h"
#include "serverreply.h"

#include <QSslSocket>
#include <QTcpSocket>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(SIMPLEMAIL_SERVER, "simplemail.server")

using namespace SimpleMail;

Server::Server(QObject *parent)
    : QObject(parent)
    , d_ptr(new ServerPrivate)
{
//    Q_D(Server);

//    switch (ct) {
//    case TcpConnection:
//        d->socket = new QTcpSocket(this);
//        break;
//    case SslConnection:
//    case TlsConnection:
//        d->socket = new QSslSocket(this);
//        d->setPeerVerificationType(d->peerVerificationType);
//        connect(static_cast<QSslSocket*>(d->socket), static_cast<void(QSslSocket::*)(const QList<QSslError> &)>(&QSslSocket::sslErrors),this, &Sender::sslErrors, Qt::DirectConnection);
//    }
//    connect(d->socket, &QTcpSocket::stateChanged, this, &Sender::socketStateChanged);
//    connect(d->socket, static_cast<void(QTcpSocket::*)(QTcpSocket::SocketError)>(&QTcpSocket::error),
//            this, &Sender::socketError);
//    connect(d->socket, &QTcpSocket::readyRead, this, &Sender::socketReadyRead);
}

Server::~Server()
{
    delete d_ptr;
}

QString Server::host() const
{
    Q_D(const Server);
    return d->host;
}

void Server::setHost(const QString &host)
{
    Q_D(Server);
    d->host = host;
}

quint16 Server::port() const
{
    Q_D(const Server);
    return d->port;
}

void Server::setPort(quint16 port)
{
    Q_D(Server);
    d->port = port;
}

QString Server::hostname() const
{
    Q_D(const Server);
    return d->hostname;
}

void Server::setHostname(const QString &hostname)
{
    Q_D(Server);
    d->hostname = hostname;
}

Server::ConnectionType Server::connectionType() const
{
    Q_D(const Server);
    return d->ct;
}

void Server::setConnectionType(Server::ConnectionType ct)
{
    Q_D(Server);
    d->ct = ct;
}

QString Server::username() const
{
    Q_D(const Server);
    return d->username;
}

void Server::setUsername(const QString &username)
{
    Q_D(Server);
    d->username = username;
}

QString Server::password() const
{
    Q_D(const Server);
    return d->password;
}

void Server::setPassword(const QString &password)
{
    Q_D(Server);
    d->password = password;
}

Server::AuthMethod Server::authMethod() const
{
    Q_D(const Server);
    return d->authMethod;
}

void Server::setAuthMethod(Server::AuthMethod method)
{
    Q_D(Server);
    d->authMethod = method;
}

ServerReply *Server::sendMail(const MimeMessage &msg)
{
    Q_D(Server);
    ServerReplyContainer cont;
    cont.reply = new ServerReply(this);
    cont.msg = msg;
    d->queue.append(cont);

    return cont.reply.data();
}

void Server::connectToServer()
{
    Q_D(Server);

    QSslSocket *sslSock = nullptr;
    switch (d->ct) {
    case Server::TlsConnection:
    case Server::TcpConnection:
        qCDebug(SIMPLEMAIL_SERVER) << "Connecting to host" << d->host << d->port;
//        d->socket->connectToHost(d->host, d->port);
        break;
    case Server::SslConnection:
    {
//        sslSock = qobject_cast<QSslSocket*>(socket);
//        if (sslSock) {
//            qCDebug(SIMPLEMAIL_SERVER) << "Connecting to host encrypted" << host << port;
//            sslSock->connectToHostEncrypted(host, port);
//        } else {
//            return false;
//        }
    }
        break;
    }
}

#include "moc_server.cpp"
