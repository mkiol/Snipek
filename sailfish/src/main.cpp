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

#include <sailfishapp.h>

#include "audioserver.h"
#include "message.h"
#include "settings.h"
#include "iconprovider.h"

static const char* APP_NAME = "Snipek";
static const char* APP_VERSION = "0.9.0";
static const char* AUTHOR = "Michal Kosciesza <michal@mkiol.net>";
static const char* PAGE = "https://github.com/mkiol";
static const char* LICENSE = "http://mozilla.org/MPL/2.0/";

int main(int argc, char *argv[])
{
    QGuiApplication* app = SailfishApp::application(argc, argv);
    QQuickView* view = SailfishApp::createView();
    auto context = view->rootContext();
    auto engine = view->engine();

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

    view->setSource(SailfishApp::pathTo("qml/main.qml"));
    view->show();

    return app->exec();
}
