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

#ifdef SAILFISH
#include <sailfishapp.h>
#include "iconprovider.h"
#endif

#include "info.h"
#include "audioserver.h"
#include "message.h"
#include "settings.h"
#include "mqttagent.h"
#include "skillserver.h"
#include "dirmodel.h"
#include "snipslocalagent.h"

int main(int argc, char *argv[])
{
#ifdef SAILFISH
    auto app = SailfishApp::application(argc, argv);
    auto view = SailfishApp::createView();
    auto context = view->rootContext();
    auto engine = view->engine();
    engine->addImageProvider(QLatin1String("icons"), new IconProvider);
    //QObject::connect(engine, &QQmlEngine::quit, app, &QCoreApplication::quit);
    qmlRegisterType<DirModel>("harbour.snipek.DirModel", 1, 0, "DirModel");
    qmlRegisterUncreatableType<SnipsLocalAgent>("harbour.snipek.Snips",
         1, 0, "Snips", "Not creatable as it is an enum type");
#else
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    auto app = new QGuiApplication(argc, argv);
    auto engine = new QQmlApplicationEngine();
    auto context = engine->rootContext();
    app->setOrganizationName(app->applicationName());
    QObject::connect(engine, &QQmlApplicationEngine::quit, app, &QCoreApplication::quit);
#endif

    context->setContextProperty("APP_NAME", Snipek::APP_NAME);
    context->setContextProperty("APP_VERSION", Snipek::APP_VERSION);
    context->setContextProperty("COPYRIGHT_YEAR", Snipek::COPYRIGHT_YEAR);
    context->setContextProperty("AUTHOR", Snipek::AUTHOR);
    context->setContextProperty("SUPPORT_EMAIL", Snipek::SUPPORT_EMAIL);
    context->setContextProperty("PAGE", Snipek::PAGE);
    context->setContextProperty("PAGE_SNIPS_INSTALL", Snipek::PAGE_SNIPS_INSTALL);
    context->setContextProperty("LICENSE", Snipek::LICENSE);
    context->setContextProperty("LICENSE_URL", Snipek::LICENSE_URL);

    qRegisterMetaType<Message>("Message");
    qRegisterMetaType<AudioServer::ErrorType>("ErrorType");

    auto settings = Settings::instance();
    context->setContextProperty("settings", settings);

#ifdef SAILFISH
    QTranslator translator;
    auto transDir = SailfishApp::pathTo("translations").toLocalFile();
    auto locale = settings->locale().name();
    if(!translator.load(locale, "snipek", "-", transDir, ".qm")) {
        qDebug() << "Cannot load translation:" << locale << transDir;
        settings->setNoTranslation();
        if (!translator.load("snipek-en", transDir)) {
            qDebug() << "Cannot load default translation";
        }
    }
    app->installTranslator(&translator);

    auto snips = SnipsLocalAgent::instance();
    context->setContextProperty("snips", snips);
#endif

    auto mqtt = MqttAgent::instance();
    context->setContextProperty("mqtt", mqtt);
    mqtt->initWithReconnect();

    auto aserver = AudioServer::instance();
    context->setContextProperty("aserver", aserver);
    aserver->init();

    auto skills = SkillServer::instance();
    context->setContextProperty("skills", skills);

#ifdef SAILFISH
    view->setSource(SailfishApp::pathTo("qml/main.qml"));
    view->show();
#else
    engine->load(QUrl(QLatin1String("qrc:/qml/main.qml")));
    if (engine->rootObjects().isEmpty())
        return -1;
#endif

    int ret = app->exec();

    snips->shutdown();
    return ret;
}
