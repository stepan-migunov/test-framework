#ifndef MIME_DBUS_FRAMEWORK_GLOBAL_H
#define MIME_DBUS_FRAMEWORK_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(MIME_DBUS_FRAMEWORK_LIBRARY)
#  define MIME_DBUS_FRAMEWORK_EXPORT Q_DECL_EXPORT
#else
#  define MIME_DBUS_FRAMEWORK_EXPORT Q_DECL_IMPORT
#endif

#endif // MIME_DBUS_FRAMEWORK_GLOBAL_H