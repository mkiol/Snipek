/* Copyright (C) 2019 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>

#include "skillserver.h"
#include "settings.h"
#include "datetimeskill.h"

SkillServer* SkillServer::inst = nullptr;

SkillServer* SkillServer::instance(QObject* parent)
{
    if (SkillServer::inst == nullptr) {
        SkillServer::inst = new SkillServer(parent);
    }

    return SkillServer::inst;
}

SkillServer::SkillServer(QObject *parent) : QObject(parent)
{
    // skills

    registerSkill(new DateTimeSkill());

    // mqtt

    auto mqtt = MqttAgent::instance();
    connect(mqtt, &MqttAgent::intentMessage,
            this, &SkillServer::processMessage);
    connect(mqtt, &MqttAgent::connectedChanged,
            this, &SkillServer::mqttConnectedHandler);
    mqttConnectedHandler();
}

void SkillServer::registerSkill(Skill* skill)
{
    for (auto name : skill->names()) {
        skills.insert(name, skill);
    }
}

void SkillServer::processMessage(const Message &msg)
{
    qDebug() << "Processing message:" << msg.topic;
    qDebug() << msg.payload;

    const auto st = QString(msg.topic).split('/');

    if (st.size() > 2 && st.at(0) == "hermes" && st.at(1) == "intent") {
        parseIntent(msg.payload);
    } else {
        qWarning() << "Unknown message received";
    }

    return;
}

void SkillServer::subscribe()
{
    auto mqtt = MqttAgent::instance();

    QString tmpl = "hermes/intent/%1";

    QHashIterator<QString, Skill*> i(skills);
    while (i.hasNext()) {
        i.next();
        mqtt->subscribe(tmpl.arg(i.key()));
    }
}

void SkillServer::mqttConnectedHandler()
{
    if (MqttAgent::instance()->isConnected()) {
        subscribe();
    }
}

void SkillServer::endSession(const QString& sessionId, const QString& text)
{
    Message msg;
    msg.topic = "hermes/dialogueManager/endSession";
    msg.payload = text.isEmpty() ?
                QString("{\"sessionId\":\"%1\"}").arg(sessionId).toUtf8() :
                QString("{\"sessionId\":\"%1\",\"text\":\"%2\"}")
                .arg(sessionId).arg(text).toUtf8();

    auto mqtt = MqttAgent::instance();
    mqtt->publish(msg);
}

void SkillServer::continueSession(const QString& sessionId, const QString& text)
{
    Message msg;
    msg.topic = "hermes/dialogueManager/continueSession";
    msg.payload = QString("{\"sessionId\":\"%1\",\"text\":\"%2\"}")
            .arg(sessionId).arg(text).toUtf8();

    auto mqtt = MqttAgent::instance();
    mqtt->publish(msg);
}

void SkillServer::parseIntent(const QByteArray& data)
{
    QJsonParseError err;
    auto json = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError) {
        qWarning() << "Error parsing json payload:" << err.errorString();
        return;
    }

    if (!json.isObject()) {
        qWarning() << "Json is not a object";
        return;
    }

    auto obj = json.object();

    Intent intent;
    intent.siteId = obj.value("siteId").toString();
    intent.sessionId = obj.value("sessionId").toString();
    intent.name = obj.value("intent").toObject().value("intentName").toString();

    if (intent.sessionId.isEmpty()) {
        qWarning() << "SessionId is empty";
        return;
    }

    if (intent.siteId.isEmpty() || intent.name.isEmpty()) {
        qWarning() << "SiteId or intent name is empty";
        endSession(intent.sessionId);
        return;
    }

    qDebug() << "New intent received:" << intent.siteId <<
                intent.sessionId << intent.name;

    if (intent.siteId != Settings::instance()->getSite()) {
        qWarning() << "Intent site ID is not my side ID";
        return;
    }

    if (!skills.contains(intent.name)) {
        qDebug() << "Doesn't have skill for intent";
        endSession(intent.sessionId);
        return;
    }

    handleIntent(intent);
}

void SkillServer::handleIntent(const Intent& intent)
{
    auto skill = skills[intent.name];
    skill->handleIntent(intent);
}
