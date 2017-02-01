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

#include <iostream>

using namespace std;
using namespace SimpleMail;

SendEmail::SendEmail(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SendEmail)
{
    ui->setupUi(this);
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


    if (dialog.exec())
        ui->attachments->addItems(dialog.selectedFiles());


}

void SendEmail::on_sendEmail_clicked()
{
    QString host = ui->host->text();
    int port = ui->port->value();
    bool ssl = ui->ssl->isChecked();
    QString user = ui->username->text();
    QString password = ui->password->text();

    EmailAddress sender = stringToEmail(ui->sender->text());

    QStringList rcptStringList = ui->recipients->text().split(QLatin1Char(';'));

    QString subject = ui->subject->text();
    QString html = ui->texteditor->toHtml();

    Sender smtp(host, port, ssl ? Sender::SslConnection : Sender::TcpConnection);
    if (!user.isEmpty()) {
        smtp.setUser(user);
        smtp.setPassword(password);
    }

    MimeMessage message;

    message.setSender(sender);
    message.setSubject(subject);

    for (int i = 0; i < rcptStringList.size(); ++i)
         message.addTo(stringToEmail(rcptStringList.at(i)));

    MimeHtml content;
    content.setHtml(html);

    message.addPart(&content);

    for (int i = 0; i < ui->attachments->count(); ++i)
    {
        message.addPart(new MimeAttachment(new QFile(ui->attachments->item(i)->text())));
    }

    if (!smtp.sendMail(message)) {
        errorMessage(QLatin1String("Mail sending failed:\n") + smtp.lastError());
        return;
    } else {
        QMessageBox okMessage (this);
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
