/* Copyright (C) 2019 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QObject>
#include <QTime>
#include <QDateTime>
#include <QTextStream>
#include <QIODevice>

#include "datetimeskill.h"
#include "skillserver.h"
#include "settings.h"

QStringList DateTimeSkill::names()
{
    return {"muki:getTime", "muki:getDate"};
}

QString DateTimeSkill::friendlyName()
{
    return tr("Date and Time");
}

void DateTimeSkill::handleIntent(const Intent& intent)
{
    QString text;
    QTextStream out(&text, QIODevice::WriteOnly);

    auto locale = Settings::instance()->locale();

    if (intent.name == "muki:getTime") {
        auto time = locale.toString(QTime::currentTime(), QLocale::ShortFormat);
        //: Do not translate if language is not supported by Snips
        out << tr("It is %1.").arg(time);
    } else if (intent.name == "muki:getDate") {
        auto date = locale.toString(QDateTime::currentDateTime().date(),
                                    QLocale::LongFormat);
        //: Do not translate if language is not supported by Snips
        out << tr("It is %1.").arg(date);
    }

    SkillServer::endSession(intent.sessionId, text);
}
