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

#ifndef SMTPCLIENT_H
#define SMTPCLIENT_H

#include <QObject>
#include <QtNetwork/QSslSocket>

#include "mimemessage.h"
#include "smtpexports.h"

class SMTP_EXPORT SmtpClient : public QObject
{
    Q_OBJECT
public:
    enum AuthMethod
    {
        AuthPlain,
        AuthLogin
    };

    enum SmtpError
    {
        ConnectionTimeoutError,
        ResponseTimeoutError,
        SendDataTimeoutError,
        AuthenticationFailedError,
        ServerError,    // 4xx smtp error
        ClientError     // 5xx smtp error
    };

    enum ConnectionType
    {
        TcpConnection,
        SslConnection,
        TlsConnection       // STARTTLS
    };

    SmtpClient(const QString &host = QLatin1String("localhost"), int port = 25, ConnectionType ct = TcpConnection);

    ~SmtpClient();

    const QString& getHost() const;
    void setHost(const QString &host);

    int getPort() const;
    void setPort(int port);

    const QString& getName() const;
    void setName(const QString &name);

    ConnectionType getConnectionType() const;
    void setConnectionType(ConnectionType ct);

    const QString & getUser() const;
    void setUser(const QString &user);

    const QString & getPassword() const;
    void setPassword(const QString &password);

    SmtpClient::AuthMethod getAuthMethod() const;
    void setAuthMethod(AuthMethod method);

    const QString & getResponseText() const;
    int getResponseCode() const;

    int getConnectionTimeout() const;
    void setConnectionTimeout(int msec);

    int getResponseTimeout() const;
    void setResponseTimeout(int msec);
    
    int getSendMessageTimeout() const;
    void setSendMessageTimeout(int msec);

    QTcpSocket* getSocket();

    bool connectToHost();

    bool login();
    bool login(const QString &user, const QString &password, AuthMethod method = AuthLogin);

    bool sendMail(MimeMessage& email);

    void quit();

protected:
    QTcpSocket *socket = nullptr;

    QString host;
    int port;
    ConnectionType connectionType;
    QString name = QLatin1String("localhost");

    QString user;
    QString password;
    AuthMethod authMethod = AuthPlain;

    int connectionTimeout = 5000;
    int responseTimeout = 5000;
    int sendMessageTimeout = 60000;
    
    
    QString responseText;
    int responseCode;


    class ResponseTimeoutException {};
    class SendMessageTimeoutException {};

    void waitForResponse();

    void sendMessage(const QString &text);

protected Q_SLOTS:
    void socketStateChanged(QAbstractSocket::SocketState state);
    void socketError(QAbstractSocket::SocketError error);
    void socketReadyRead();

Q_SIGNALS:
    void smtpError(SmtpClient::SmtpError e);
};

#endif // SMTPCLIENT_H
