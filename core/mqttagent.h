/* Copyright (C) 2018 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef MQTTAGENT_H
#define MQTTAGENT_H

#include <QDebug>
#include <QObject>
#include <QThread>
#include <QTimer>
#include <QString>
#include <QByteArray>
#include <queue>

#include "MQTTClient.h"
#include "message.h"

class MqttAgent : public QThread
{
    Q_OBJECT
    Q_PROPERTY (MqttState state READ getState NOTIFY stateChanged)
public:
    enum MqttState {
        MqttUnknown = 0,
        MqttDisconnected,
        MqttConnecting,
        MqttConnected,
        MqttShutdowning
    };
    Q_ENUM(MqttState)

    enum ErrorType {
        E_Unknown = 0,
        E_NoAddr,
        E_Conn
    };

    static MqttAgent* instance(QObject* parent = nullptr);
    MqttState getState();
    void publish(const Message &msg);
    void subscribe(const QString &topic);
    void unsubscribe(const QString &topic);

public slots:
    void init();
    void shutdown();

signals:
    void message(const Message &msg);
    void audioServerMessage(const Message &msg);
    void dialogueManagerMessage(const Message &msg);
    void ttsMessage(const Message &msg);
    void asrMessage(const Message &msg);
    void intentMessage(const Message &msg);
    void stateChanged();
    void error(ErrorType error);
    void doReconnectLater();

private:
    struct Action {
        enum Type {
            Unknown = 0,
            Connect,
            Shutdown
        };
        Type type = Unknown;
        QByteArray url;
        Action(Type type, QByteArray url = QByteArray()) :
            type(type), url(url) {}
    };

    static MqttAgent* inst;
    MQTTClient client = nullptr;
    int id = 0;
    MqttState state = MqttDisconnected;
    std::queue<Action> actionQueue;
    std::queue<Message> msgQueue;
    std::queue<QString> subscribeQueue;
    std::queue<QString> unsubscribeQueue;
    int reconCounter = 0;
    QTimer reconTimer;
    QByteArray url;

    void checkActions();
    void publishAll();
    void subscribeAll();
    void unsubscribeAll();
    void receive();
    MqttAgent(QObject* parent = nullptr);
    void run();
    QByteArray makeUrl();

private slots:
    void reconnect();
    void reconnectLater();
};

#endif // MQTTAGENT_H
