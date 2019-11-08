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

#ifndef SIMPLEMAIL_SENDER_H
#define SIMPLEMAIL_SENDER_H

#include <QObject>
#include <QtNetwork/QSslSocket>

#include "smtpexports.h"

namespace SimpleMail {

class MimeMessage;
class SenderPrivate;
class SMTP_EXPORT Sender : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Sender)
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
        TlsConnection,      // STARTTLS
    };
    Q_ENUM(ConnectionType)

    enum PeerVerificationType
    {
        VerifyNone,
        VerifyPeer
    };
    Q_ENUM(PeerVerificationType)

    explicit Sender(QObject *parent = nullptr);
    Sender(const QString &host, int port, ConnectionType ct, QObject *parent = nullptr);
    virtual ~Sender();

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
     * Defines the client's name. This name is sent by the EHLO command.
     * Defaults to the local host name
     */
    QString name() const;

    /**
     * Defines the client's name. This name is sent by the EHLO command.
     * Defaults to the local host name
     */
    void setName(const QString &name);

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
    QString user() const;

    /**
     * Defines the username that will authenticate on the SMTP server
     */
    void setUser(const QString &user);

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
     * Returns the response text the SMTP server last returned
     */
    QByteArray responseText() const;

    /**
     * Returns the response code the SMTP server last returned
     */
    int responseCode() const;

    /**
     * Returns the connection timeout when connecting to the SMTP server
     */
    int connectionTimeout() const;

    /**
     * Defines the connection timeout when connecting to the SMTP server
     */
    void setConnectionTimeout(int msec);

    /**
     * Returns the response timeout when waiting for a message to be processed by the SMTP server
     */
    int responseTimeout() const;

    /**
     * Defines the response timeout when waiting for a message to be processed by the SMTP server
     */
    void setResponseTimeout(int msec);
    
    /**
     * Returns the timeout when waiting for a message data to be sent to the SMTP server
     */
    int sendMessageTimeout() const;

    /**
     * Defines the timeout when waiting for a message data to be sent to the SMTP server
     */
    void setSendMessageTimeout(int msec);

    /**
     * @brief ignoreSslErrors tells the socket to ignore all pending ssl errors if SSL encryption is active.
     *      Must be called in a direct connected slot/functor
     */
    void ignoreSslErrors();

    /**
     * @brief ignoreSslErrors tells the socket to ignore the given ssl errors if SSL encryption is active.
     *      Must be called in a direct connected slot/functor
     * @param errors defines the errors to ignore
     */
    void ignoreSslErrors(const QList<QSslError> &errors);

    /**
     * @brief setPeerVerificationType Defines how the mail-server's SSL certificate should be examined
     * @param type  VerifyNone does not try to verify the identity, VerifyPeer does check on server identity
     */
    void setPeerVerificationType(PeerVerificationType type);

    /**
     * @brief peerVerificationType Returns the type of verification done for the mail-server's SSL certificate
     * @return PeerVerificationType as VerifyNone or VerifyPeer
     */
    PeerVerificationType peerVerificationType();

    bool sendMail(const MimeMessage &email);

    QString lastError() const;

    void quit();

protected Q_SLOTS:
    void socketStateChanged(QAbstractSocket::SocketState state);
    void socketError(QAbstractSocket::SocketError error);
    void socketReadyRead();

Q_SIGNALS:
    void smtpError(SmtpError e);
    void sslErrors(const QList<QSslError> &sslErrorList);

protected:
    SenderPrivate *d_ptr;
};

}

#endif // SIMPLEMAIL_SENDER_H
