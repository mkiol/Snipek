/* Copyright (C) 2018 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

#include <QGuiApplication>
#include <QtQml>
#include <QQmlEngine>
#include <QQuickView>
#include <QQmlContext>
#include <QLocale>
#include <QTranslator>

#ifdef Q_OS_SAILFISH
#include <sailfishapp.h>
#endif

#include "audioserver.h"
#include "message.h"
#include "settings.h"
#include "iconprovider.h"

static const char* APP_NAME = "Snipek";
static const char* APP_VERSION = "0.9.1";
static const char* AUTHOR = "Michal Kosciesza <michal@mkiol.net>";
static const char* PAGE = "https://github.com/mkiol";
static const char* LICENSE = "http://mozilla.org/MPL/2.0/";

int main(int argc, char *argv[])
{
#ifdef Q_OS_SAILFISH
    auto app = SailfishApp::application(argc, argv);
    auto view = SailfishApp::createView();
    auto context = view->rootContext();
    auto engine = view->engine();
#else
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    auto app = new QGuiApplication(argc, argv);
    auto engine = new QQmlApplicationEngine();
    auto context = engine->rootContext();
#endif

    app->setApplicationDisplayName(APP_NAME);
    app->setApplicationVersion(APP_VERSION);

    context->setContextProperty("APP_NAME", APP_NAME);
    context->setContextProperty("APP_VERSION", APP_VERSION);
    context->setContextProperty("AUTHOR", AUTHOR);
    context->setContextProperty("PAGE", PAGE);
    context->setContextProperty("LICENSE", LICENSE);

    engine->addImageProvider(QLatin1String("icons"), new IconProvider);

    qRegisterMetaType<Message>("Message");
    qRegisterMetaType<AudioServer::ErrorType>("ErrorType");

    AudioServer* server = AudioServer::instance();
    context->setContextProperty("server", server);
    server->init();

    Settings* settings = Settings::instance();
    context->setContextProperty("settings", settings);

#ifdef Q_OS_SAILFISH
    view->setSource(SailfishApp::pathTo("qml/main.qml"));
    view->show();
#else
    engine->load(QUrl(QLatin1String("qrc:/qml/main.qml")));
    if (engine->rootObjects().isEmpty())
        return -1;
#endif

    return app->exec();
}
