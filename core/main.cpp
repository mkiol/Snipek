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

int main(int argc, char *argv[])
{
#ifdef SAILFISH
    auto app = SailfishApp::application(argc, argv);
    auto view = SailfishApp::createView();
    auto context = view->rootContext();
    auto engine = view->engine();
    engine->addImageProvider(QLatin1String("icons"), new IconProvider);
    //QObject::connect(engine, &QQmlEngine::quit, app, &QCoreApplication::quit);
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
    context->setContextProperty("LICENSE", Snipek::LICENSE);
    context->setContextProperty("LICENSE_URL", Snipek::LICENSE_URL);

    qRegisterMetaType<Message>("Message");
    qRegisterMetaType<AudioServer::ErrorType>("ErrorType");

#ifdef SAILFISH
    QTranslator translator;
    const QString locale = QLocale::system().name();
    const QString transDir = SailfishApp::pathTo("translations").toLocalFile();
    if(translator.load("snipek-" + locale, transDir)) {
        app->installTranslator(&translator);
    } else {
        qWarning() << "Cannot load translation for" << locale;
        if (translator.load("snipek", transDir)) {
            app->installTranslator(&translator);
        } else {
            qWarning() << "Cannot load default translation";
        }
    }
#endif

    auto mqtt = MqttAgent::instance();
    context->setContextProperty("mqtt", mqtt);
    mqtt->init();

    auto aserver = AudioServer::instance();
    context->setContextProperty("aserver", aserver);
    aserver->init();

    auto skills = SkillServer::instance();
    context->setContextProperty("skills", skills);

    auto settings = Settings::instance();
    context->setContextProperty("settings", settings);

#ifdef SAILFISH
    view->setSource(SailfishApp::pathTo("qml/main.qml"));
    view->show();
#else
    engine->load(QUrl(QLatin1String("qrc:/qml/main.qml")));
    if (engine->rootObjects().isEmpty())
        return -1;
#endif

    return app->exec();
}
