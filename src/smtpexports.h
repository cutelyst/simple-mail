#ifndef SMTPEXPORTS_H
#define SMTPEXPORTS_H

#include <QtCore/QtGlobal>

#if defined(SimpleMailQt5_EXPORTS)
#define SMTP_EXPORT Q_DECL_EXPORT
#else
#define SMTP_EXPORT Q_DECL_IMPORT
#endif

#endif // SMTPEXPORTS_H
