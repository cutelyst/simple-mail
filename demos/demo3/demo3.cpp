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

    message.setSubject(QLatin1String("SmtpClient for Qt - Demo"));

    // Add some text
    MimeText text;
    text.setText(QLatin1String("Hi!\n This is an email with some attachments."));
    message.addPart(&text);

    // Now we create the attachment object
    MimeAttachment attachment(new QFile(QLatin1String("image1.jpg")));

    // the file type can be setted. (by default is application/octet-stream)
    attachment.setContentType(QByteArrayLiteral("image/jpg"));

    // Now add it to message
    message.addPart(&attachment);

    // Add an another attachment
    MimeAttachment document(new QFile(QLatin1String("document.pdf")));
    message.addPart(&document);

    // Now we can send the mail
    if (!smtp.sendMail(message)) {
        qDebug() << "Failed to send mail!" << smtp.lastError();
        return -3;
    }

    smtp.quit();
}
