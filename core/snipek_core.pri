CORE_DIR = $$PROJECTDIR/core

INCLUDEPATH += $$CORE_DIR

SOURCES += \
    $$CORE_DIR/main.cpp \
    $$CORE_DIR/audioserver.cpp \
    $$CORE_DIR/mqttagent.cpp \
    $$CORE_DIR/settings.cpp \
    $$CORE_DIR/datetimeskill.cpp \
    $$CORE_DIR/skill.cpp \
    $$CORE_DIR/skillserver.cpp

HEADERS += \
    $$CORE_DIR/audioserver.h \
    $$CORE_DIR/mqttagent.h \
    $$CORE_DIR/message.h \
    $$CORE_DIR/settings.h \
    $$CORE_DIR/info.h \
    $$CORE_DIR/datetimeskill.h \
    $$CORE_DIR/skill.h \
    $$CORE_DIR/skillserver.h

sailfish {
    HEADERS += \
        $$CORE_DIR/iconprovider.h \
        $$CORE_DIR/callhistoryskill.h

    SOURCES += \
        $$CORE_DIR/iconprovider.cpp \
        $$CORE_DIR/callhistoryskill.cpp
}

x86_64 {
    LIBS += -L$$PROJECTDIR/libs/paho.mqtt.c/build/ -lpaho-mqtt3c-static-x86_64
}
x86 {
    LIBS += -L$$PROJECTDIR/libs/paho.mqtt.c/build/ -lpaho-mqtt3c-static-i486
}
arm {
    LIBS += -L$$PROJECTDIR/libs/paho.mqtt.c/build/ -lpaho-mqtt3c-static-armv7hl
}

INCLUDEPATH += $$PROJECTDIR/libs/paho.mqtt.c/src
