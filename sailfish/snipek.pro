TARGET = harbour-snipek

CONFIG += c++11 sailfishapp json
QT += multimedia

PKGCONFIG += mlite5 commhistory-qt5 Qt5Contacts

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
    qml/CoverPage.qml \
    qml/ChangelogPage.qml \
    qml/LogItem.qml \
    qml/DirPage.qml \
    qml/SimpleListItem.qml

OTHER_FILES += \
    $$PROJECTDIR/paho.mqtt.c/src/*.h \
    $$PROJECTDIR/paho.mqtt.c/src/*.c \
    $$PROJECTDIR/README.md

OTHER_FILES += \
    harbour-snipek.desktop \
    rpm/$${TARGET}.yaml \
    rpm/$${TARGET}.changes.in \
    rpm/$${TARGET}.spec \
    translations/*.ts


SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172 256x256

# Languages supported by Snips:
# de (German)
# en (English)
# es (Spanish)
# fr (French)
# it (Italian)
# ja (Japanese)
# pt_BR (Brazilian Portuguese)

TRANSLATION_SOURCE_DIRS += $$PROJECTDIR/core
CONFIG += sailfishapp_i18n_include_obsolete
TRANSLATIONS += \
    translations/snipek-de.ts \
    translations/snipek-en.ts \
    translations/snipek-es.ts \
    translations/snipek-fr.ts \
    translations/snipek-it.ts \
    translations/snipek-ja.ts \
    translations/snipek-it.ts \
    translations/snipek-pt_BR.ts
include(sailfishapp_i18n.pri)

images.files = images/*
images.path = /usr/share/$${TARGET}/images
INSTALLS += images

snips.files = snips/*.sh
snips.path = /usr/share/$${TARGET}/snips
INSTALLS += snips

DEPENDPATH += $$INCLUDEPATH
