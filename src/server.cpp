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
#include <QHostInfo>
#include <QMessageAuthenticationCode>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(SIMPLEMAIL_SERVER, "simplemail.server")

using namespace SimpleMail;

Server::Server(QObject *parent)
    : QObject(parent)
    , d_ptr(new ServerPrivate(this))
{
    Q_D(Server);
    d->hostname = QHostInfo::localHostName();
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
    delete d->socket;
    d->socket = nullptr;
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
    if (d->authMethod == Server::AuthNone) {
        d->authMethod = Server::AuthPlain;
    }
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

ServerReply *Server::sendMail(const MimeMessage &email)
{
    Q_D(Server);
    ServerReplyContainer cont(email);
    cont.reply = new ServerReply(this);
//    cont.msg = email;

    // Send the MAIL command with the sender
    cont.commands << "MAIL FROM: <" + email.sender().address().toLatin1() + ">\r\n";
    cont.awaitedCodes << 250;

    // Send RCPT command for each recipient
    // To (primary recipients)
    const auto toRecipients = email.toRecipients();
    for (const EmailAddress &rcpt : toRecipients) {
        cont.commands << "RCPT TO: <" + rcpt.address().toLatin1() + ">\r\n";
        cont.awaitedCodes << 250;
    }

    // Cc (carbon copy)
    const auto ccRecipients = email.ccRecipients();
    for (const EmailAddress &rcpt : ccRecipients) {
        cont.commands << "RCPT TO: <" + rcpt.address().toLatin1() + ">\r\n";
        cont.awaitedCodes << 250;
    }

    // Bcc (blind carbon copy)
    const auto bccRecipients = email.bccRecipients();
    for (const EmailAddress &rcpt : bccRecipients) {
        cont.commands << "RCPT TO: <" + rcpt.address().toLatin1() + ">\r\n";
        cont.awaitedCodes << 250;
    }

    // DATA command
    cont.commands << QByteArrayLiteral("DATA\r\n");
    cont.awaitedCodes << 354;

    // Add to the mail queue
    d->queue.append(cont);

    if (d->state == ServerPrivate::Disconnected) {
        connectToServer();
    } else if (d->state == ServerPrivate::Ready) {
        d->processNextMail();
    }

    return cont.reply.data();
}

int Server::queueSize() const
{
    Q_D(const Server);
    return d->queue.size();
}

void Server::connectToServer()
{
    Q_D(Server);

    d->createSocket();

    switch (d->ct) {
    case Server::TlsConnection:
    case Server::TcpConnection:
        qCDebug(SIMPLEMAIL_SERVER) << "Connecting to host" << d->host << d->port;
        d->socket->connectToHost(d->host, d->port);
        d->state = ServerPrivate::Connecting;
        break;
    case Server::SslConnection:
    {
        auto sslSock = qobject_cast<QSslSocket*>(d->socket);
        if (sslSock) {
            qCDebug(SIMPLEMAIL_SERVER) << "Connecting to host encrypted" << d->host << d->port;
            sslSock->connectToHostEncrypted(d->host, d->port);
            d->state = ServerPrivate::Connecting;
        } else {
            return /*false*/;
        }
    }
        break;
    }
}

void ServerPrivate::createSocket()
{
    Q_Q(Server);

    if (socket) {
        return;
    }

    switch (ct) {
    case Server::TcpConnection:
        socket = new QTcpSocket(q);
        break;
    case Server::SslConnection:
    case Server::TlsConnection:
        socket = new QSslSocket(q);
        //        setPeerVerificationType(peerVerificationType);
        q->connect(static_cast<QSslSocket*>(socket), static_cast<void(QSslSocket::*)(const QList<QSslError> &)>(&QSslSocket::sslErrors),
                   q, &Server::sslErrors, Qt::DirectConnection);
    }
    q->connect(socket, &QTcpSocket::stateChanged, q, [=] (QAbstractSocket::SocketState sockState) {
        qDebug() << "stateChanged" << sockState << socket->readAll();
        if (sockState == QAbstractSocket::ClosingState) {
            state = Closing;
        } else if (sockState == QAbstractSocket::UnconnectedState) {
            state = Disconnected;
            awaitedCodes.clear();
        }
    });

    q->connect(socket, &QTcpSocket::connected, q, [=] () {
        qDebug() << "connected" << state << socket->readAll();
        state = WaitingForServiceReady220;
    });

    q->connect(socket, static_cast<void(QTcpSocket::*)(QTcpSocket::SocketError)>(&QTcpSocket::error),
               q, [=] (QAbstractSocket::SocketError error) {
        qDebug() << "SocketError" << error << socket->readAll();
    });

    q->connect(socket, &QTcpSocket::readyRead, q, [=] {
        qDebug() << "readyRead" << socket->bytesAvailable();
        switch (state) {
        case SendingMail:
            while (socket->canReadLine()) {
                if (!queue.isEmpty()) {
                    ServerReplyContainer &cont = queue[0];
                    if (cont.state == ServerReplyContainer::SendingCommands) {
                        if (cont.reply.isNull()) {
                            socket->disconnectFromHost();
                            return;
                        }

                        if (!cont.awaitedCodes.isEmpty()) {
                            const int awaitedCode = cont.awaitedCodes.takeFirst();
                            if (!parseResponseCode(awaitedCode)) {
                                qDebug() << "readyRead READY ERROR" << socket->readAll();
                                return;
                            }

                            if (!capPipelining && !cont.awaitedCodes.isEmpty()) {
                                // Write next command
                                socket->write(cont.commands[cont.commands.size() - cont.awaitedCodes.size()]);
                            }
                        }

                        if (cont.awaitedCodes.isEmpty()) {

                            cont.state = ServerReplyContainer::SendingData;
                            if (cont.msg.write(socket)) {
                                qCDebug(SIMPLEMAIL_SERVER) << "Mail sent";
                            } else {
                                qWarning() << "error writing mail";
                                return;
                            }
                        }
                    } else if (cont.state == ServerReplyContainer::SendingData) {
                        if (!parseResponseCode(250)) {
                            qDebug() << "readyRead READY ERROR" << socket->readAll();
                            return;
                        }

                        if (!cont.reply.isNull()) {
                            Q_EMIT cont.reply->finished();
                        }
                        queue.removeFirst();
                        processNextMail();
                    }
                } else {
                    state = Ready;
                    break;
                }
            }
            break;
        case WaitingForServerCaps250:
            while (socket->canReadLine()) {
                int ret = parseCaps();
                if (ret != 0 && ret == 1) {
                    qCDebug(SIMPLEMAIL_SERVER) << "CAPS" << caps;
                    capPipelining = caps.contains(QStringLiteral("250-PIPELINING"));
                    if (ct == Server::TlsConnection) {
                        qCDebug(SIMPLEMAIL_SERVER) << "TODO Sending STARTTLS";
                    } else {
                        login();
                    }
                    break;
                } else if (ret == -1) {
                    break;
                }
            }
            break;
        case WaitingForAuthPlain235:
            if (socket->canReadLine()) {
                if (parseResponseCode(235)) {
                    state = Ready;
                    processNextMail();
                }
            }
            break;
        case WaitingForServiceReady220:
            if (socket->canReadLine()) {
                if (parseResponseCode(220)) {
                    // The client's first command must be EHLO/HELO
                    socket->write("EHLO " + hostname.toLatin1() + "\r\n");
                    state = WaitingForServerCaps250;
                }
            }
            break;
        default:
            qDebug() << "readyRead unknown state" << socket->readAll() << state;
        }
        qDebug() << "readyRead" << socket->bytesAvailable();
    });
}

void ServerPrivate::login()
{
    qCDebug(SIMPLEMAIL_SERVER) << "LOGIN" << authMethod;
    if (authMethod == Server::AuthPlain) {
        qCDebug(SIMPLEMAIL_SERVER) << "Sending authentication plain" << state;

        // Sending command: AUTH PLAIN base64('\0' + username + '\0' + password)
        const QByteArray plain = '\0' + username.toUtf8() + '\0' + password.toUtf8();
        socket->write(QByteArrayLiteral("AUTH PLAIN ") + plain.toBase64() + "\r\n");
        state = WaitingForAuthPlain235;

        //        // If the response is not 235 then the authentication was faild
        //        if (!waitForResponse(235)) {
        //            Q_EMIT q->smtpError(Server::AuthenticationFailedError);
        //            return false;
        //        }
    } else if (authMethod == Server::AuthLogin) {
        // Sending command: AUTH LOGIN
        qCDebug(SIMPLEMAIL_SERVER) << "Sending authentication login";
        socket->write(QByteArrayLiteral("AUTH LOGIN\r\n"));
        state = WaitingForAuthLogin334_step1;

        // Wait for 334 response code
        //        if (!waitForResponse(334)) {
        //            Q_EMIT q->smtpError(Server::AuthenticationFailedError);
        //            return false;
        //        }

        // Send the username in base64
        qCDebug(SIMPLEMAIL_SERVER) << "Sending authentication user" << username;
        socket->write(username.toUtf8().toBase64() + "\r\n");

        // Wait for 334
        //        if (!waitForResponse(334)) {
        //            Q_EMIT q->smtpError(Server::AuthenticationFailedError);
        //            return false;
        //        }

        // Send the password in base64
        qCDebug(SIMPLEMAIL_SERVER) << "Sending authentication password";
        socket->write(password.toUtf8().toBase64() + "\r\n");

        // If the response is not 235 then the authentication was faild
        //        if (!waitForResponse(235)) {
        //            Q_EMIT q->smtpError(Server::AuthenticationFailedError);
        //            return false;
        //        }
    } else if (authMethod == Server::AuthCramMd5) {
        // NOTE Implementando - Ready
        qCDebug(SIMPLEMAIL_SERVER) << "Sending authentication CRAM-MD5";
        socket->write(QByteArrayLiteral("AUTH CRAM-MD5\r\n"));
        state = WaitingForAuthCramMd5_334_step1;

        // Wait for 334
        //        if (!waitForResponse(334)) {
        //            Q_EMIT q->smtpError(Server::AuthenticationFailedError);
        //            return false;
        //        }

        // Challenge
        //        QByteArray ch = QByteArray::fromBase64(responseText.mid((4)));

        // Calculamos el hash
        //        QMessageAuthenticationCode code(QCryptographicHash::Md5);
        //        code.setKey(password.toUtf8());
        //        code.addData(ch);

        //        QByteArray data(user.toLatin1() + " " + code.result().toHex());
        //        sendMessage(data.toBase64());

        // Wait for 334
        //        if (!waitForResponse(235)) {
        //            Q_EMIT q->smtpError(Server::AuthenticationFailedError);
        //            return false;
        //        }
    } else {
        state = ServerPrivate::Ready;
        processNextMail();
    }
}

void ServerPrivate::processNextMail()
{
     while (!queue.isEmpty()) {
        ServerReplyContainer &cont = queue[0];
        if (cont.reply.isNull()) {
            queue.removeFirst();
            continue;
        }

        if (cont.state == ServerReplyContainer::Initial) {
            qCDebug(SIMPLEMAIL_SERVER) << "Sending MAIL command" << capPipelining << cont.commands.size() << cont.commands << cont.awaitedCodes;
            if (capPipelining) {
                for (const QByteArray &cmd : cont.commands) {
                    socket->write(cmd);
                }
            } else {
                socket->write(cont.commands.first());
            }
            awaitedCodes.append(cont.awaitedCodes);

            state = SendingMail;
            cont.state = ServerReplyContainer::SendingCommands;
            return;
        } else {
            return;
        }
    };

    state = Ready;
}

bool ServerPrivate::parseResponseCode(int expectedCode)
{
    Q_Q(Server);

    // Save the server's response
    const QByteArray responseText = socket->readLine().trimmed();
    qCDebug(SIMPLEMAIL_SERVER) << "Got response" << responseText << "expected" << expectedCode;

    // Extract the respose code from the server's responce (first 3 digits)
    int responseCode = responseText.left(3).toInt();

    if (responseCode / 100 == 4) {
        //        lastError = QString::fromLatin1(responseText);
        Q_EMIT q->smtpError(Server::ServerError, QString::fromLatin1(responseText));
    }

    if (responseCode / 100 == 5) {
        //        lastError = QString::fromLatin1(responseText);
        Q_EMIT q->smtpError(Server::ClientError, QString::fromLatin1(responseText));
    }

    if (responseText[3] == ' ') {
        if (responseCode != expectedCode) {
            const QString lastError = QString::fromLatin1(responseText);
            qCWarning(SIMPLEMAIL_SERVER) << "Unexpected server response" << lastError << expectedCode;
            Q_EMIT q->smtpError(Server::ServerError, lastError);
            return false;
        }
        return true;
    }

    const QString lastError = QString::fromLatin1(responseText);
    qCWarning(SIMPLEMAIL_SERVER) << "Unexpected server response" << lastError << expectedCode;
    Q_EMIT q->smtpError(Server::ServerError, lastError);
    return false;
}

int ServerPrivate::parseCaps()
{
    Q_Q(Server);

    // Save the server's response
    const QByteArray responseText = socket->readLine().trimmed();
    qCDebug(SIMPLEMAIL_SERVER) << "Got response" << responseText;

    // Extract the respose code from the server's responce (first 3 digits)
    int responseCode = responseText.left(3).toInt();
    if (responseCode == 250) {
        caps.append(QString::fromLatin1(responseText));
        if (responseText[3] == ' ') {
            return 1;
        } else {
            return 0;
        }
    } else {
        const QString lastError = QString::fromLatin1(responseText);
        qCWarning(SIMPLEMAIL_SERVER) << "Unexpected server caps" << lastError;
        Q_EMIT q->smtpError(Server::ServerError, lastError);
        return -1;
    }
}

void ServerPrivate::commandReset()
{
    socket->write(QByteArrayLiteral("RSET\r\n"));
    awaitedCodes << 250;
}

void ServerPrivate::commandNoop()
{
    socket->write(QByteArrayLiteral("NOOP\r\n"));
    awaitedCodes << 250;
}

void ServerPrivate::commandQuit()
{
    socket->write(QByteArrayLiteral("QUIT\r\n"));
    awaitedCodes << 250;
}


#include "moc_server.cpp"
