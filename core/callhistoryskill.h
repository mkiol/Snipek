/* Copyright (C) 2019 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CALLHISTORYSKILL_H
#define CALLHISTORYSKILL_H

#include <QCoreApplication>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QList>
#include <QDateTime>
#include <QHash>
#include <QTextStream>
#include <CommHistory/Event>

#include "skill.h"

using namespace CommHistory;

class CallHistorySkill : public Skill
{
    Q_DECLARE_TR_FUNCTIONS(CallHistorySkill)

public:
    enum CallType {
      Unknown = 0,
      Incoming,
      Outgoing,
      Missed
    };

    struct Call {
        QDateTime time;
        CallType type;
        QString contact;
    };

    struct SessionData
    {
        QString sessionId;
        int idx = 0;
        QList<Call> callList;
    };

    QStringList names();
    QString friendlyName();
    void handleIntent(const Intent& intent);
    void handleSessionEnded(const QString& sessionId);

private:
    static void printEvent(const Event &event);
    QHash<QString, SessionData> sessions;
    Call makeCall(const Event &event);
    QList<Call> getCalls(bool onlyMissed, const QDateTime &refTime);
    bool readNextCalls(SessionData &session, QTextStream& out, int count);
};

#endif // CALLHISTORYSKILL_H
