#include "../../src/SimpleMail"

#include <QtCore>

using namespace SimpleMail;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // First we need to create a Server object
    // We will use the Gmail's smtp server (smtp.gmail.com, port 465, ssl)

    Server server;
    server.setHost(QLatin1String("smtp.gmail.com"));
    server.setPort(465);
    server.setConnectionType(Server::SslConnection);

    server.setUsername(QLatin1String("your_email@host.com"));
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

    // Now we can send the mail
    ServerReply *reply = server.sendMail(message);
    QObject::connect(reply, &ServerReply::finished, [=] {
        qDebug() << "ServerReply finished" << reply->error() << reply->responseText();
        reply->deleteLater();
        qApp->exit(reply->error() ? -3 : 0);
    });

    app.exec();
}
