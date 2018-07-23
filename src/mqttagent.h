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

#include "MQTTClient.h"
#include "message.h"

class MqttAgent : public QObject
{
    Q_OBJECT
public:
    enum ErrorType {
        E_Unknown = 0,
        E_NoAddr,
        E_Conn
    };

    static MqttAgent* instance(QObject* parent = nullptr);

    bool isConnected();
    void receive();

public slots:
    void publish(Message &msg);
    bool init();
    void deInit();

signals:
    void message(int id);
    void connectedChanged(bool value);
    void error(ErrorType error);

private:
    static MqttAgent* inst;
    MQTTClient client = nullptr;
    int id = 0;
    bool connected = false;

    MqttAgent(QObject* parent = nullptr);
};

#endif // MQTTAGENT_H
