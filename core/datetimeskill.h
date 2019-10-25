/* Copyright (C) 2019 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef DATETIMESKILL_H
#define DATETIMESKILL_H

#include <QCoreApplication>
#include <QString>
#include <QStringList>

#include "skill.h"

class DateTimeSkill : public Skill
{
    Q_DECLARE_TR_FUNCTIONS(DateTimeSkill)

public:
    QStringList names();
    QString friendlyName();
    void handleIntent(const Intent& intent);
};

#endif // DATETIMESKILL_H
