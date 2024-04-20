QT -= gui

TEMPLATE = lib
DEFINES += MIME_DBUS_FRAMEWRORK_LIBRARY

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    mime_dbus_framewrork.cpp

HEADERS += \
    mime_dbus_framewrork_global.h \
    mime_dbus_framewrork.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
