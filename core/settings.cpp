/* Copyright (C) 2018 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QTime>

#include "settings.h"

Settings* Settings::inst = nullptr;

Settings::Settings(QObject* parent) :
    QObject(parent),
    settings(parent)
{
    // Seed init, needed for rand
    //qsrand(QTime::currentTime().msec());
}

Settings* Settings::instance(QObject* parent)
{
    if (Settings::inst == nullptr) {
        Settings::inst = new Settings(parent);
    }

    return Settings::inst;
}

QString Settings::getRandId()
{
    return "snipek-" + randString();
}

QString Settings::getSite()
{
    QString v = settings.value("site").toString().trimmed();
    if (v.isEmpty()) {
        v = getRandId();
        settings.setValue("site", v);
    }

    return v;
}

void Settings::setSite(const QString &value)
{
    QString v = value.trimmed();
    if (getSite() != v) {
        if (v.isEmpty())
            v = getRandId();
        settings.setValue("site", v);
        emit siteChanged();
    }
}

QString Settings::getMqttId()
{
    QString v = settings.value("mqttid").toString().trimmed();
    if (v.isEmpty()) {
        v = getRandId();
        settings.setValue("mqttid", v);
    }

    return v;
}

QString Settings::getMqttAddress()
{
    return settings.value("mqttaddress", "").toString();
}

void Settings::setMqttAddress(const QString& value)
{
    QString v = value.trimmed();
    if (getMqttAddress() != v) {
        settings.setValue("mqttaddress", v);
        emit mqttChanged();
    }
}

int Settings::getMqttPort()
{
    return settings.value("mqttport", 1883).toInt();
}

void Settings::setMqttPort(int value)
{
    if (value < 1 || value > 65535)
        value = 1883;

    if (getMqttPort() != value) {
        settings.setValue("mqttport", value);
        emit mqttChanged();
    }
}

QString Settings::getInAudio()
{
    return settings.value("inaudio", "").toString();
}

void Settings::setInAudio(const QString& value)
{
    if (getInAudio() != value) {
        settings.setValue("inaudio", value);
        emit audioChanged();
    }
}

QString Settings::getOutAudio()
{
    return settings.value("outaudio", "").toString();
}

void Settings::setOutAudio(const QString& value)
{
    if (getOutAudio() != value) {
        settings.setValue("outaudio", value);
        emit audioChanged();
    }
}

QString Settings::randString(int len)
{
   qsrand(QTime::currentTime().msec());
   const QString pc("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");

   QString rs;

   for(int i = 0; i < len; ++i) {
       int in = qrand() % pc.length();
       QChar nc = pc.at(in);
       rs.append(nc);
   }

   return rs;
}
