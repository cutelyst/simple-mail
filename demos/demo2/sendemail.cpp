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
#include "ui_sendemail.h"

#include <QFileDialog>
#include <QErrorMessage>
#include <QMessageBox>

#include "server.h"
#include "serverreply.h"

using namespace SimpleMail;

SendEmail::SendEmail(QWidget *parent) : QWidget(parent)
  , ui(new Ui::SendEmail)
{
    ui->setupUi(this);

    ui->host->setText(m_settings.value(QStringLiteral("host"), QStringLiteral("localhost")).toString());
    ui->port->setValue(m_settings.value(QStringLiteral("port"), 25).toInt());
    ui->username->setText(m_settings.value(QStringLiteral("username")).toString());
    ui->password->setText(m_settings.value(QStringLiteral("password")).toString());
    ui->ssl->setChecked(m_settings.value(QStringLiteral("ssl")).toBool());
    ui->sender->setText(m_settings.value(QStringLiteral("sender")).toString());
    ui->asyncCB->setChecked(m_settings.value(QStringLiteral("async")).toBool());
}

SendEmail::~SendEmail()
{
    delete ui;
}

EmailAddress SendEmail::stringToEmail(const QString &str)
{
    int p1 = str.indexOf(QLatin1String("<"));
    int p2 = str.indexOf(QLatin1String(">"));

    if (p1 == -1) {
        // no name, only email address
        return EmailAddress(str);
    } else {
        return EmailAddress(str.mid(p1 + 1, p2 - p1 - 1), str.left(p1));
    }
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
    EmailAddress sender = stringToEmail(ui->sender->text());

    QStringList rcptStringList = ui->recipients->text().split(QLatin1Char(';'));

    QString subject = ui->subject->text();
    QString html = ui->texteditor->toHtml();

    MimeMessage message;

    message.setSender(sender);
    message.setSubject(subject);

    for (int i = 0; i < rcptStringList.size(); ++i)
         message.addTo(stringToEmail(rcptStringList.at(i)));

    auto content = new MimeHtml;
    content->setHtml(html);

    message.addPart(content);

    for (int i = 0; i < ui->attachments->count(); ++i) {
        message.addPart(new MimeAttachment(new QFile(ui->attachments->item(i)->text())));
    }

    m_settings.setValue(QStringLiteral("host"), ui->host->text());
    m_settings.setValue(QStringLiteral("port"), ui->port->value());
    m_settings.setValue(QStringLiteral("username"), ui->username->text());
    m_settings.setValue(QStringLiteral("password"), ui->password->text());
    m_settings.setValue(QStringLiteral("ssl"), ui->ssl->isChecked());
    m_settings.setValue(QStringLiteral("sender"), ui->sender->text());
    m_settings.setValue(QStringLiteral("async"), ui->asyncCB->isChecked());

    if (ui->asyncCB->isChecked()) {
        sendMailAsync(message);
    } else {
        sendMailSync(message);
    }
}

void SendEmail::sendMailAsync(const MimeMessage &msg)
{
    auto server = new Server(this);
    server->setHost(ui->host->text());
    server->setPort(quint16(ui->port->value()));
    server->setConnectionType(ui->ssl->isChecked() ? Server::SslConnection : Server::TcpConnection);
    const QString user = ui->username->text();
    if (!user.isEmpty()) {
        server->setUsername(user);
        server->setPassword(ui->password->text());
    }

    ServerReply *reply = server->sendMail(msg);
    connect(reply, &ServerReply::finished, this, [=] {
        qDebug() << "ServerReply finished" << reply->error() << reply->errorText();
        reply->deleteLater();
        if (reply->error()) {
            errorMessage(QLatin1String("Mail sending failed:\n") + reply->errorText());
        } else {
            QMessageBox okMessage(this);
            okMessage.setText(QLatin1String("The email was succesfully sent."));
            okMessage.exec();
        }
    });
}

void SendEmail::sendMailSync(const MimeMessage &msg)
{
    QString host = ui->host->text();
    quint16 port = quint16(ui->port->value());
    bool ssl = ui->ssl->isChecked();
    QString user = ui->username->text();
    QString password = ui->password->text();

    Sender smtp(host, port, ssl ? Sender::SslConnection : Sender::TcpConnection);
    if (!user.isEmpty()) {
        smtp.setUser(user);
        smtp.setPassword(password);
    }

    if (!smtp.sendMail(msg)) {
        errorMessage(QLatin1String("Mail sending failed:\n") + smtp.lastError());
        return;
    } else {
        QMessageBox okMessage(this);
        okMessage.setText(QLatin1String("The email was succesfully sent."));
        okMessage.exec();
    }

    smtp.quit();
}

void SendEmail::errorMessage(const QString &message)
{
    QErrorMessage err (this);

    err.showMessage(message);

    err.exec();
}
