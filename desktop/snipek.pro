QT += quick multimedia quickcontrols2
CONFIG += c++11 json

DEFINES += QT_DEPRECATED_WARNINGS

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

OTHER_FILES += \
    ../paho.mqtt.c/src/*.h \
    ../paho.mqtt.c/src/*.c

RESOURCES += qml.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += $$PWD/../paho.mqtt.c/src
DEPENDPATH += $$PWD/../paho.mqtt.c

unix:!macx: LIBS += -L$$PWD/../paho.mqtt.c/lib/ -lpaho-mqtt3c-static-x86_64
unix:!macx: PRE_TARGETDEPS += $$PWD/../paho.mqtt.c/lib/libpaho-mqtt3c-static-x86_64.a

DISTFILES += \
    qtquickcontrols2.conf
