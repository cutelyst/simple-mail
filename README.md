SimpleMail [![Build Status](https://travis-ci.org/cutelyst/simple-mail.svg?branch=master)](https://travis-ci.org/cutelyst/simple-mail)
=============================================

The SimpleMail is small library writen for Qt 5 or 6 (C++11) that allows applications to send complex emails (plain text, html, attachments, inline files, etc.) using the Simple Mail Transfer Protocol (SMTP).

## Features:

- Blocking and Asyncronous modes
- SMTP pipelining
- TCP and SSL connections to SMTP servers (STARTTLS included)
- SMTP authentication (PLAIN, LOGIN, CRAM-MD5 methods)
- sending MIME emails (to multiple recipients)
- plain text and HTML (with inline files) content in emails
- nested mime emails (mixed/alternative, mixed/related)
- multiple attachments and inline files (used in HTML)
- different character sets (ascii, utf-8, etc) and encoding methods (7bit, 8bit, base64)
- multiple types of recipients (to, cc, bcc)
- error handling (including RESET command)
- output compilant with RFC2045

## Examples

Async Exaplame:

```c++
#include <QCoreApplication>
#include <SimpleMail/SimpleMail>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // First we need to create an Server object
    auto server = new SimpleMail::Server;

    // We will use the Gmail's smtp server (smtp.gmail.com, port 465, ssl)
    server->setHost("smtp.gmail.com");
    server->setPort(465);
    server->setConnectionType(SimpleMail::Server::SslConnection);

    // We need to set the username (your email address) and the password for smtp authentification.
    server->setUsername("your_email_address@gmail.com");
    server->setPassword("your_password");

    // Now we create a MimeMessage object. This will be the email.
    SimpleMail::MimeMessage message;
    message.setSender(SimpleMail::EmailAddress("your_email_address@gmail.com", "Your Name"));
    message.addTo(SimpleMail::EmailAddress("Recipient's Name <recipient@host.com>"));
    message.setSubject("Testing Subject");

    // First we create a MimeText object.
    // This must be created with new otherwise it will be deleted once we leave the scope.
    auto text = new MimeText;

    // Now add some text to the email.
    text->setText("Hi,\nThis is a simple email message.\n");

    // Now add it to the mail
    message.addPart(text);

    // Now we can send the mail
    SimpleMail::ServerReply *reply = server->sendMail(message);
    QObject::connect(reply, &SimpleMail::ServerReply::finished, [reply] {
        qDebug() << "ServerReply finished" << reply->error() << reply->responseText();
        reply->deleteLater();// Don't forget to delete it

        qApp->quit();
    });

    app.exec();
}
```

Blocking example:

```c++
#include <QCoreApplication>
#include <SimpleMail/SimpleMail>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // First we need to create an Sender object
    // We will use the Gmail's smtp server (smtp.gmail.com, port 465, ssl)
    SimpleMail::Sender sender("smtp.gmail.com", 465, SimpleMail::Sender::SslConnection);

    // We need to set the username (your email address) and the password
    // for smtp authentification.
    sender.setUser("your_email_address@gmail.com");
    sender.setPassword("your_password");

    // Now we create a MimeMessage object. This will be the email.
    SimpleMail::MimeMessage message;
    message.setSender(SimpleMail::EmailAddress("your_email_address@gmail.com", "Your Name"));
    message.addTo(SimpleMail::EmailAddress("Recipient's Name <recipient@host.com>"));
    message.setSubject("Testing Subject");

    // First we create a MimeText object.
    SimpleMail::MimeText text;

    // Now add some text to the email.
    text.setText("Hi,\nThis is a simple email message.\n");

    // Now add it to the mail
    message.addPart(&text);

    // Now we can send the mail
    sender.sendMail(message); // Blocks untill mail is delivered or errored
}
```

## License

This project (all files including the demos/examples) is licensed under the GNU LGPL, version 2.1+.
