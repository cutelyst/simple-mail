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
#ifndef SERVER_H
#define SERVER_H

#include <QObject>

#include "smtpexports.h"

class QSslError;
namespace SimpleMail {

class MimeMessage;
class ServerReply;
class ServerPrivate;
class SMTP_EXPORT Server : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Server)
public:
    enum AuthMethod
    {
        AuthNone,
        AuthPlain,
        AuthLogin,
        AuthCramMd5,
    };
    Q_ENUM(AuthMethod)

    enum SmtpError
    {
        ConnectionTimeoutError,
        ResponseTimeoutError,
        SendDataTimeoutError,
        AuthenticationFailedError,
        ServerError,    // 4xx smtp error
        ClientError,    // 5xx smtp error
    };
    Q_ENUM(SmtpError)

    enum ConnectionType
    {
        TcpConnection,
        SslConnection,
        TlsConnection,     // STARTTLS
    };
    Q_ENUM(ConnectionType)

    enum PeerVerificationType
    {
        VerifyNone,
        VerifyPeer,
    };
    Q_ENUM(PeerVerificationType)

    explicit Server(QObject *parent = nullptr);
    virtual ~Server();

    /**
     * Returns the hostname of the SMTP server
     */
    QString host() const;

    /**
     * Defines the hostname of the SMTP server
     */
    void setHost(const QString &host);

    /**
     * Returns the port of the SMTP server
     */
    quint16 port() const;

    /**
     * Defines the port of the SMTP server
     */
    void setPort(quint16 port);

    /**
     * The hostname is sent by the EHLO command.
     */
    QString hostname() const;

    /**
     * Defines the client's hostname. This is the hostname is sent by the EHLO command.
     * Defaults to the local hostname
     */
    void setHostname(const QString &hostname);

    /**
     * Returns the connection type of the SMTP server
     */
    ConnectionType connectionType() const;

    /**
     * Defines the connection type of the SMTP server
     */
    void setConnectionType(ConnectionType ct);

    /**
     * Returns the username that will authenticate on the SMTP server
     */
    QString username() const;

    /**
     * Defines the username that will authenticate on the SMTP server
     */
    void setUsername(const QString &username);

    /**
     * Returns the password that will authenticate on the SMTP server
     */
    QString password() const;

    /**
     * Defines the password that will authenticate on the SMTP server
     */
    void setPassword(const QString &password);

    /**
     * Returns the authenticaion method of the SMTP server
     */
    AuthMethod authMethod() const;

    /**
     * Defines the authenticaion method of the SMTP server
     */
    void setAuthMethod(AuthMethod method);

    /**
     * Sends the email async.
     * The email is added to a queue and is processed once
     * the connection and protocol commands are exchanged,
     * if the server reports PIPELINING the recipients commands
     * are sent in one go automatically.
     *
     * You must delete the returned object, if you do so before
     * it's finished() signal is emited this class won't send
     * the email.
     */
    ServerReply *sendMail(const MimeMessage &msg);

    /**
     * Returns the number of emails in queue
     * Can be useful if you create multiple Server instances and
     * want to load balance your emails.
     */
    int queueSize() const;

    /**
     * Connects to the SMTP server.
     * This is called automatically when an email is sent, and usually SMTP servers
     * timeout the connection after a while.
     */
    void connectToServer();

    /**
     * @brief ignoreSslErrors tells the socket to ignore all pending ssl errors if SSL encryption is active.
     *      Must be called in a direct connected slot/functor
     */
    void ignoreSslErrors();

    /**
     * @brief ignoreSslErrors tells the socket to ignore the given ssl errors if SSL encryption is active.
     * @param errors defines the errors to ignore
     */
    void ignoreSslErrors(const QList<QSslError> &errors);

Q_SIGNALS:
    void smtpError(SmtpError e, const QString &description);
    void sslErrors(const QList<QSslError> &sslErrorList);

private:
    ServerPrivate *d_ptr;
};

}

#endif // SERVER_H
