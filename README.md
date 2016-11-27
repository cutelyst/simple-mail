
SimpleMail [![Build Status](https://travis-ci.org/cutelyst/simple-mail.svg?branch=master)](https://travis-ci.org/cutelyst/simple-mail)
=============================================

The SimpleMail is small library writen for Qt 5 (C++11 version) that allows application to send complex emails (plain text, html, attachments, inline files, etc.) using the Simple Mail Transfer Protocol (SMTP).

## Features:

- TCP and SSL connections to SMTP servers (STARTTLS included)

- SMTP authentication (PLAIN and LOGIN methods)

- sending MIME emails (to multiple recipients)

- plain text and HTML (with inline files) content in emails

- nested mime emails (mixed/alternative, mixed/related)

- multiple attachments and inline files (used in HTML)

- different character sets (ascii, utf-8, etc) and encoding methods (7bit, 8bit, base64)

- multiple types of recipients (to, cc, bcc)

- error handling

- output compilant with RFC2045

## Examples

Lets see a simple example:

```c++
#include <QCoreApplication>
#include "../src/SmtpMime"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // This is a first demo application of the SmtpClient for Qt project

    // First we need to create an SmtpClient object
    // We will use the Gmail's smtp server (smtp.gmail.com, port 465, ssl)

    SimpleMail::Sender sender("smtp.gmail.com", 465, SimpleMail::Sender::SslConnection);

    // We need to set the username (your email address) and the password
    // for smtp authentification.

    sender.setUser("your_email_address@gmail.com");
    sender.setPassword("your_password");

    // Now we create a MimeMessage object. This will be the email.

    MimeMessage message;

    message.setSender(EmailAddress("your_email_address@gmail.com", "Your Name"));
    message.addTo(EmailAddress("recipient@host.com", "Recipient's Name"));
    message.setSubject("SmtpClient for Qt - Demo");

    // Now add some text to the email.
    // First we create a MimeText object.

    MimeText text;

    text.setText("Hi,\nThis is a simple email message.\n");

    // Now add it to the mail

    message.addPart(&text);

    // Now we can send the mail
    sender.sendMail(message);

}
```

## License

This project (all files including the demos/examples) is licensed under the GNU LGPL, version 2.1+.
