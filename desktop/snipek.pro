CONFIG += c++11 json x86_64 desktop
QT += core quick multimedia quickcontrols2

DEFINES += QT_DEPRECATED_WARNINGS DESKTOP

PROJECTDIR = $$PWD/..

INCLUDEPATH += /usr/include/c++/7

include($$PROJECTDIR/core/snipek_core.pri)

DISTFILES += \
    qml/main.qml \
    qml/AboutPage.qml \
    qml/SettingsPage.qml \
    qml/PaddedLabel.qml \
    qml/FirstPage.qml \
    qml/Mic.qml \
    qml/PageHeader.qml \
    qml/Theme.qml

OTHER_FILES += \
    ../paho.mqtt.c/src/*.h \
    ../paho.mqtt.c/src/*.c \
    rpm/*

RESOURCES += qml.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    qtquickcontrols2.conf
