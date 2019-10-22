/* Copyright (C) 2019 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SKILL_H
#define SKILL_H

#include <QString>
#include <QStringList>


struct Intent
{
    QString name;
    QString sessionId;
    QString siteId;
};

class Skill
{
public:
    virtual QStringList names();
    virtual QString friendlyName();
    virtual void handleIntent(const Intent& intent);
};

#endif // SKILL_H
