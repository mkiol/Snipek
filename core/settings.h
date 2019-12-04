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
    Q_PROPERTY (QString lang READ getLang WRITE setLang NOTIFY langChanged)
    Q_PROPERTY (QString intentNs READ getIntentNs WRITE setIntentNs NOTIFY intentNsChanged)
    Q_PROPERTY (QString snipsLocalDir READ getSnipsLocalDir WRITE setSnipsLocalDir NOTIFY snipsLocalChanged)
    Q_PROPERTY (int mqttPort READ getMqttPort WRITE setMqttPort NOTIFY mqttChanged)
    Q_PROPERTY (bool audioFeedback READ getAudioFeedback WRITE setAudioFeedback NOTIFY audioFeedbackChanged)
    Q_PROPERTY (int sessionStart READ getSessionStart WRITE setSessionStart NOTIFY sessionStartChanged)
    Q_PROPERTY (bool snipsLocal READ getSnipsLocal WRITE setSnipsLocal NOTIFY snipsLocalChanged)

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
    QString getLang();
    void setLang(const QString& value);
    QLocale locale();
    bool isLangSupportedBySnips(const QString &langName);
    void setNoTranslation() {noTranslation = true;}
    int getSessionStart();
    void setSessionStart(int value);
    Q_INVOKABLE bool isSkillEnabled(const QString& name);
    Q_INVOKABLE void setSkillEnabled(const QString& name, bool value);
    QString getIntentNs();
    void setIntentNs(const QString& value);
    bool getSnipsLocal();
    void setSnipsLocal(bool value);
    QString getSnipsLocalDir();
    void setSnipsLocalDir(const QString& value);
    QString getSnipsLocalDirDefault();
signals:
    void siteChanged();
    void mqttChanged();
    void audioChanged();
    void audioFeedbackChanged();
    void langChanged();
    void sessionStartChanged();
    void skillEnabledChanged();
    void intentNsChanged();
    void snipsLocalChanged();

private:
#ifdef SAILFISH
    static constexpr char const * HW_RELEASE_FILE = "/etc/hw-release";
#endif
    QSettings settings;
    bool noTranslation = false;
    static Settings* inst;
    QString hwName;

    explicit Settings(QObject* parent = nullptr);
    QString getRandId();
    QString getNewSiteId();
    QString randString(int len = 4);
    bool checkIntentNs(const QString& ns);
#ifdef SAILFISH
    QString readHwInfo();
#endif
};

#endif // SETTINGS_H
