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
#include <queue>

#include "MQTTClient.h"
#include "message.h"

class MqttAgent : public QThread
{
    Q_OBJECT
    Q_PROPERTY (bool connected READ isConnected NOTIFY connectedChanged)
public:
    enum ErrorType {
        E_Unknown = 0,
        E_NoAddr,
        E_Conn
    };

    static MqttAgent* instance(QObject* parent = nullptr);
    bool isConnected();
    void publish(const Message &msg);
    void subscribe(const QString &topic);
    void unsubscribe(const QString &topic);

public slots:
    bool init();
    void deInit();

signals:
    void message(const Message &msg);
    void audioServerMessage(const Message &msg);
    void dialogueManagerMessage(const Message &msg);
    void ttsMessage(const Message &msg);
    void asrMessage(const Message &msg);
    void intentMessage(const Message &msg);
    void connectedChanged();
    void error(ErrorType error);

private:
    static MqttAgent* inst;
    MQTTClient client = nullptr;
    int id = 0;
    bool connected = false;
    bool shutdown = false;
    std::queue<Message> msgQueue;
    std::queue<QString> subscribeQueue;
    std::queue<QString> unsubscribeQueue;

    bool checkConnected();
    void publishAll();
    void subscribeAll();
    void unsubscribeAll();
    void receive();
    MqttAgent(QObject* parent = nullptr);
    void run();
};

#endif // MQTTAGENT_H
