/* Copyright (C) 2018 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QDebug>
#include <QTime>
#include <QStringList>
#include <QLocale>
#include <QSysInfo>
#include <QStandardPaths>
#include <QDir>
#ifdef SAILFISH
#include <QRegExp>
#include <QFile>
#endif

#include "settings.h"

Settings* Settings::inst = nullptr;

/* Languages supported by Snips:
de (German)
en (English)
es (Spanish)
fr (French)
it (Italian)
ja (Japanese)
pt_BR (Brazilian Portuguese) */

QStringList Settings::snipsLangs = {
    "auto",
    "de",
    "en",
    "es",
    "fr",
    "it",
    "ja",
    "pt_br"
};

Settings::Settings(QObject* parent) :
    QObject(parent),
    settings(parent)
{
#ifdef SAILFISH
    hwName = readHwInfo();
#else
    hwName = QSysInfo::machineHostName();
#endif
    qDebug() << "HW name:" << hwName;
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

QString Settings::getNewSiteId()
{
    auto id = hwName.replace(QRegExp("[^a-zA-Z0-9]"), "_");
    return id.isEmpty() ? getRandId() : ("snipek-" + id);
}

QString Settings::getSite()
{
    QString v = settings.value("site").toString().trimmed();
    if (v.isEmpty()) {
        v = getNewSiteId();
        settings.setValue("site", v);
    }

    return v;
}

void Settings::setSite(const QString &value)
{
    QString v = value.trimmed();
    if (v.isEmpty()) {
        qWarning() << "Resetting siteId";
        v = getNewSiteId();
    }

    if (getSite() != v) {
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
    if (getSnipsLocal())
        return "127.1.0.0";
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
    if (getSnipsLocal())
        return 1883;
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

bool Settings::getAudioFeedback()
{
    return settings.value("audiofeedback", true).toBool();
}

void Settings::setAudioFeedback(bool value)
{
    if (getAudioFeedback() != value) {
        settings.setValue("audiofeedback", value);
        emit audioFeedbackChanged();
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

QLocale Settings::locale()
{
    if (noTranslation) {
        return QLocale(QLocale::English); // default
    }

    auto lang = getLang();

    if (lang == "auto") {
        auto sysLocale = QLocale::system();
        if (isLangSupportedBySnips(sysLocale.name()))
            return sysLocale;
        return QLocale(QLocale::English); // default
    }

    return QLocale(lang);
}

bool Settings::isLangSupportedBySnips(const QString &langName)
{
    for (const auto& l : snipsLangs) {
        if (l.contains(langName, Qt::CaseInsensitive))
            return true;
    }

    return false;
}

QString Settings::getLang()
{
    return settings.value("lang", "auto").toString();
}

void Settings::setLang(const QString& value)
{
    if (isLangSupportedBySnips(value)) {
        if (getLang() != value) {
            settings.setValue("lang", value);
            emit langChanged();
        }
    } else {
        qWarning() << "Lang is not supported by Snips:" << value;
    }
}

int Settings::getSessionStart()
{
    /* 0 - hot-word & tap
     * 1 - only tap
     * 2 - only hot-word */
    return settings.value("sessionstart", 0).toInt();
}

void Settings::setSessionStart(int value)
{
    if (value < 0 || value > 2)
        value = 0;

    if (getSessionStart() != value) {
        settings.setValue("sessionstart", value);
        emit sessionStartChanged();
    }
}

bool Settings::isSkillEnabled(const QString &name)
{
    return settings.value("skill_" + name, true).toBool();
}

void Settings::setSkillEnabled(const QString &name, bool value)
{
    if (isSkillEnabled(name) != value) {
        settings.setValue("skill_" + name, value);
        emit skillEnabledChanged();
    }
}

bool Settings::checkIntentNs(const QString& ns)
{
    if (ns.isEmpty())
        return false;

    for (const auto& c : ns)
        if (!c.isLetterOrNumber())
            return false;

    return true;
}

QString Settings::getIntentNs()
{
    return settings.value("intentns", "muki").toString();
}

void Settings::setIntentNs(const QString& value)
{
    auto ns = value.trimmed().toLower();
    if (!checkIntentNs(ns)) {
        qWarning() << "Namespace for intents is invalid";
        ns = "muki";
    }

    if (getIntentNs() != ns) {
        settings.setValue("intentns", ns);
        emit intentNsChanged();
    }
}

bool Settings::getSnipsLocal()
{
    return settings.value("snipslocal", false).toBool();
}

void Settings::setSnipsLocal(bool value)
{
    if (getSnipsLocal() != value) {
        settings.setValue("snipslocal", value);
        emit snipsLocalChanged();
    }
}

QString Settings::getSnipsLocalDirDefault()
{
    return QDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation))
            .absoluteFilePath("snips");
}

QString Settings::getSnipsLocalDir()
{
    auto dir = settings.value("snipslocaldir", QString()).toString();
    return dir.isEmpty() ? getSnipsLocalDirDefault() : dir;
}

void Settings::setSnipsLocalDir(const QString& value)
{
    if (getSnipsLocalDir() != value) {
        settings.setValue("snipslocaldir", value);
        emit snipsLocalChanged();
    }
}

#ifdef SAILFISH
QString Settings::readHwInfo()
{
    QFile f(HW_RELEASE_FILE);
    if (f.open(QIODevice::ReadOnly)) {
        auto d = f.readAll();
        f.close();
        auto data = QString::fromUtf8(d).split(QRegExp("[\r\n]"), QString::SkipEmptyParts);
        QRegExp rx("^NAME=\"?([^\"]*)\"?$", Qt::CaseInsensitive);
        for (const auto& line: data) {
            if (rx.indexIn(line) != -1)
                return rx.cap(1);
        }
    } else {
        qWarning() << "Cannot open file" << f.fileName() <<
                      "for reading (" + f.errorString() + ")";
    }

    return QString();
}
#endif
