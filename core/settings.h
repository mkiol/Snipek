/* Copyright (C) 2018 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QSettings>
#include <QLocale>

class Settings: public QObject
{
    Q_OBJECT
    Q_PROPERTY (QString site READ getSite WRITE setSite NOTIFY siteChanged)
    Q_PROPERTY (QString mqttAddress READ getMqttAddress WRITE setMqttAddress NOTIFY mqttChanged)
    Q_PROPERTY (QString inAudio READ getInAudio WRITE setInAudio NOTIFY audioChanged)
    Q_PROPERTY (QString outAudio READ getOutAudio WRITE setOutAudio NOTIFY audioChanged)
    Q_PROPERTY (QString snipsLang READ getSnipsLang WRITE setSnipsLang NOTIFY snipsLangChanged)
    Q_PROPERTY (int mqttPort READ getMqttPort WRITE setMqttPort NOTIFY mqttChanged)
    Q_PROPERTY (bool audioFeedback READ getAudioFeedback WRITE setAudioFeedback NOTIFY audioFeedbackChanged)

public:
    static Settings* instance(QObject* parent = nullptr);
    static QStringList snipsLangs;

    QString getMqttId();
    QString getSite();
    void setSite(const QString& value);
    QString getMqttAddress();
    void setMqttAddress(const QString& value);
    int getMqttPort();
    void setMqttPort(int value);
    QString getInAudio();
    void setInAudio(const QString& value);
    QString getOutAudio();
    void setOutAudio(const QString& value);
    bool getAudioFeedback();
    void setAudioFeedback(bool value);
    QString getSnipsLang();
    void setSnipsLang(const QString& value);
    QLocale locale();
    bool isLangSupportedBySnips(const QString &langName);

signals:
    void siteChanged();
    void mqttChanged();
    void audioChanged();
    void audioFeedbackChanged();
    void snipsLangChanged();

private:
    QSettings settings;
    static Settings* inst;

    explicit Settings(QObject* parent = nullptr);
    QString getRandId();
    QString randString(int len = 4);
};

#endif // SETTINGS_H
