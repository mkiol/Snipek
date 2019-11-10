/* Copyright (C) 2019 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "skill.h"
#include "skillserver.h"

QStringList Skill::intentsNames()
{
    return QStringList();
}

QString Skill::name()
{
    return QString();
}

QString Skill::friendlyName()
{
    return QString();
}

void Skill::handleIntent(const Intent& intent)
{
    Q_UNUSED(intent);
}

void Skill::handleSessionEnded(const QString& sessionId)
{
    Q_UNUSED(sessionId);
}
