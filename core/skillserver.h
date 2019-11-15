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
#include <QStringList>
#include <QByteArray>
#include <QVariant>

#include "listmodel.h"
#include "mqttagent.h"
#include "skill.h"

class SkillItem : public ListItem
{
    Q_OBJECT
public:
    enum Roles {
        FriendlyNameRole = Qt::DisplayRole,
        NameRole = Qt::UserRole,
        DescriptionRole
    };

public:
    SkillItem(QObject *parent = nullptr): ListItem(parent) {}
    explicit SkillItem(const Skill *skill,
                       QObject *parent = nullptr);
    QVariant data(int role) const;
    QHash<int, QByteArray> roleNames() const;
    inline QString id() const { return skill->name(); }
    inline QString name() const { return skill->name(); }
    inline QString friendlyName() const { return skill->friendlyName(); }
    inline QString description() const { return skill->description(); }

private:
    const Skill* skill;
};

class SkillServer : public ListModel
{
    Q_OBJECT
public:
    static SkillServer* instance(QObject* parent = nullptr);
    static void endSession(const QString& sessionId,
                           const QString& text = QString());
    static void continueSession(const QString& sessionId,
                                const QString& text,
                                const QStringList& intentFilters = QStringList());
    static void askForConfirmation(const QString& sessionId,
                                   const QString& text);

public slots:
    void processMessage(const Message& msg);

private slots:
    void mqttConnectedHandler();
    void handleSkillsChange();
    void handleNsChange();

private:
    static SkillServer* inst;
    explicit SkillServer(QObject *parent = nullptr);
    QHash<QString, Skill*> intentNameToSkills;
    QHash<QString, Skill*> sessionIdToSkills;
    QString ns;

    QString addNs(const QString &name);
    QString removeNs(const QString &name);
    void subscribe();
    void unsubscribe();
    void parseSessionEnded(const QByteArray& data);
    void handleSessionEnded(const QString& sessionId);
    void parseIntent(const QByteArray& data);
    void handleIntent(const Intent& intent);
    void registerSkill(Skill *skill);
};

#endif // SKILLSERVER_H
