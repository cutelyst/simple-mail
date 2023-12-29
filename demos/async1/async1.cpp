#include "../../src/SimpleMail"

#include <QCoreApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // First we need to create an Server object
    auto server = new SimpleMail::Server;

    // We will use the Gmail's smtp server (smtp.gmail.com, port 465, ssl)
    server->setHost(QLatin1String("smtp.gmail.com"));
    server->setPort(465);
    server->setConnectionType(SimpleMail::Server::SslConnection);

    // We need to set the username (your email address) and the password for smtp authentification.
    server->setUsername(QLatin1String("your_email_address@gmail.com"));
    server->setPassword(QLatin1String("your_password"));

    // Now we create a MimeMessage object. This will be the email.
    SimpleMail::MimeMessage message;
    message.setSender(SimpleMail::EmailAddress(QLatin1String("your_email_address@gmail.com"),
                                               QLatin1String("Your Name")));
    message.addTo(SimpleMail::EmailAddress(QLatin1String("Recipient's Name <recipient@host.com>")));
    message.setSubject(QLatin1String("Testing Subject"));

    // First we create a MimeText object.
    // This must be created with new otherwise it will be deleted once we leave the scope.
    auto text = std::make_shared<SimpleMail::MimeText>();

    // Now add some text to the email.
    text->setText(QLatin1String("Hi,\nThis is a simple email message.\n"));

    // Now add it to the mail
    message.addPart(text);

    // Now we can send the mail
    SimpleMail::ServerReply *reply = server->sendMail(message);
    QObject::connect(reply, &SimpleMail::ServerReply::finished, [reply] {
        qDebug() << "ServerReply finished" << reply->error() << reply->responseText();
        reply->deleteLater(); // Don't forget to delete it

        qApp->quit();
    });

    app.exec();
}
