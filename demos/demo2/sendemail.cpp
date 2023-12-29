/*
  Copyright (c) 2011 - Tőkés Attila

  This file is part of SmtpClient for Qt.

  SmtpClient for Qt is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  SmtpClient for Qt is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY.

  See the LICENSE file for more details.
*/

#include "sendemail.h"

#include "server.h"
#include "serverreply.h"
#include "ui_sendemail.h"

#include <QDebug>
#include <QErrorMessage>
#include <QFileDialog>
#include <QMessageBox>
#include <QSslError>

using namespace SimpleMail;

SendEmail::SendEmail(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SendEmail)
{
    ui->setupUi(this);

    ui->host->setText(
        m_settings.value(QStringLiteral("host"), QStringLiteral("localhost")).toString());
    ui->port->setValue(m_settings.value(QStringLiteral("port"), 25).toInt());
    ui->username->setText(m_settings.value(QStringLiteral("username")).toString());
    ui->password->setText(m_settings.value(QStringLiteral("password")).toString());
    ui->security->setCurrentIndex(m_settings.value(QStringLiteral("ssl")).toInt());
    ui->sender->setText(m_settings.value(QStringLiteral("sender")).toString());
}

SendEmail::~SendEmail()
{
    delete ui;
}

void SendEmail::on_addAttachment_clicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFiles);

    if (dialog.exec()) {
        ui->attachments->addItems(dialog.selectedFiles());
    }
}

void SendEmail::on_sendEmail_clicked()
{
    MimeMessage message;

    message.setSender(EmailAddress{ui->sender->text()});
    message.setSubject(ui->subject->text());

    const QStringList rcptStringList =
        ui->recipients->text().split(QLatin1Char(';'), Qt::SkipEmptyParts);
    for (const QString &to : rcptStringList) {
        message.addTo(EmailAddress{to});
    }

    message.addPart(std::make_shared<MimeHtml>(ui->texteditor->toHtml()));

    for (int i = 0; i < ui->attachments->count(); ++i) {
        message.addPart(std::make_shared<MimeAttachment>(
            std::make_shared<QFile>(ui->attachments->item(i)->text())));
    }

    m_settings.setValue(QStringLiteral("host"), ui->host->text());
    m_settings.setValue(QStringLiteral("port"), ui->port->value());
    m_settings.setValue(QStringLiteral("username"), ui->username->text());
    m_settings.setValue(QStringLiteral("password"), ui->password->text());
    m_settings.setValue(QStringLiteral("ssl"), ui->security->currentIndex());
    m_settings.setValue(QStringLiteral("sender"), ui->sender->text());

    sendMailAsync(message);
}

void SendEmail::sendMailAsync(const MimeMessage &msg)
{
    qDebug() << "sendMailAsync";

    const QString host = ui->host->text();
    const quint16 port(ui->port->value());
    const Server::ConnectionType ct = ui->security->currentIndex() == 0   ? Server::TcpConnection
                                      : ui->security->currentIndex() == 1 ? Server::SslConnection
                                                                          : Server::TlsConnection;

    Server *server = nullptr;
    for (auto srv : m_aServers) {
        if (srv->host() == host && srv->port() == port && srv->connectionType() == ct) {
            server = srv;
            break;
        }
    }

    if (!server) {
        server = new Server(this);
        connect(server, &Server::sslErrors, this, [](const QList<QSslError> &errors) {
            qDebug() << "Server SSL errors" << errors.size();
        });
        server->setHost(host);
        server->setPort(port);
        server->setConnectionType(ct);
        m_aServers.push_back(server);
    }

    const QString user = ui->username->text();
    if (!user.isEmpty()) {
        server->setAuthMethod(Server::AuthLogin);
        server->setUsername(user);
        server->setPassword(ui->password->text());
    }

    ServerReply *reply = server->sendMail(msg);
    connect(reply, &ServerReply::finished, this, [=] {
        qDebug() << "ServerReply finished" << reply->error() << reply->responseText();
        reply->deleteLater();
        if (reply->error()) {
            errorMessage(QLatin1String("Mail sending failed:\n") + reply->responseText());
        } else {
            QMessageBox okMessage(this);
            okMessage.setText(QLatin1String("The email was succesfully sent:\n") +
                              reply->responseText());
            okMessage.exec();
        }
    });
}

void SendEmail::errorMessage(const QString &message)
{
    QErrorMessage err(this);

    err.showMessage(message);

    err.exec();
}
