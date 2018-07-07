/* Copyright (C) 2018 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "settings.h"

Settings* Settings::inst = nullptr;

Settings::Settings(QObject* parent) :
    QObject(parent),
    settings(parent)
{
}

Settings* Settings::instance(QObject* parent)
{
    if (Settings::inst == nullptr) {
        Settings::inst = new Settings(parent);
    }

    return Settings::inst;
}

QString Settings::getSite()
{
    return settings.value("site", "snipek").toString();
}

void Settings::setSite(const QString &value)
{
    QString v = value.trimmed();
    if (getSite() != v) {
        if (v.isEmpty())
            v = "snipek";
        settings.setValue("site", v);
        emit siteChanged();
    }
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
