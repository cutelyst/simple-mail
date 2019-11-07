/*
 * Copyright (C) 2015 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef SENDER_P_H
#define SENDER_P_H

#include "sender.h"

#include <QtNetwork/QHostInfo>

namespace SimpleMail {

class SenderPrivate
{
    Q_DECLARE_PUBLIC(Sender)
public:
    enum State {
        Error,
        Disconnected,
        Connected,
        Ready
    };
    SenderPrivate(Sender *parent);

    bool sendMail(const MimeMessage &email);
    inline void sendMessage(const QByteArray &data);
    bool connectToHost();
    bool login();
    bool waitForResponse(int expectedCode);
    bool processState();
    void setPeerVerificationType(const Sender::PeerVerificationType &type);

    State state = State::Disconnected;
    Sender *q_ptr;
    QTcpSocket *socket = nullptr;
    QString lastError;

    QString host = QLatin1String("localhost");
    quint16 port = 25;
    Sender::ConnectionType connectionType;
    QString name = QHostInfo::localHostName();
    Sender::PeerVerificationType peerVerificationType = Sender::VerifyPeer;

    QString user;
    QString password;
    Sender::AuthMethod authMethod = Sender::AuthNone;

    int connectionTimeout = 5000;
    int responseTimeout = 5000;
    int sendMessageTimeout = 60000;

    QByteArray responseText;
    int responseCode;
};

}

#endif // SENDER_P_H
