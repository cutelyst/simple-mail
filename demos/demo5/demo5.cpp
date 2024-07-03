#include "../../src/SimpleMail"

#include <QtCore>

using namespace SimpleMail;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // This is a demo that shows you how to sign and/or encrypt an email

    // First we need to create an SmtpClient object
    // We will use the Gmail's smtp server (smtp.gmail.com, port 465, ssl)

    Server server;
    server.setHost(QLatin1String("smtp.gmail.com"));
    server.setPort(465);
    server.setConnectionType(Server::SslConnection);

    // We need to set the username (your email address) and password
    // for smtp authentification.

    server.setUsername(QLatin1String("your_email_address@host.com"));
    server.setPassword(QLatin1String("your_password"));

    // Now we create a MimeMessage object. This is the email.

    MimeMessage message;

    EmailAddress sender(QLatin1String("your_email_address@host.com"), QLatin1String("Your Name"));
    message.setSender(sender);

    EmailAddress to(QLatin1String("recipient@host.com"), QLatin1String("Recipient's Name"));
    message.addTo(to);

    message.setSubject(QLatin1String("SmtpClient for Qt - Demo"));

    // Now add some text to the email.
    // First we create a MimeText object.

    auto text = std::make_shared<MimeText>();

    text->setText(QLatin1String("Hi,\nThis is a simple email message.\n"));

    // Now add it to the mail

    message.addPart(text);

    // Add an attachment

    auto document =
        std::make_shared<MimeAttachment>(std::make_shared<QFile>(QLatin1String("document.pdf")));
    message.addPart(document);

    // Now create an SMime object

    auto smime = new SimpleMail::SMime(&message);

    // Setup private and public key/certificate in PKCS#12 format

    smime->setKeyFile(QLatin1String("your_private_key.p12"), QLatin1String("your_private_key_password"));
    smime->setPublicKey(QLatin1String("recipient_public_key.cert"));

    // Sign the message. Only your private key is required.
    // if(!smime->sign()) {
    //     qDebug() << "Failed to create signed email";
    //     delete smime;
    //     return -3;
    // }

    // Encrypt the message. Only the recipient's public key/certificate is required.
    // if(!smime->encrypt()) {
    //     qDebug() << "Failed to create encrypted email";
    //     delete smime;
    //     return -3;
    // }

    // Sign and encrypt the message
    if (!smime->signAndEncrypt()) {
        qDebug() << "Failed to create signed and encrypted email";
        delete smime;
        return -3;
    }

    // Now we can send the mail
    ServerReply *reply = server.sendMail(message);
    QObject::connect(reply, &ServerReply::finished, [=] {
        qDebug() << "ServerReply finished" << reply->error() << reply->responseText();
        reply->deleteLater();
        qApp->exit(reply->error() ? -3 : 0);
    });

    app.exec();
}
