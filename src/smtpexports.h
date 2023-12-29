#pragma once

#include <QtCore/QtGlobal>

#if defined(SIMPLE_MAIL_QT_STATIC)
#    define SMTP_EXPORT
#elif defined(SimpleMail2Qt5_EXPORTS) || defined(SimpleMail2Qt6_EXPORTS)
#    define SMTP_EXPORT Q_DECL_EXPORT
#else
#    define SMTP_EXPORT Q_DECL_IMPORT
#endif
