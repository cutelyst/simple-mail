#pragma once

#include <QtCore/QtGlobal>

#if defined(SIMPLE_MAIL_QT_STATIC)
#    define SMTP_EXPORT
#elif defined(SimpleMail3Qt5_EXPORTS) || defined(SimpleMail3Qt6_EXPORTS)
#    define SMTP_EXPORT Q_DECL_EXPORT
#else
#    define SMTP_EXPORT Q_DECL_IMPORT
#endif
