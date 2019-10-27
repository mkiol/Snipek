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

    static void printEvent(const Event &event);
    QStringList names();
    QString friendlyName();
    void handleIntent(const Intent& intent);

private:
    Call makeCall(const Event &event);
    QList<Call> getCalls(bool onlyMissed = false, const QDateTime &refTime = QDateTime());
};

#endif // CALLHISTORYSKILL_H
