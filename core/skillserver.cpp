/* Copyright (C) 2019 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextStream>
#include <QRegExp>
#include <QTimer>

#include "skillserver.h"
#include "settings.h"
#include "datetimeskill.h"
#ifdef SAILFISH
#include "callhistoryskill.h"
#endif

SkillItem::SkillItem(const Skill *skill,
                     QObject *parent) :
    ListItem(parent),
    skill(skill)
{}

QHash<int, QByteArray> SkillItem::roleNames() const
{
    QHash<int, QByteArray> names;
    names[NameRole] = "name";
    names[FriendlyNameRole] = "friendlyName";
    names[DescriptionRole] = "description";
    return names;
}

QVariant SkillItem::data(int role) const
{
    switch(role) {
    case NameRole:
        return skill->name();
    case FriendlyNameRole:
        return skill->friendlyName();
    case DescriptionRole:
        return skill->description();
    default:
        return QVariant();
    }
}

SkillServer* SkillServer::inst = nullptr;

SkillServer* SkillServer::instance(QObject* parent)
{
    if (SkillServer::inst == nullptr) {
        SkillServer::inst = new SkillServer(parent);
    }

    return SkillServer::inst;
}

SkillServer::SkillServer(QObject *parent) : ListModel(new SkillItem, parent)
{
    // namespace
    auto s = Settings::instance();
    ns = s->getIntentNs();

    // skills
    registerSkill(new DateTimeSkill());
#ifdef SAILFISH
    registerSkill(new CallHistorySkill());
#endif

    // mqtt
    auto mqtt = MqttAgent::instance();
    connect(mqtt, &MqttAgent::intentMessage,
            this, &SkillServer::processMessage);
    connect(mqtt, &MqttAgent::dialogueManagerMessage,
            this, &SkillServer::processMessage);
    connect(mqtt, &MqttAgent::stateChanged,
            this, &SkillServer::handleMqttStateChange);
    handleMqttStateChange();

    // settings events
    connect(s, &Settings::skillEnabledChanged,
            this, &SkillServer::handleSkillsChange);
    connect(s, &Settings::intentNsChanged,
            this, &SkillServer::handleNsChange);
}

QString SkillServer::addNs(const QString& name)
{
    return ns + ":" + name;
}

QString SkillServer::removeNs(const QString& name)
{
    return name.contains(':') ? name.split(':').last() : name;
}

void SkillServer::handleSkillsChange()
{
    if (MqttAgent::instance()->getState() == MqttAgent::MqttConnected)
        subscribe();
}

void SkillServer::handleNsChange()
{
    auto mqtt = MqttAgent::instance();
    if (mqtt->getState() == MqttAgent::MqttConnected) {
        unsubscribe();
        mqtt->shutdown();
    }

    ns = Settings::instance()->getIntentNs();
}

void SkillServer::registerSkill(Skill* skill)
{
    for (auto name : skill->intentsNames())
        intentNameToSkills.insert(name, skill);
    appendRow(new SkillItem(skill));
}

void SkillServer::processMessage(const Message &msg)
{
    qDebug() << "Processing message:" << msg.topic;
    qDebug() << msg.payload;

    const auto st = QString(msg.topic).split('/');

    if (st.size() > 2 && st.at(0) == "hermes") {
        if (st.at(1) == "intent")
            parseIntent(msg.payload);
        else if (st.at(2) == "sessionEnded")
            parseSessionEnded(msg.payload);
    } else {
        qWarning() << "Unknown message received";
    }

    return;
}

void SkillServer::subscribe()
{
    auto mqtt = MqttAgent::instance();
    auto s = Settings::instance();

    QString tmpl = "hermes/intent/%1";

    mqtt->subscribe(tmpl.arg(addNs("confirmation"))); // confirmation intent

    QHashIterator<QString, Skill*> i(intentNameToSkills);
    while (i.hasNext()) {
        i.next();
        if (s->isSkillEnabled(i.value()->name()))
            mqtt->subscribe(tmpl.arg(addNs(i.key())));
        else
            mqtt->unsubscribe(tmpl.arg(addNs(i.key())));
    }
}

void SkillServer::unsubscribe()
{
    auto mqtt = MqttAgent::instance();
    auto s = Settings::instance();

    QString tmpl = "hermes/intent/%1";

    mqtt->unsubscribe(tmpl.arg(addNs("confirmation"))); // confirmation intent

    QHashIterator<QString, Skill*> i(intentNameToSkills);
    while (i.hasNext()) {
        i.next();
        if (s->isSkillEnabled(i.value()->name()))
            mqtt->unsubscribe(tmpl.arg(addNs(i.key())));
    }
}

void SkillServer::handleMqttStateChange()
{
    if (MqttAgent::instance()->getState() == MqttAgent::MqttConnected) {
        subscribe();
    } else {
        // TODO reset skills
    }
}

void SkillServer::endSession(const QString& sessionId, const QString& text)
{
    auto ss = SkillServer::instance();
    if (ss->sessionIdToSkills.contains(sessionId)) {
        Message msg;
        msg.topic = "hermes/dialogueManager/endSession";
        msg.payload = text.isEmpty() ?
                    QString("{\"sessionId\":\"%1\"}").arg(sessionId).toUtf8() :
                    QString("{\"sessionId\":\"%1\",\"text\":\"%2\"}")
                    .arg(sessionId).arg(text).toUtf8();

        auto mqtt = MqttAgent::instance();
        mqtt->publish(msg);
    } else {
        qWarning() << "Session doesn't exists";
    }
}

void SkillServer::continueSession(const QString& sessionId,
                                  const QString& text,
                                  const QStringList& intentFilters)
{
    auto ss = SkillServer::instance();
    if (ss->sessionIdToSkills.contains(sessionId)) {
        Message msg;
        msg.topic = "hermes/dialogueManager/continueSession";
        if (intentFilters.isEmpty()) {
            msg.payload = QString("{\"sessionId\":\"%1\",\"text\":\"%2\"}")
                    .arg(sessionId).arg(text).toUtf8();
        } else {
            QStringList list(intentFilters);
            list.replaceInStrings(QRegExp("^(.*)$"), "\"" + ss->addNs("\\1\""));
            msg.payload = QString("{\"sessionId\":\"%1\",\"text\":\"%2\",\"intentFilter\":[%3]}")
                            .arg(sessionId).arg(text).arg(list.join(',')).toUtf8();
        }

        auto mqtt = MqttAgent::instance();
        mqtt->publish(msg);
    } else {
        qWarning() << "Session doesn't exists";
    }
}

void SkillServer::askForConfirmation(const QString& sessionId, const QString& text)
{
    continueSession(sessionId, text, {"confirmation"});
}

void SkillServer::parseSessionEnded(const QByteArray& data)
{
    QJsonParseError err;
    auto json = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError) {
        qWarning() << "Error parsing json payload:" << err.errorString();
        return;
    }

    if (!json.isObject()) {
        qWarning() << "Json is not an object";
        return;
    }

    auto sessionId = json.object().value("sessionId").toString();

    if (sessionId.isEmpty()) {
        qWarning() << "SessionId is empty";
        return;
    }

    handleSessionEnded(sessionId);
}

void SkillServer::handleSessionEnded(const QString& sessionId)
{
    if (sessionIdToSkills.contains(sessionId)) {
        qDebug() << "Session exists, so will be removed:" << sessionId;
        auto skill = sessionIdToSkills[sessionId];
        skill->handleSessionEnded(sessionId);
        sessionIdToSkills.remove(sessionId);
    } else {
        qDebug() << "Session doesn't exist";
    }
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
    intent.name = removeNs(obj.value("intent").toObject().value("intentName").toString());

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


    QJsonArray slotArr = obj.value("slots").toArray();
    if (!slotArr.isEmpty()) {
        auto name = slotArr[0].toObject().value("slotName").toString();
        auto value = slotArr[0].toObject().value("value").toObject().value("value").toVariant();
        intent.slotList.insert(name, value);
    }

    handleIntent(intent);
}

void SkillServer::handleIntent(const Intent& intent)
{
    if (sessionIdToSkills.contains(intent.sessionId)) {
        qDebug() << "Session exists for intent";
        auto skill = sessionIdToSkills[intent.sessionId];
        skill->handleIntent(intent);
    } else if (intentNameToSkills.contains(intent.name)) {
        qDebug() << "New session for intent";
        auto skill = intentNameToSkills[intent.name];
        sessionIdToSkills.insert(intent.sessionId, skill);
        skill->handleIntent(intent);
    } else {
        qDebug() << "Do not have skill for intent";
        endSession(intent.sessionId);
    }
}
