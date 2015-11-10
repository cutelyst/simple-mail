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

#include "sender_p.h"

using namespace SimpleMail;

Sender::Sender(QObject *parent) : QObject(parent)
  , d_ptr(new SenderPrivate)
{
    setConnectionType(TcpConnection);
}

Sender::Sender(const QString &host, int port, ConnectionType connectionType, QObject *parent) : QObject(parent)
  , d_ptr(new SenderPrivate)
{
    Q_D(Sender);

    setConnectionType(connectionType);

    d->host = host;
    d->port = port;
}

Sender::~Sender()
{
    delete d_ptr;
}

QString Sender::host() const
{
    Q_D(const Sender);
    return d->host;
}

void Sender::setUser(const QString &user)
{
    Q_D(Sender);
    d->user = user;
}

QString Sender::password() const
{
    Q_D(const Sender);
    return d->password;
}

void Sender::setPassword(const QString &password)
{
    Q_D(Sender);
    d->password = password;
}

Sender::AuthMethod Sender::authMethod() const
{
    Q_D(const Sender);
    return d->authMethod;
}

void Sender::setAuthMethod(AuthMethod method)
{
    Q_D(Sender);
    d->authMethod = method;
}

void Sender::setHost(const QString &host)
{
    Q_D(Sender);
    d->host = host;
}

int Sender::port() const
{
    Q_D(const Sender);
    return d->port;
}

void Sender::setPort(int port)
{
    Q_D(Sender);
    d->port = port;
}

QString Sender::name() const
{
    Q_D(const Sender);
    return d->name;
}

void Sender::setName(const QString &name)
{
    Q_D(Sender);
    d->name = name;
}

Sender::ConnectionType Sender::connectionType() const
{
    Q_D(const Sender);
    return d->connectionType;
}

void Sender::setConnectionType(ConnectionType connectionType)
{
    Q_D(Sender);

    d->connectionType = connectionType;

    delete d->socket;

    switch (connectionType) {
    case TcpConnection:
        d->socket = new QTcpSocket(this);
        break;
    case SslConnection:
    case TlsConnection:
        d->socket = new QSslSocket(this);
    }
    connect(d->socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            this, SLOT(socketStateChanged(QAbstractSocket::SocketState)));
    connect(d->socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(socketError(QAbstractSocket::SocketError)));
    connect(d->socket, SIGNAL(readyRead()),
            this, SLOT(socketReadyRead()));
}

QString Sender::user() const
{
    Q_D(const Sender);
    return d->user;
}

QByteArray Sender::responseText() const
{
    Q_D(const Sender);
    return d->responseText;
}

int Sender::responseCode() const
{
    Q_D(const Sender);
    return d->responseCode;
}

int Sender::connectionTimeout() const
{
    Q_D(const Sender);
    return d->connectionTimeout;
}

void Sender::setConnectionTimeout(int msec)
{
    Q_D(Sender);
    d->connectionTimeout = msec;
}

int Sender::responseTimeout() const
{
    Q_D(const Sender);
    return d->responseTimeout;
}

void Sender::setResponseTimeout(int msec)
{
    Q_D(Sender);
    d->responseTimeout = msec;
}
int Sender::sendMessageTimeout() const
{
    Q_D(const Sender);
    return d->sendMessageTimeout;
}
void Sender::setSendMessageTimeout(int msec)
{
    Q_D(Sender);
    d->sendMessageTimeout = msec;
}

bool Sender::connectToHost()
{
    Q_D(Sender);
    switch (d->connectionType) {
    case TlsConnection:
    case TcpConnection:
        d->socket->connectToHost(d->host, d->port);
        break;
    case SslConnection:
    {
        QSslSocket *sslSock = qobject_cast<QSslSocket*>(d->socket);
        if (sslSock) {
            sslSock->connectToHostEncrypted(d->host, d->port);
        } else {
            return false;
        }
    }
        break;
    }

    // Tries to connect to server
    if (!d->socket->waitForConnected(d->connectionTimeout)) {
        Q_EMIT smtpError(ConnectionTimeoutError);
        return false;
    }

    try
    {
        // Wait for the server's response
        waitForResponse();

        // If the response code is not 220 (Service ready)
        // means that is something wrong with the server
        if (d->responseCode != 220)
        {
            Q_EMIT smtpError(ServerError);
            return false;
        }

        // Send a EHLO/HELO message to the server
        // The client's first command must be EHLO/HELO
        sendMessage("EHLO " + d->name.toLatin1());

        // Wait for the server's response
        waitForResponse();

        // The response code needs to be 250.
        if (d->responseCode != 250) {
            Q_EMIT smtpError(ServerError);
            return false;
        }

        if (d->connectionType == TlsConnection) {
            // send a request to start TLS handshake
            sendMessage(QByteArrayLiteral("STARTTLS"));

            // Wait for the server's response
            waitForResponse();

            // The response code needs to be 220.
            if (d->responseCode != 220) {
                Q_EMIT smtpError(ServerError);
                return false;
            };

            // TODO check if Tls creates a QSslSocket instance
            QSslSocket *sslSock = qobject_cast<QSslSocket*>(d->socket);
            if (sslSock) {
                sslSock->startClientEncryption();
            }

            if (sslSock && !sslSock->waitForEncrypted(d->connectionTimeout)) {
                qDebug() << sslSock->errorString();
                Q_EMIT smtpError(ConnectionTimeoutError);
                return false;
            }

            // Send ELHO one more time
            sendMessage(QByteArrayLiteral("EHLO ") + d->name.toLatin1());

            // Wait for the server's response
            waitForResponse();

            // The response code needs to be 250.
            if (d->responseCode != 250) {
                Q_EMIT smtpError(ServerError);
                return false;
            }
        }
    }
    catch (ResponseTimeoutException)
    {
        return false;
    }
    catch (SendMessageTimeoutException)
    {
        return false;
    }

    // If no errors occured the function returns true.
    return true;
}

