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

#include <QtCore>

#include "../../src/SmtpMime"

using namespace SimpleMail;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // First create the SmtpClient object and set the user and the password.

    Sender smtp(QLatin1String("smtp.gmail.com"), 465, Sender::SslConnection);

    smtp.setUser(QLatin1String("your_email@host.com"));
    smtp.setPassword(QLatin1String("your_password"));

    // Create a MimeMessage

    MimeMessage message;

    EmailAddress sender(QLatin1String("your_email_address@host.com"), QLatin1String("Your Name"));
    message.setSender(sender);

    EmailAddress to(QLatin1String("recipient@host.com"), QLatin1String("Recipient's Name"));
    message.addTo(to);

    message.setSubject(QLatin1String("SmtpClient for Qt - Example 4 - Html email with images"));

    // Now we need to create a MimeHtml object for HTML content
    MimeHtml html;

    html.setHtml(QLatin1String("<h1> Hello! </h1>"
                               "<h2> This is the first image </h2>"
                               "<img src='cid:image1' />"
                               "<h2> This is the second image </h2>"
                               "<img src='cid:image2' />"));


    // Create a MimeInlineFile object for each image
    MimeInlineFile image1 (new QFile(QLatin1String("image1.jpg")));

    // An unique content id must be setted
    image1.setContentId(QByteArrayLiteral("image1"));
    image1.setContentType(QByteArrayLiteral("image/jpg"));

    MimeInlineFile image2 (new QFile(QLatin1String("image2.jpg")));
    image2.setContentId(QByteArrayLiteral("image2"));
    image2.setContentType(QByteArrayLiteral("image/jpg"));

    message.addPart(&html);
    message.addPart(&image1);
    message.addPart(&image2);

    // Now the email can be sended
    if (!smtp.sendMail(message)) {
        qDebug() << "Failed to send mail!" << smtp.lastError();
        return -3;
    }

    smtp.quit();

}
