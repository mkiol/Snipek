/* Copyright (C) 2019 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SNIPSLOCALAGENT_H
#define SNIPSLOCALAGENT_H

#include <queue>
#include <QObject>
#include <QThread>

class SnipsLocalAgent : public QThread
{
    Q_OBJECT
    Q_PROPERTY (SnipsStatus snipsStatus READ getSnipsStatus NOTIFY snipsChanged)
public:
    enum SnipsStatus {
        SnipsUnknown = 0,
        SnipsNotInstalled,
        SnipsOutdated,
        SnipsStopped,
        SnipsStarted
    };
    Q_ENUM(SnipsStatus)

    static SnipsLocalAgent* instance(QObject* parent = nullptr);
    SnipsLocalAgent::SnipsStatus getSnipsStatus();

public slots:
    void checkSnips();
    void startSnips();
    void stopSnips();
    void shutdown();
    void installSnips();
    void installAssistant();

private slots:
    void handleSettingsChange();

signals:
    void snipsChanged();

private:
    enum RequestType {
        RequestUnknown = 0,
        RequestCheck,
        RequestStart,
        RequestStop,
        RequestInstall,
        RequestInstallAssistant
    };
    static const QString installScript;
    static const QString startScript;
    static SnipsLocalAgent* inst;
    std::queue<RequestType> reqQueue;
    SnipsStatus status = SnipsUnknown;
    explicit SnipsLocalAgent(QObject *parent = nullptr);
    void run();
    int execScript(const QStringList& params);
    int doCheckRunning();
    int doCheckInstalled();
    int doStop();
    int doStart();
};

#endif // SNIPSLOCALAGENT_H
