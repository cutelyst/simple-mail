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

#include "mimemessage.h"

#include <QLoggingCategory>
#include <QMessageAuthenticationCode>

Q_LOGGING_CATEGORY(SIMPLEMAIL_SENDER, "simplemail.sender")

using namespace SimpleMail;

Sender::Sender(QObject *parent) : QObject(parent)
  , d_ptr(new SenderPrivate(this))
{
    setConnectionType(TcpConnection);
}

Sender::Sender(const QString &host, int port, ConnectionType connectionType, QObject *parent) : QObject(parent)
  , d_ptr(new SenderPrivate(this))
{
    Q_D(Sender);

    setConnectionType(connectionType);

    d->host = host;
    d->port = quint16(port);
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
    if (d->authMethod == Sender::AuthNone) {
        d->authMethod = Sender::AuthPlain;
    }
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
    if (d->authMethod == Sender::AuthNone) {
        d->authMethod = Sender::AuthPlain;
    }
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

quint16 Sender::port() const
{
    Q_D(const Sender);
    return d->port;
}

void Sender::setPort(quint16 port)
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
        d->setPeerVerificationType(d->peerVerificationType);
        connect(static_cast<QSslSocket*>(d->socket), static_cast<void(QSslSocket::*)(const QList<QSslError> &)>(&QSslSocket::sslErrors),this, &Sender::sslErrors, Qt::DirectConnection);
    }
    connect(d->socket, &QTcpSocket::stateChanged, this, &Sender::socketStateChanged);
    connect(d->socket, static_cast<void(QTcpSocket::*)(QTcpSocket::SocketError)>(&QTcpSocket::error),
            this, &Sender::socketError);
    connect(d->socket, &QTcpSocket::readyRead, this, &Sender::socketReadyRead);
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

void Sender::ignoreSslErrors()
{
    Q_D(Sender);
    if (connectionType() == SslConnection || connectionType() == TlsConnection)
    {
        static_cast<QSslSocket*>(d->socket)->ignoreSslErrors();
    }
}

void Sender::ignoreSslErrors(const QList<QSslError> &errors)
{
    Q_D(Sender);
    if (connectionType() == SslConnection || connectionType() == TlsConnection)
    {
        static_cast<QSslSocket*>(d->socket)->ignoreSslErrors(errors);
    }
}

void Sender::setPeerVerificationType(Sender::PeerVerificationType type)
{
    Q_D(Sender);
    d->setPeerVerificationType(type);
}

Sender::PeerVerificationType Sender::peerVerificationType()
{
    Q_D(Sender);
    return d->peerVerificationType;
}

bool Sender::sendMail(const MimeMessage &email)
{
    Q_D(Sender);
    return d->sendMail(email);
}

QString Sender::lastError() const
{
    Q_D(const Sender);
    return d->lastError;
}

