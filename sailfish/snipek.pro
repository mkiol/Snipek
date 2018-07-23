TARGET = harbour-snipek

CONFIG += sailfishapp json

DEFINES += Q_OS_SAILFISH

QT += multimedia

PKGCONFIG += mlite5

linux-g++-32 {
    LIBS += -L$$PWD/../paho.mqtt.c/lib/ -lpaho-mqtt3c-static-i486
}

linux-g++ {
    LIBS += -L$$PWD/../paho.mqtt.c/lib/ -lpaho-mqtt3c-static-armv7hl
}

INCLUDEPATH += $$PWD/../paho.mqtt.c/src
INCLUDEPATH += /usr/include/c++/7

SOURCES += \
    ../src/main.cpp \
    ../src/audioserver.cpp \
    ../src/taskexecutor.cpp \
    ../src/mqttagent.cpp \
    ../src/settings.cpp \
    ../src/iconprovider.cpp

HEADERS += \
    ../src/audioserver.h \
    ../src/taskexecutor.h \
    ../src/mqttagent.h \
    ../src/message.h \
    ../src/settings.h \
    ../src/iconprovider.h

DISTFILES += \
    harbour-snipek.desktop \
    rpm/$${TARGET}.yaml \
    rpm/$${TARGET}.changes.in \
    rpm/$${TARGET}.spec \
    translations/*.ts \
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

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172

images.files = images/*
images.path = /usr/share/$${TARGET}/images
INSTALLS += images

DEPENDPATH += $$INCLUDEPATH
