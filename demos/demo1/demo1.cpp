#include <QtCore>

#include "../../src/SmtpMime"

using namespace SimpleMail;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // This is a first demo application of the SmtpClient for Qt project


    // First we need to create an SmtpClient object
    // We will use the Gmail's smtp server (smtp.gmail.com, port 465, ssl)

    Sender smtp(QLatin1String("smtp.gmail.com"), 465, Sender::SslConnection);

    // We need to set the username (your email address) and password
    // for smtp authentification.

    smtp.setUser(QLatin1String("your_email_address@host.com"));
    smtp.setPassword(QLatin1String("your_password"));

    // Now we create a MimeMessage object. This is the email.

    MimeMessage message;

    EmailAddress sender(QLatin1String("your_email_address@host.com"), QLatin1String("Your Name"));
    message.setSender(sender);

    EmailAddress to(QLatin1String("recipient@host.com"), QLatin1String("Recipient's Name"));
    message.addTo(to);

    message.setSubject(QLatin1String("SmtpClient for Qt - Demo"));

    // Now add some text to the email.
    // First we create a MimeText object.

    MimeText text;

    text.setText(QLatin1String("Hi,\nThis is a simple email message.\n"));

    // Now add it to the mail

    message.addPart(&text);

    // Now we can send the mail
    if (!smtp.sendMail(message)) {
        qDebug() << "Failed to send mail!" << smtp.lastError();
        return -3;
    }

    smtp.quit();

}
