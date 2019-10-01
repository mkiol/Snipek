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
#include <mutex>

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

public slots:
    bool init();
    void deInit();

signals:
    void message(int id);
    void connectedChanged();
    void error(ErrorType error);

private:
    static MqttAgent* inst;
    std::mutex mutex;
    MQTTClient client = nullptr;
    int id = 0;
    bool connected = false;
    std::queue<Message> msgQueue;

    void publishQueue();
    void receive();
    MqttAgent(QObject* parent = nullptr);
    void run();
};

#endif // MQTTAGENT_H
