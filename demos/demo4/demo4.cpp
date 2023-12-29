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

    message.setSubject(QLatin1String("SmtpClient for Qt - Example 4 - Html email with images"));

    // Now we need to create a MimeHtml object for HTML content
    auto html = std::make_shared<MimeHtml>();

    html->setHtml(QLatin1String("<h1> Hello! </h1>"
                                "<h2> This is the first image </h2>"
                                "<img src='cid:image1' />"
                                "<h2> This is the second image </h2>"
                                "<img src='cid:image2' />"));

    // Create a MimeInlineFile object for each image
    auto image1 =
        std::make_shared<MimeInlineFile>(std::make_shared<QFile>(QLatin1String("image1.jpg")));

    // An unique content id must be setted
    image1->setContentId(QByteArrayLiteral("image1"));
    image1->setContentType(QByteArrayLiteral("image/jpeg"));

    auto image2 =
        std::make_shared<MimeInlineFile>(std::make_shared<QFile>(QLatin1String("image2.jpg")));
    image2->setContentId(QByteArrayLiteral("image2"));
    image2->setContentType(QByteArrayLiteral("image/jpeg"));

    message.addPart(html);
    message.addPart(image1);
    message.addPart(image2);

    // Now we can send the mail
    ServerReply *reply = server.sendMail(message);
    QObject::connect(reply, &ServerReply::finished, [=] {
        qDebug() << "ServerReply finished" << reply->error() << reply->responseText();
        reply->deleteLater();
        qApp->exit(reply->error() ? -3 : 0);
    });

    app.exec();
}
