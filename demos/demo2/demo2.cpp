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
#include "sendemail.h"

#include <iostream>

#include <QtWidgets/QApplication>

using namespace std;

int main(int argc, char *argv[])
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QCoreApplication::setApplicationName(QStringLiteral("SimpleMailQt"));
    QCoreApplication::setOrganizationName(QStringLiteral("Cutelyst"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("org.cutelyst.simple-mail-qt"));
    QCoreApplication::setApplicationVersion(QStringLiteral("2.0.0"));

    QApplication a(argc, argv);
    SendEmail form;

    form.setWindowTitle(QLatin1String("SimpleMailQt - Demo 2"));

    form.show();

    a.exec();
}
