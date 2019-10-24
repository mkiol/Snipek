/* Copyright (C) 2019 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SKILLSERVER_H
#define SKILLSERVER_H

#include <QObject>
#include <QHash>
#include <QLocale>

#include "mqttagent.h"
#include "skill.h"

class SkillServer : public QObject
{
    Q_OBJECT
public:
    static SkillServer* instance(QObject* parent = nullptr);
    static void endSession(const QString& sessionId, const QString& text = QString());
    static void continueSession(const QString& sessionId, const QString& text);
    static QString translate(const char *text, const QLocale& locale);
    static QString translate(const char *text);

public slots:
    void processMessage(const Message& msg);

private slots:
    void mqttConnectedHandler();

private:
    static SkillServer* inst;
    explicit SkillServer(QObject *parent = nullptr);
    QHash<QString, Skill*> skills;

    void subscribe();
    void parseIntent(const QByteArray& data);
    void handleIntent(const Intent& intent);
    void registerSkill(Skill* skill);
};

#endif // SKILLSERVER_H
