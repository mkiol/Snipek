/* Copyright (C) 2019 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QDebug>
#include <QProcess>
#include <QString>
#include <QStringList>

#include "snipslocalagent.h"
#include "settings.h"

SnipsLocalAgent* SnipsLocalAgent::inst = nullptr;
const QString SnipsLocalAgent::installScript("/usr/share/harbour-snipek/snips/snips_download.sh");
const QString SnipsLocalAgent::startScript("/usr/share/harbour-snipek/snips/snips_start.sh");

SnipsLocalAgent* SnipsLocalAgent::instance(QObject* parent)
{
    if (SnipsLocalAgent::inst == nullptr) {
        SnipsLocalAgent::inst = new SnipsLocalAgent(parent);
    }

    return SnipsLocalAgent::inst;
}

SnipsLocalAgent::SnipsLocalAgent(QObject *parent) : QThread(parent),
    reqQueue()
{
    auto s = Settings::instance();
    connect(s, &Settings::snipsLocalChanged, this, &SnipsLocalAgent::handleSettingsChange);
    if (s->getSnipsLocal()) {
        this->startSnips();
        this->checkSnips();
    }
}

void SnipsLocalAgent::shutdown()
{
    if (status == SnipsStarted) {
        this->stopSnips();
        wait();
    }
}

void SnipsLocalAgent::handleSettingsChange()
{
    auto s = Settings::instance();
    if (s->getSnipsLocal()) {
        this->startSnips();
        this->checkSnips();
    } else {
        this->stopSnips();
    }
}

SnipsLocalAgent::SnipsStatus SnipsLocalAgent::getSnipsStatus()
{
    return status;
}

void SnipsLocalAgent::checkSnips()
{
    reqQueue.push(RequestCheck);
    if (!isRunning()) {
        start();
    }
}

void SnipsLocalAgent::startSnips()
{
    reqQueue.push(RequestStart);
    if (!isRunning())
        start();
}

void SnipsLocalAgent::stopSnips()
{
    reqQueue.push(RequestStop);
    if (!isRunning())
        start();
}

void SnipsLocalAgent::installSnips()
{
    reqQueue.push(RequestInstall);
    if (!isRunning())
        start();
}

void SnipsLocalAgent::installAssistant()
{
    reqQueue.push(RequestInstallAssistant);
    if (!isRunning())
        start();
}

int SnipsLocalAgent::execScript(const QStringList& params)
{
    QProcess process;
    process.start("bash", params);
    process.waitForFinished(2000);
    qDebug() << "proces exit:" << process.exitCode();
    return process.exitCode();
}

int SnipsLocalAgent::doCheckInstalled()
{
    qDebug() << "Checking if Snips is installed...";
    auto s = Settings::instance();
    QStringList params;
    params << installScript << "-c" << "-d" << s->getSnipsLocalDir();
    return execScript(params);
}

int SnipsLocalAgent::doCheckRunning()
{
    qDebug() << "Checking if Snips is running...";
    auto s = Settings::instance();
    QStringList params;
    params << startScript << "-c" << "-d" << s->getSnipsLocalDir();
    return execScript(params);
}

int SnipsLocalAgent::doStop()
{
    qDebug() << "Stopping Snips...";
    auto s = Settings::instance();
    QStringList params;
    params << startScript << "-k" << "-d" << s->getSnipsLocalDir();
    return execScript(params);
}

int SnipsLocalAgent::doStart()
{
    qDebug() << "Starting Snips...";
    auto s = Settings::instance();
    QStringList params;
    params << startScript << "-d" << s->getSnipsLocalDir();
    return execScript(params);
}

void SnipsLocalAgent::run()
{
    while (!reqQueue.empty()) {
        SnipsStatus newStatus = status;
        RequestType reqType = reqQueue.front();
        switch (reqType) {
        case RequestCheck:
        {
            int inst_code = doCheckInstalled();
            if (inst_code == 0) {
                int runn_code = doCheckRunning();
                newStatus = runn_code == 0 ? SnipsStarted : SnipsStopped;
            } else {
                newStatus = inst_code == 2 ? SnipsOutdated : SnipsNotInstalled;
            }
            break;
        }
        case RequestStart:
            newStatus = doStart() == 0 ? SnipsStarted : SnipsStopped;
            break;
        case RequestStop:
            doStop();
            newStatus = SnipsStopped;
            break;
        case RequestInstall:
            // TODO: Implement Snips installation
        case RequestInstallAssistant:
            // TODO: Implement Snips installation
        default:
            qWarning() << "Unknown request type";
        }

        if (status != newStatus) {
            status = newStatus;
            emit snipsChanged();
        }

        reqQueue.pop();
    }
}
