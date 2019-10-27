/* Copyright (C) 2019 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "callhistoryskill.h"

#include <QDebug>
#include <QObject>
#include <QEventLoop>
#include <QTextStream>
#include <QIODevice>
#include <CommHistory/CallModel>

#include "skillserver.h"
#include "settings.h"

QStringList CallHistorySkill::names()
{
    return {"muki:getCalls", "muki:getMissedCalls"};
}

QString CallHistorySkill::friendlyName()
{
    return tr("Call history");
}

void CallHistorySkill::printEvent(const Event &event)
{
    qDebug() << "event:" << event.id();
    qDebug() << "  type:" << event.type();
    qDebug() << "  category:" << event.category();
    qDebug() << "  contacts:" << event.contacts();
    qDebug() << "  startTime:" << event.startTime();
    qDebug() << "  missedCall:" << event.isMissedCall();
    qDebug() << "  groupId:" << event.groupId();
    qDebug() << "  direction:" << event.direction();
}

CallHistorySkill::Call CallHistorySkill::makeCall(const Event &event)
{
    CallType type = event.isMissedCall() ?
                Missed : event.direction() == Event::Inbound ?
                    Incoming : Outgoing;

    QString contact = event.contactName();
    if (contact.isEmpty()) {
        contact = tr("Unknown");
    }

    return {event.startTime(), type, contact};
}

QList<CallHistorySkill::Call> CallHistorySkill::getCalls(bool onlyMissed,
                                                         const QDateTime &refTime)
{
    QList<CallHistorySkill::Call> list;

    QDateTime rf;
    if (!refTime.isValid()) {
        rf = QDateTime::currentDateTime().addDays(-7);
        rf.setTime({0,0});
    } else {
        rf = refTime;
    }

    CallModel model;
    model.setFilter(CallModel::SortByTime, onlyMissed ?
                        CallEvent::MissedCallType : CallEvent::UnknownCallType, rf);
    model.setResolveContacts(CallModel::ResolveImmediately);
    model.setQueryMode(EventModel::AsyncQuery);

    if (!model.getEvents()) {
        qWarning() << "Error fetching call events";
        return list;
    }

    if (!model.isReady()) {
        QEventLoop loop;
        QObject::connect(&model, &CallModel::modelReady, [&loop](bool successful){
            Q_UNUSED(successful);
            loop.quit();
        });
        loop.exec();
    }

    for (int i = 0; i < model.rowCount() && i < 10; i++) {
        Event e = model.event(model.index(i, 0));
        printEvent(e);
        list.push_back(makeCall(e));
    }

    return list;
}

void CallHistorySkill::handleIntent(const Intent& intent)
{
    QDateTime refTime;
    if (intent.slotList.contains("refTime")) {
        refTime = intent.slotList.value("refTime").toDateTime();
        if (refTime.isValid()) {
            qDebug() << "Got ref time:" << refTime;
        }
    }
    bool missedOnly = intent.name.contains("getMissedCalls");

    auto list = getCalls(missedOnly, refTime);

    QString text;
    QTextStream out(&text, QIODevice::WriteOnly);

    if (list.isEmpty()) {
        if (missedOnly)
            out << tr("You don't have missed calls.");
        else
            out << tr("You don't have call events.");
    } else {
        out << tr("You have %n call(s).", nullptr, list.size());
        for (int i = 1; i <= list.size(); ++i) {
            const auto& call = list[i-1];
            out << " ";
            if (call.type == Missed)
                out << tr("%1. Missed call from %2.").arg(i).arg(call.contact);
            else if (call.type == Incoming)
                out << tr("%1. Incoming call from %2.").arg(i).arg(call.contact);
            else
                out << tr("%1. Outgoing call to %2.").arg(i).arg(call.contact);
        }
    }

    SkillServer::endSession(intent.sessionId, text);
}
