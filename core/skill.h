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
    virtual QStringList intentsNames() const = 0; // without namespace
    virtual QString name() const = 0;
    virtual QString friendlyName() const = 0;
    virtual QString description() const = 0;
    virtual void handleIntent(const Intent& intent) = 0;
    virtual void handleSessionEnded(const QString& sessionId);
};

#endif // SKILL_H
