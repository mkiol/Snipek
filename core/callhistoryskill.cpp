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
#include <QDate>
#include <CommHistory/CallModel>

#include "skillserver.h"
#include "settings.h"

QStringList CallHistorySkill::intentsNames()
{
    return {"muki:getCalls", "muki:getMissedCalls"};
}

QString CallHistorySkill::name()
{
    return tr("callhistory");
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

    CallModel model;
    model.setFilter(CallModel::SortByTime, onlyMissed ?
                        CallEvent::MissedCallType : CallEvent::UnknownCallType, refTime);
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

    for (int i = 0; i < model.rowCount() && i < 25; i++) {
        Event e = model.event(model.index(i, 0));
        printEvent(e);
        list.push_back(makeCall(e));
    }

    return list;
}

bool CallHistorySkill::readNextCalls(SessionData& session, QTextStream& out, int count)
{
    auto& i = session.idx;
    auto& list = session.callList;
    int size = list.size();
    const int maxIdx = i + count;

    auto locale = Settings::instance()->locale();

    for (; i < size && i < maxIdx; ++i) {
        const auto& call = list[i];

        auto time = locale.toString(call.time.time(), QLocale::ShortFormat);
        auto date = locale.toString(call.time.date(), "dddd MMMM d");

        if (size == 1)
            out << " ";
        else
            out << " " << (i + 1) << ". ";

        if (QDate::currentDate() == call.time.date()) // today
            out << time << ", ";
        else if (QDate::currentDate().addDays(-1) == call.time.date()) // yesterday
            out << tr("Yesterday") << ", " << time << ", ";
        else
            out << date << ", " << time << ", ";

        if (call.type == Missed)
            out << tr("Missed call from %2.").arg(call.contact);
        else if (call.type == Incoming)
            out << tr("Incoming call from %2.").arg(call.contact);
        else
            out << tr("Outgoing call to %2.").arg(call.contact);
    }

    return i < size;
}

void CallHistorySkill::handleSessionEnded(const QString& sessionId)
{
    if (sessions.contains(sessionId)) {
        qDebug() << "Session ended, so removing session data:" << sessionId;
        sessions.remove(sessionId);
    }
}

void CallHistorySkill::handleIntent(const Intent& intent)
{
    QString text;
    QTextStream out(&text);

    if (intent.name.contains("confirmation") && sessions.contains(intent.sessionId)) {
        qDebug() << "Confirmation intent received:" << intent.slotList.value("answer");
        if (intent.slotList.value("answer") == "yes") {
            if (readNextCalls(sessions[intent.sessionId], out, 1)) {
                qDebug() << "Next confirmation is required";
                out << " " << tr("Continue?");
                SkillServer::askForConfirmation(intent.sessionId, text);
                return;
            } else {
                out << " " << tr("That was the last call event.");
            }
        }
    } else if (intent.name.contains("getCalls") ||
               intent.name.contains("getMissedCalls")) {
        QDateTime refTime;
        bool todayTime = false;
        if (intent.slotList.contains("refTime"))
            refTime = intent.slotList.value("refTime").toDateTime();
        if (refTime.isValid() && QDateTime::currentDateTime() >= refTime) {
            qDebug() << "Intent contains valid ref time:" << refTime;
            if (QDateTime::currentDateTime().date() == refTime.date())
                todayTime = true;
        } else {
            qDebug() << "Ref time is invalid or in the future:" << refTime;
            // default ref time is today
            refTime = QDateTime::currentDateTime();
            refTime.setTime({0,0});
            todayTime = true;
        }

        bool missedOnly = intent.name.contains("getMissedCalls");

        // new session data
        auto& session = sessions[intent.sessionId];
        session.sessionId = intent.sessionId;
        session.callList = getCalls(missedOnly, refTime);

        int size = session.callList.size();
        auto date = Settings::instance()->locale().toString(refTime.date(), "dddd MMMM d");

        if (size == 0) {
            if (missedOnly) {
                if (todayTime)
                    out << tr("You didn't have any missed calls today.");
                else
                    out << tr("You didn't have any missed calls since %1.").arg(date);
            } else {
                if (todayTime)
                    out << tr("You didn't have any calls today.");
                else
                    out << tr("You didn't have any calls since %1.").arg(date);
            }
        } else {
            if (missedOnly) {
                if (todayTime)
                    out << tr("You had %n missed call(s) today.", nullptr, size);
                else
                    out << tr("You had %n missed call(s) since %1.", nullptr, size).arg(date);
            } else {
                if (todayTime)
                    out << tr("You had %n call(s) today.", nullptr, size);
                else
                    out << tr("You had %n call(s) since %1.", nullptr, size).arg(date);
            }

            if (readNextCalls(session, out, 1)) {
                qDebug() << "Confirmation is required";
                out << " " << tr("Continue?");
                SkillServer::askForConfirmation(intent.sessionId, text);
                return;
            }
        }
    }

    SkillServer::endSession(intent.sessionId, text);
}
