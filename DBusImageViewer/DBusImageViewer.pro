QT += quick dbus

SOURCES += \
        applicationInterface.cpp \
        main.cpp

resources.files += main.qml
resources.files += image_view.qml
resources.prefix = /$${TARGET}
RESOURCES += resources


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


DISTFILES += \
    image_view.qml

unix:!macx: LIBS += -L$$PWD/../build-mime_dbus_framewrork-Desktop_Qt_6_2_4_GCC_64bit-Debug/ -lmime_dbus_framewrork

INCLUDEPATH += $$PWD/../mime_dbus_framewrork
DEPENDPATH += $$PWD/../mime_dbus_framewrork

HEADERS += \
    applicationInterface.h