void Sender::quit()
{
    Q_D(Sender);
    d->sendMessage(QByteArrayLiteral("QUIT"));
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

SenderPrivate::SenderPrivate(Sender *parent) :
    q_ptr(parent)
{

}

bool SenderPrivate::sendMail(const MimeMessage &email)
{
    qCDebug(SIMPLEMAIL_SENDER) << "Sending MAIL" << this;

    if (!processState()) {
        return false;
    }

    qCDebug(SIMPLEMAIL_SENDER) << "Sending MAIL command";
    // Send the MAIL command with the sender
    sendMessage("MAIL FROM:<" + email.sender().address().toLatin1() + '>');
    if (!waitForResponse(250)) {
        return false;
    }

    qCDebug(SIMPLEMAIL_SENDER) << "Sending RCPT TO command";
    // Send RCPT command for each recipient
    // To (primary recipients)
    const auto toRecipients = email.toRecipients();
    for (const EmailAddress &rcpt : toRecipients) {
        sendMessage("RCPT TO:<" + rcpt.address().toLatin1() + '>');

        if (!waitForResponse(250)) {
            return false;
        }
    }

    // Cc (carbon copy)
    const auto ccRecipients = email.ccRecipients();
    for (const EmailAddress &rcpt : ccRecipients) {
        sendMessage("RCPT TO:<" + rcpt.address().toLatin1() + '>');

        if (!waitForResponse(250)) {
            return false;
        }
    }

    // Bcc (blind carbon copy)
    const auto bccRecipients = email.bccRecipients();
    for (const EmailAddress &rcpt : bccRecipients) {
        sendMessage("RCPT TO:<" + rcpt.address().toLatin1() + '>');

        if (!waitForResponse(250)) {
            return false;
        }
    }

    qCDebug(SIMPLEMAIL_SENDER) << "Sending DATA command";
    // Send DATA command
    sendMessage(QByteArrayLiteral("DATA"));

    if (!waitForResponse(354)) {
        return false;
    }

    qCDebug(SIMPLEMAIL_SENDER) << "Sending email";
    if (!email.write(socket)) {
        return false;
    }

    // Send \r\n.\r\n to end the mail data
    sendMessage(QByteArrayLiteral("\r\n."));
    if (!waitForResponse(250)) {
        return false;
    }
    qCDebug(SIMPLEMAIL_SENDER) << "Mail sent";

    return true;
}

void SenderPrivate::sendMessage(const QByteArray &data)
{
    Q_Q(Sender);
    socket->write(data);
    socket->write("\r\n", 2);
    if (!socket->waitForBytesWritten(sendMessageTimeout)) {
        Q_EMIT q->smtpError(Sender::SendDataTimeoutError);
    }
}

bool SenderPrivate::connectToHost()
{
    Q_Q(Sender);

    QSslSocket *sslSock = nullptr;
    switch (connectionType) {
    case Sender::TlsConnection:
    case Sender::TcpConnection:
        qCDebug(SIMPLEMAIL_SENDER) << "Connecting to host" << host << port;
        socket->connectToHost(host, port);
        break;
    case Sender::SslConnection:
    {
        sslSock = qobject_cast<QSslSocket*>(socket);
        if (sslSock) {
            qCDebug(SIMPLEMAIL_SENDER) << "Connecting to host encrypted" << host << port;
            sslSock->connectToHostEncrypted(host, port);
        } else {
            return false;
        }
    }
        break;
    }

    // Tries to connect to server
    if (!socket->waitForConnected(connectionTimeout)) {
        lastError = socket->errorString();
        qCDebug(SIMPLEMAIL_SENDER) << "Connection failed" << socket->errorString();
        Q_EMIT q->smtpError(Sender::ConnectionTimeoutError);
        return false;
    }

    // If the response code is not 220 (Service ready)
    // means that is something wrong with the server
    if (!waitForResponse(220)) {
        Q_EMIT q->smtpError(Sender::ServerError);
        return false;
    }

    qCDebug(SIMPLEMAIL_SENDER) << "Sending EHLO" << name;
    // Send a EHLO/HELO message to the server
    // The client's first command must be EHLO/HELO
    sendMessage("EHLO " + name.toLatin1());

    // The response code needs to be 250.
    if (!waitForResponse(250)) {
        Q_EMIT q->smtpError(Sender::ServerError);
        return false;
    }
    qCDebug(SIMPLEMAIL_SENDER) << "Sent hello";

    if (connectionType == Sender::TlsConnection) {
        qCDebug(SIMPLEMAIL_SENDER) << "Sending STARTTLS";

        // send a request to start TLS handshake
        sendMessage(QByteArrayLiteral("STARTTLS"));

        // The response code needs to be 220.
        if (!waitForResponse(220)) {
            Q_EMIT q->smtpError(Sender::ServerError);
            return false;
        }

        sslSock = qobject_cast<QSslSocket *>(socket);
        if (sslSock) {
            qCDebug(SIMPLEMAIL_SENDER) << "Starting client encryption";
            sslSock->startClientEncryption();

            if (!sslSock->waitForEncrypted(connectionTimeout)) {
                qCDebug(SIMPLEMAIL_SENDER) << "Failed to encrypt connection" << sslSock->errorString();
                Q_EMIT q->smtpError(Sender::ConnectionTimeoutError);
                return false;
            }
        } else {
            qCDebug(SIMPLEMAIL_SENDER) << "Failed to start TLS negotiation";
            return false;
        }

        qCDebug(SIMPLEMAIL_SENDER) << "Sending second EHLO" << name;
        // Send ELHO one more time
        sendMessage(QByteArrayLiteral("EHLO ") + name.toLatin1());

        // The response code needs to be 250.
        if (!waitForResponse(250)) {
            Q_EMIT q->smtpError(Sender::ServerError);
            return false;
        }
    }

    state = SenderPrivate::Connected;

    // If no errors occured the function returns true.
    return true;

}

bool SenderPrivate::login()
{
    Q_Q(Sender);

    if (authMethod == Sender::AuthPlain) {
        qCDebug(SIMPLEMAIL_SENDER) << "Sending authentication plain";

        // Sending command: AUTH PLAIN base64('\0' + username + '\0' + password)
        QString userpass = QLatin1Char('\0') + user + QLatin1Char('\0') + password;
        sendMessage(QByteArrayLiteral("AUTH PLAIN ") + userpass.toLatin1().toBase64());

        // If the response is not 235 then the authentication was faild
        if (!waitForResponse(235)) {
            Q_EMIT q->smtpError(Sender::AuthenticationFailedError);
            return false;
        }
    } else if (authMethod == Sender::AuthLogin) {
        // Sending command: AUTH LOGIN
        qCDebug(SIMPLEMAIL_SENDER) << "Sending authentication login";
        sendMessage(QByteArrayLiteral("AUTH LOGIN"));

        // Wait for 334 response code
        if (!waitForResponse(334)) {
            Q_EMIT q->smtpError(Sender::AuthenticationFailedError);
            return false;
        }

        // Send the username in base64
        qCDebug(SIMPLEMAIL_SENDER) << "Sending authentication user" << user;
        sendMessage(user.toLatin1().toBase64());

        // Wait for 334
        if (!waitForResponse(334)) {
            Q_EMIT q->smtpError(Sender::AuthenticationFailedError);
            return false;
        }

        // Send the password in base64
        qCDebug(SIMPLEMAIL_SENDER) << "Sending authentication password";
        sendMessage(password.toUtf8().toBase64());

        // If the response is not 235 then the authentication was faild
        if (!waitForResponse(235)) {
            Q_EMIT q->smtpError(Sender::AuthenticationFailedError);
            return false;
        }
    } else if (authMethod == Sender::AuthCramMd5) {
        // NOTE Implementando - Ready
        qCDebug(SIMPLEMAIL_SENDER) << "Sending authentication CRAM-MD5";
        sendMessage(QByteArrayLiteral("AUTH CRAM-MD5"));

        // Wait for 334
        if (!waitForResponse(334)) {
            Q_EMIT q->smtpError(Sender::AuthenticationFailedError);
            return false;
        }

        // Challenge
        QByteArray ch = QByteArray::fromBase64(responseText.mid((4)));

        // Compute the hash
        QMessageAuthenticationCode code(QCryptographicHash::Md5);
        code.setKey(password.toLatin1());
        code.addData(ch);

        QByteArray data(user.toLatin1() + " " + code.result().toHex());
        sendMessage(data.toBase64());

        // Wait for 235
        if (!waitForResponse(235)) {
            Q_EMIT q->smtpError(Sender::AuthenticationFailedError);
            return false;
        }
    }

    state = SenderPrivate::Ready;

    return true;
}

bool SenderPrivate::waitForResponse(int expectedCode)
{
    Q_Q(Sender);

    do {
        if (!socket->waitForReadyRead(responseTimeout)) {
            lastError = QObject::tr("Response timed out");
            qCDebug(SIMPLEMAIL_SENDER) << "Response timed out";
            Q_EMIT q->smtpError(Sender::ResponseTimeoutError);
            return false;
        }

        while (socket->canReadLine()) {
            // Save the server's response
            responseText = socket->readLine().trimmed();
            qCDebug(SIMPLEMAIL_SENDER) << "Got response" << responseText;

            // Extract the respose code from the server's responce (first 3 digits)
            responseCode = responseText.left(3).toInt();

            if (responseCode / 100 == 4) {
                lastError = QString::fromLatin1(responseText);
                Q_EMIT q->smtpError(Sender::ServerError);
            }

            if (responseCode / 100 == 5) {
                lastError = QString::fromLatin1(responseText);
                Q_EMIT q->smtpError(Sender::ClientError);
            }

            if (responseText[3] == ' ') {
                if (responseCode != expectedCode) {
                    lastError = QString::fromLatin1(responseText);
                    qCDebug(SIMPLEMAIL_SENDER) << "Unexpected server response" << lastError << expectedCode;
                    return false;
                }
                return true;
            }
        }
    } while (true);
}

bool SenderPrivate::processState()
{
    switch (state) {
    case SenderPrivate::Ready:
    case SenderPrivate::Error:
        if (socket->state() != QAbstractSocket::ConnectedState) {
            state = SenderPrivate::Disconnected;
            if (socket->state() != QAbstractSocket::UnconnectedState) {
                socket->disconnectFromHost();
                socket->waitForDisconnected();
            }
        }
        break;
    default:
        break;
    }

    while (state != SenderPrivate::Ready) {
        qCDebug(SIMPLEMAIL_SENDER) << "Processing state" << state;
        switch (state) {
        case SenderPrivate::Disconnected:
            if (!connectToHost()) {
                state = SenderPrivate::Error;
                return false;
            }
            break;
        case SenderPrivate::Connected:
            if (!login()) {
                state = SenderPrivate::Error;
                return false;
            }
            break;
        case SenderPrivate::Error:
            // try again
            socket->disconnectFromHost();
            socket->waitForDisconnected();
            break;
        case SenderPrivate::Ready:
            break;
        }
    }
    return true;
}

void SenderPrivate::setPeerVerificationType(const Sender::PeerVerificationType &type)
{
    peerVerificationType = type;
    if (socket != Q_NULLPTR)
    {
        if (connectionType == Sender::SslConnection || connectionType == Sender::TlsConnection)
        {
            switch (type) {
                case Sender::VerifyNone:
                    static_cast<QSslSocket*>(socket)->setPeerVerifyMode(QSslSocket::VerifyNone);
                    break;
//                case Sender::VerifyPeer:
                default:
                    static_cast<QSslSocket*>(socket)->setPeerVerifyMode(QSslSocket::VerifyPeer);
                    break;
            }
        }
    }
}

#include "moc_sender.cpp"
