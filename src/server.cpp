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

Q_LOGGING_CATEGORY(SIMPLEMAIL_SERVER, "simplemail.server", QtInfoMsg)

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
    return d->connectionType;
}

void Server::setConnectionType(Server::ConnectionType ct)
{
    Q_D(Server);
    delete d->socket;
    d->socket = nullptr;
    d->connectionType = ct;
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

    switch (d->connectionType) {
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

void Server::ignoreSslErrors()
{
    Q_D(Server);
    auto sslSock = qobject_cast<QSslSocket*>(d->socket);
    if (sslSock) {
        sslSock->ignoreSslErrors();
    }
}

void Server::ignoreSslErrors(const QList<QSslError> &errors)
{
    Q_D(Server);
    auto sslSock = qobject_cast<QSslSocket*>(d->socket);
    if (sslSock) {
        sslSock->ignoreSslErrors(errors);
    }
}

void ServerPrivate::createSocket()
{
    Q_Q(Server);

    if (socket) {
        return;
    }

    switch (connectionType) {
    case Server::TcpConnection:
        socket = new QTcpSocket(q);
        break;
    case Server::SslConnection:
    case Server::TlsConnection:
        socket = new QSslSocket(q);
        setPeerVerificationType(peerVerificationType);
        q->connect(static_cast<QSslSocket*>(socket), static_cast<void(QSslSocket::*)(const QList<QSslError> &)>(&QSslSocket::sslErrors),
                   q, &Server::sslErrors, Qt::DirectConnection);
    }
    q->connect(socket, &QTcpSocket::stateChanged, q, [=] (QAbstractSocket::SocketState sockState) {
        qCDebug(SIMPLEMAIL_SERVER) << "stateChanged" << sockState << socket->readAll();
        if (sockState == QAbstractSocket::ClosingState) {
            state = Closing;
        } else if (sockState == QAbstractSocket::UnconnectedState) {
            state = Disconnected;
            if (!queue.isEmpty()) {
                q->connectToServer();
            }
        }
    });

    q->connect(socket, &QTcpSocket::connected, q, [=] () {
        qCDebug(SIMPLEMAIL_SERVER) << "connected" << state << socket->readAll();
        state = WaitingForServiceReady220;
    });

    auto erroFn = [=] (QAbstractSocket::SocketError error) {
        qCDebug(SIMPLEMAIL_SERVER) << "SocketError" << error << socket->readAll();
        if (!queue.isEmpty()) {
            ServerReplyContainer &cont = queue[0];
            if (!cont.reply.isNull()) {
                ServerReply *reply = cont.reply;
                queue.removeFirst();
                reply->finish(true, -1, socket->errorString());
            } else {
                queue.removeFirst();
            }
        }
    };
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    q->connect(socket, &QTcpSocket::errorOccurred, q, erroFn);
#else
    q->connect(socket, static_cast<void(QTcpSocket::*)(QTcpSocket::SocketError)>(&QTcpSocket::error), q, erroFn);
#endif

    q->connect(socket, &QTcpSocket::readyRead, q, [=] {
        qCDebug(SIMPLEMAIL_SERVER) << "readyRead" << socket->bytesAvailable();
        switch (state) {
        case SendingMail:
            while (socket->canReadLine()) {
                if (!queue.isEmpty()) {
                    ServerReplyContainer &cont = queue[0];
                    if (cont.state == ServerReplyContainer::SendingCommands) {
                        while (!cont.awaitedCodes.isEmpty() && socket->canReadLine()) {
                            const int awaitedCode = cont.awaitedCodes.takeFirst();

                            QByteArray responseText;
                            const int code = parseResponseCode(&responseText);
                            if (code != awaitedCode) {
                                // Reset connection
                                if (!cont.reply.isNull()) {
                                    ServerReply *reply = cont.reply;
                                    queue.removeFirst();
                                    reply->finish(true, code, QString::fromLatin1(responseText));
                                } else {
                                    queue.removeFirst();
                                }
                                const QByteArray consume = socket->readAll();
                                qDebug() << "Mail error" << consume;
                                state = Ready;
                                commandReset();
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
                                qCCritical(SIMPLEMAIL_SERVER) << "Error writing mail";
                                if (!cont.reply.isNull()) {
                                    ServerReply *reply = cont.reply;
                                    queue.removeFirst();
                                    reply->finish(true, -1, q->tr("Error sending mail DATA"));
                                } else {
                                    queue.removeFirst();
                                }
                                socket->disconnectFromHost();
                                return;
                            }
                        }
                    } else if (cont.state == ServerReplyContainer::SendingData) {
                        QByteArray responseText;
                        const int code = parseResponseCode(&responseText);
                        if (!cont.reply.isNull()) {
                            ServerReply *reply = cont.reply;
                            queue.removeFirst();
                            reply->finish(code != 250, code, QString::fromLatin1(responseText));
                        } else {
                            queue.removeFirst();
                        }
                        qCDebug(SIMPLEMAIL_SERVER) << "MAIL FINISHED" << code << queue.size() << socket->canReadLine();

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
                    if (connectionType == Server::TlsConnection) {
                        auto sslSocket = qobject_cast<QSslSocket*>(socket);
                        if (sslSocket) {
                            if (!sslSocket->isEncrypted()) {
                                qCDebug(SIMPLEMAIL_SERVER) << "Sending STARTTLS";
                                socket->write(QByteArrayLiteral("STARTTLS\r\n"));
                                state = WaitingForServerStartTls_220;
                            } else {
                                login();
                            }
                        }
                    } else {
                        login();
                    }
                    break;
                } else if (ret == -1) {
                    break;
                }
            }
            break;
        case WaitingForServerStartTls_220:
            if (socket->canReadLine()) {
                if (parseResponseCode(220)) {
                    auto sslSock = qobject_cast<QSslSocket *>(socket);
                    if (sslSock) {
                        qCDebug(SIMPLEMAIL_SERVER) << "Starting client encryption";
                        sslSock->startClientEncryption();

                        // This will be queued and sent once the connection get's encrypted
                        socket->write("EHLO " + hostname.toLatin1() + "\r\n");
                        state = WaitingForServerCaps250;
                        caps.clear();
                    }
                }
            }
            break;
        case Noop_250:
        case Reset_250:
            if (parseResponseCode(250)) {
                qCDebug(SIMPLEMAIL_SERVER) << "Got NOOP/RSET OK";
                state = Ready;
                processNextMail();
            }
            break;
        case WaitingForAuthPlain235:
        case WaitingForAuthLogin235_step3:
        case WaitingForAuthCramMd5_235_step2:
            if (socket->canReadLine()) {
                if (parseResponseCode(235, Server::AuthenticationFailedError)) {
                    state = Ready;
                    processNextMail();
                }
            }
            break;
        case WaitingForAuthLogin334_step1:
            if (socket->canReadLine()) {
                if (parseResponseCode(334, Server::AuthenticationFailedError)) {
                    // Send the username in base64
                    qCDebug(SIMPLEMAIL_SERVER) << "Sending authentication user" << username;
                    socket->write(username.toUtf8().toBase64() + "\r\n");
                    state = WaitingForAuthLogin334_step2;
                }
            }
            break;
        case WaitingForAuthLogin334_step2:
            if (socket->canReadLine()) {
                if (parseResponseCode(334, Server::AuthenticationFailedError)) {
                    // Send the password in base64
                    qCDebug(SIMPLEMAIL_SERVER) << "Sending authentication password";
                    socket->write(password.toUtf8().toBase64() + "\r\n");
                    state = WaitingForAuthLogin235_step3;
                }
            }
            break;
        case WaitingForAuthCramMd5_334_step1:
            if (socket->canReadLine()) {
                QByteArray responseMessage;
                if (parseResponseCode(334, Server::AuthenticationFailedError, &responseMessage)) {
                    // Challenge
                    QByteArray ch = QByteArray::fromBase64(responseMessage);

                    // Compute the hash
                    QMessageAuthenticationCode code(QCryptographicHash::Md5);
                    code.setKey(password.toUtf8());
                    code.addData(ch);

                    QByteArray data(username.toUtf8() + " " + code.result().toHex());
                    socket->write(data.toBase64() + "\r\n");
                    state = WaitingForAuthCramMd5_235_step2;
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
            qCDebug(SIMPLEMAIL_SERVER) << "readyRead unknown state" << socket->readAll() << state;
        }
        qCDebug(SIMPLEMAIL_SERVER) << "readyRead" << socket->bytesAvailable();
    });
}

void ServerPrivate::setPeerVerificationType(const Server::PeerVerificationType &type)
{
    peerVerificationType = type;
    if (socket != Q_NULLPTR)
    {
        if (connectionType == Server::SslConnection || connectionType == Server::TlsConnection)
        {
            switch (type) {
                case Server::VerifyNone:
                    static_cast<QSslSocket*>(socket)->setPeerVerifyMode(QSslSocket::VerifyNone);
                    break;
//                case Server::VerifyPeer:
                default:
                    static_cast<QSslSocket*>(socket)->setPeerVerifyMode(QSslSocket::VerifyPeer);
                    break;
            }
        }
    }
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
    } else if (authMethod == Server::AuthLogin) {
        // Sending command: AUTH LOGIN
        qCDebug(SIMPLEMAIL_SERVER) << "Sending authentication login";
        socket->write(QByteArrayLiteral("AUTH LOGIN\r\n"));
        state = WaitingForAuthLogin334_step1;
    } else if (authMethod == Server::AuthCramMd5) {
        // NOTE Implementando - Ready
        qCDebug(SIMPLEMAIL_SERVER) << "Sending authentication CRAM-MD5";
        socket->write(QByteArrayLiteral("AUTH CRAM-MD5\r\n"));
        state = WaitingForAuthCramMd5_334_step1;
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
            // Send the MAIL command with the sender
            cont.commands << "MAIL FROM:<" + cont.msg.sender().address().toLatin1() + ">\r\n";
            cont.awaitedCodes << 250;

            // Send RCPT command for each recipient
            // To (primary recipients)
            const auto toRecipients = cont.msg.toRecipients();
            for (const EmailAddress &rcpt : toRecipients) {
                cont.commands << "RCPT TO:<" + rcpt.address().toLatin1() + ">\r\n";
                cont.awaitedCodes << 250;
            }

            // Cc (carbon copy)
            const auto ccRecipients = cont.msg.ccRecipients();
            for (const EmailAddress &rcpt : ccRecipients) {
                cont.commands << "RCPT TO:<" + rcpt.address().toLatin1() + ">\r\n";
                cont.awaitedCodes << 250;
            }

            // Bcc (blind carbon copy)
            const auto bccRecipients = cont.msg.bccRecipients();
            for (const EmailAddress &rcpt : bccRecipients) {
                cont.commands << "RCPT TO:<" + rcpt.address().toLatin1() + ">\r\n";
                cont.awaitedCodes << 250;
            }

            // DATA command
            cont.commands << QByteArrayLiteral("DATA\r\n");
            cont.awaitedCodes << 354;

            qCDebug(SIMPLEMAIL_SERVER) << "Sending MAIL command" << capPipelining << cont.commands.size() << cont.commands << cont.awaitedCodes;
            if (capPipelining) {
                for (const QByteArray &cmd : cont.commands) {
                    socket->write(cmd);
                }
            } else {
                socket->write(cont.commands.first());
            }

            state = SendingMail;
            cont.state = ServerReplyContainer::SendingCommands;
            return;
        } else {
            return;
        }
    }

     state = Ready;
}

bool ServerPrivate::parseResponseCode(int expectedCode, Server::SmtpError defaultError, QByteArray *responseMessage)
{
    Q_Q(Server);

    // Save the server's response
    const QByteArray responseText = socket->readLine().trimmed();
    qCDebug(SIMPLEMAIL_SERVER) << "Got response" << responseText << "expected" << expectedCode;

    // Extract the respose code from the server's responce (first 3 digits)
    const int responseCode = responseText.left(3).toInt();

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
            Q_EMIT q->smtpError(defaultError, lastError);
            return false;
        }
        if (responseMessage) {
            *responseMessage = responseText.mid(4);
        }
        return true;
    }

    const QString lastError = QString::fromLatin1(responseText);
    qCWarning(SIMPLEMAIL_SERVER) << "Unexpected server response" << lastError << expectedCode;
    Q_EMIT q->smtpError(defaultError, lastError);
    return false;
}

int ServerPrivate::parseResponseCode(QByteArray *responseMessage)
{
    Q_Q(Server);

    // Save the server's response
    const QByteArray responseText = socket->readLine().trimmed();
    qCDebug(SIMPLEMAIL_SERVER) << "Got response" << responseText;

    // Extract the respose code from the server's responce (first 3 digits)
    const int responseCode = responseText.left(3).toInt();

    if (responseCode / 100 == 4) {
        Q_EMIT q->smtpError(Server::ServerError, QString::fromLatin1(responseText));
    }

    if (responseCode / 100 == 5) {
        Q_EMIT q->smtpError(Server::ClientError, QString::fromLatin1(responseText));
    }

    if (responseMessage) {
        *responseMessage = responseText.mid(4);
    }

    return responseCode;
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
    if (state == Ready) {
        qCDebug(SIMPLEMAIL_SERVER) << "Sending RESET";
        socket->write(QByteArrayLiteral("RSET\r\n"));
        state = Reset_250;
    }
}

void ServerPrivate::commandNoop()
{
    if (state == Ready) {
        qCDebug(SIMPLEMAIL_SERVER) << "Sending NOOP";
        socket->write(QByteArrayLiteral("NOOP\r\n"));
        state = Noop_250;
    }
}

void ServerPrivate::commandQuit()
{
    socket->write(QByteArrayLiteral("QUIT\r\n"));
}

#include "moc_server.cpp"
