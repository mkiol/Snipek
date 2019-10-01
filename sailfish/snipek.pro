TARGET = harbour-snipek

CONFIG += c++11 sailfishapp json
QT += multimedia

PKGCONFIG += mlite5

linux-g++-32: CONFIG += x86
linux-g++: CONFIG += arm

DEFINES += SAILFISH

PROJECTDIR = $$PWD/..

INCLUDEPATH += /usr/include/c++/9

CONFIG += sailfish
DEFINES += SAILFISH

include($$PROJECTDIR/core/snipek_core.pri)

DISTFILES += \
    qml/main.qml \
    qml/AboutPage.qml \
    qml/Notification.qml \
    qml/SettingsPage.qml \
    qml/Spacer.qml \
    qml/PaddedLabel.qml \
    qml/FirstPage.qml \
    qml/Mic.qml \
    qml/CoverPage

OTHER_FILES += \
    ../paho.mqtt.c/src/*.h \
    ../paho.mqtt.c/src/*.c

OTHER_FILES += \
    harbour-snipek.desktop \
    rpm/$${TARGET}.yaml \
    rpm/$${TARGET}.changes.in \
    rpm/$${TARGET}.spec
    translations/*.ts

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172

CONFIG += sailfishapp_i18n
TRANSLATIONS += \
    translations/snipek.ts \
    translations/snipek-pl.ts \
    translations/snipek-ru.ts \
    translations/snipek-de.ts \
    translations/snipek-es.ts

images.files = images/*
images.path = /usr/share/$${TARGET}/images
INSTALLS += images

DEPENDPATH += $$INCLUDEPATH