bool Sender::login()
{
    Q_D(const Sender);
    return login(d->user, d->password, d->authMethod);
}

bool Sender::login(const QString &user, const QString &password, AuthMethod method)
{
    Q_D(Sender);
    try {
        if (method == AuthPlain)
        {
            // Sending command: AUTH PLAIN base64('\0' + username + '\0' + password)
            QString userpass = QLatin1Char('\0') % user % QLatin1Char('\0') % password;
            sendMessage(QByteArrayLiteral("AUTH PLAIN ") + userpass.toLatin1().toBase64());

            // Wait for the server's response
            waitForResponse();

            // If the response is not 235 then the authentication was faild
            if (d->responseCode != 235) {
                Q_EMIT smtpError(AuthenticationFailedError);
                return false;
            }
        }
        else if (method == AuthLogin)
        {
            // Sending command: AUTH LOGIN
            sendMessage(QByteArrayLiteral("AUTH LOGIN"));

            // Wait for 334 response code
            waitForResponse();
            if (d->responseCode != 334) {
                Q_EMIT smtpError(AuthenticationFailedError);
                return false;
            }

            // Send the username in base64
            sendMessage(user.toLatin1().toBase64());

            // Wait for 334
            waitForResponse();
            if (d->responseCode != 334) {
                Q_EMIT smtpError(AuthenticationFailedError);
                return false;
            }

            // Send the password in base64
            sendMessage(password.toUtf8().toBase64());

            // Wait for the server's responce
            waitForResponse();

            // If the response is not 235 then the authentication was faild
            if (d->responseCode != 235) {
                Q_EMIT smtpError(AuthenticationFailedError);
                return false;
            }
        }
    }
    catch (ResponseTimeoutException)
    {
        // Responce Timeout exceeded
        Q_EMIT smtpError(AuthenticationFailedError);
        return false;
    }
    catch (SendMessageTimeoutException)
    {
        // Send Timeout exceeded
        Q_EMIT smtpError(AuthenticationFailedError);
        return false;
    }

    return true;
}

bool Sender::sendMail(MimeMessage &email)
{
    Q_D(Sender);
    try
    {
        // Send the MAIL command with the sender
        sendMessage("MAIL FROM: <" + email.sender().address().toLatin1() + '>');

        waitForResponse();

        if (d->responseCode != 250) {
            return false;
        }

        // Send RCPT command for each recipient
        // To (primary recipients)
        Q_FOREACH (const EmailAddress &rcpt, email.toRecipients()) {
            sendMessage("RCPT TO: <" + rcpt.address().toLatin1() + '>');
            waitForResponse();

            if (d->responseCode != 250) {
                return false;
            }
        }

        // Cc (carbon copy)
        Q_FOREACH (const EmailAddress &rcpt, email.ccRecipients()) {
            sendMessage("RCPT TO: <" + rcpt.address().toLatin1() + '>');
            waitForResponse();

            if (d->responseCode != 250) {
                return false;
            }
        }

        // Bcc (blind carbon copy)
        Q_FOREACH (const EmailAddress &rcpt, email.bccRecipients()) {
            sendMessage("RCPT TO: <" + rcpt.address().toLatin1() + '>');
            waitForResponse();

            if (d->responseCode != 250) {
                return false;
            }
        }

        // Send DATA command
        sendMessage(QByteArrayLiteral("DATA"));
        waitForResponse();

        if (d->responseCode != 354) {
            return false;
        }

        if (!email.write(d->socket)) {
            return false;
        }

        // Send \r\n.\r\n to end the mail data
        sendMessage(QByteArrayLiteral("\r\n."));

        waitForResponse();

        if (d->responseCode != 250) {
            return false;
        }
    }
    catch (ResponseTimeoutException)
    {
        return false;
    }
    catch (SendMessageTimeoutException)
    {
        return false;
    }

    return true;
}

void Sender::quit()
{
    sendMessage(QByteArrayLiteral("QUIT"));
}

void Sender::waitForResponse()
{
    Q_D(Sender);

    do {
        if (!d->socket->waitForReadyRead(d->responseTimeout)) {
            Q_EMIT smtpError(ResponseTimeoutError);
            throw ResponseTimeoutException();
        }

        while (d->socket->canReadLine()) {
            // Save the server's response
            d->responseText = d->socket->readLine();

            // Extract the respose code from the server's responce (first 3 digits)
            d->responseCode = d->responseText.left(3).toInt();

            if (d->responseCode / 100 == 4) {
                Q_EMIT smtpError(ServerError);
            }

            if (d->responseCode / 100 == 5) {
                Q_EMIT smtpError(ClientError);
            }

            if (d->responseText[3] == ' ') {
                return;
            }
        }
    } while (true);
}

void Sender::sendMessage(const QByteArray &data)
{
    Q_D(Sender);
    d->socket->write(data);
    d->socket->write("\r\n", 2);
    if (!d->socket->waitForBytesWritten(d->sendMessageTimeout)) {
        Q_EMIT smtpError(SendDataTimeoutError);
        throw SendMessageTimeoutException();
    }
}

void Sender::socketStateChanged(QAbstractSocket::SocketState /*state*/)
{
}

void Sender::socketError(QAbstractSocket::SocketError /*socketError*/)
{
}

void Sender::socketReadyRead()
{
}
