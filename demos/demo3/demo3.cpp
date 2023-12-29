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

#include "../../src/SimpleMail"

#include <QtCore>

using namespace SimpleMail;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    Server server;
    server.setHost(QLatin1String("smtp.gmail.com"));
    server.setPort(465);
    server.setConnectionType(Server::SslConnection);

    server.setUsername(QLatin1String("your_email@host.com"));
    server.setPassword(QLatin1String("your_password"));

    // Create a MimeMessage

    MimeMessage message;

    EmailAddress sender(QLatin1String("your_email_address@host.com"), QLatin1String("Your Name"));
    message.setSender(sender);

    EmailAddress to(QLatin1String("recipient@host.com"), QLatin1String("Recipient's Name"));
    message.addTo(to);

    message.setSubject(QLatin1String("SmtpClient for Qt - Demo"));

    // Add some text
    auto text = std::make_shared<MimeText>();
    text->setText(QLatin1String("Hi!\n This is an email with some attachments."));
    message.addPart(text);

    // Now we create the attachment object
    auto attachment =
        std::make_shared<MimeAttachment>(std::make_shared<QFile>(QLatin1String("image1.jpg")));

    // the file type can be setted. (by default is application/octet-stream)
    attachment->setContentType(QByteArrayLiteral("image/jpeg"));

    // Now add it to message
    message.addPart(attachment);

    // Add an another attachment
    auto document =
        std::make_shared<MimeAttachment>(std::make_shared<QFile>(QLatin1String("document.pdf")));
    message.addPart(document);

    // Now we can send the mail
    ServerReply *reply = server.sendMail(message);
    QObject::connect(reply, &ServerReply::finished, [=] {
        qDebug() << "ServerReply finished" << reply->error() << reply->responseText();
        reply->deleteLater();
        qApp->exit(reply->error() ? -3 : 0);
    });

    app.exec();
}
