QT += quick dbus

SOURCES += \
        main.cpp

resources.files = main.qml 
resources.files += image_view.qml
resources.prefix = /$${TARGET}
RESOURCES += resources

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    image_view.qml

unix:!macx: LIBS += -L$$PWD/../build-mime_dbus_framework-Desktop_Qt_6_2_4_GCC_64bit-Debug/ -lmime_dbus_framework

INCLUDEPATH += $$PWD/../mime_dbus_framework
DEPENDPATH += $$PWD/../mime_dbus_framework
