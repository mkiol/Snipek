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
#include <QHash>
#include <QVariant>

struct Intent
{
    QString name;
    QString sessionId;
    QString siteId;
    QHash<QString, QVariant> slotList;
};

class Skill
{
public:
    virtual QStringList names();
    virtual QString friendlyName();
    virtual void handleIntent(const Intent& intent);
    virtual void handleSessionEnded(const QString& sessionId);
};

#endif // SKILL_H
