/* Copyright (C) 2018 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QThread>
#include <QByteArray>
#include <QCoreApplication>

#include "mqttagent.h"
#include "settings.h"

MqttAgent* MqttAgent::inst = nullptr;

MqttAgent* MqttAgent::instance(QObject* parent)
{
    if (MqttAgent::inst == nullptr) {
        MqttAgent::inst = new MqttAgent(parent);
    }

    return MqttAgent::inst;
}

MqttAgent::MqttAgent(QObject *parent) :
    QThread(parent)
{
    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit,
            this, &MqttAgent::deInit);

    auto settings = Settings::instance();
    connect(settings, &Settings::mqttChanged, this, &MqttAgent::deInit);
    connect(settings, &Settings::siteChanged, this, &MqttAgent::deInit);
}

bool MqttAgent::init()
{
    deInit();

    qDebug() << "MQTT init";

    auto settings = Settings::instance();

    QString addr = settings->getMqttAddress();
    int port = settings->getMqttPort();

    if (addr.isEmpty() || port < 1) {
        qWarning() << "No MQTT addres or port defined";
        emit error(E_NoAddr);
        return false;
    }

    QByteArray url = QString("tcp://%1:%2").arg(addr).arg(port).toLatin1();

    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

    QByteArray id = settings->getMqttId().toUtf8();
    qDebug() << "MQTT ID:" << id;

    MQTTClient_create(&client, url.data(), id.data(),
        MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 10;
    conn_opts.cleansession = 1;
    conn_opts.connectTimeout = 1;
    conn_opts.retryInterval = 5;

    int rc = MQTTClient_connect(client, &conn_opts);
    if (rc != MQTTCLIENT_SUCCESS) {
        qWarning() << "Failed to connect, return code:" << rc;
        MQTTClient_destroy(&client);
        emit error(E_Conn);
        return false;
    }

    shutdown = false;
    start(QThread::IdlePriority);

    return true;
}

void MqttAgent::subscribe(const QString& topic)
{
    subscribeQueue.push(topic);
}

void MqttAgent::subscribeAll()
{
    while (!subscribeQueue.empty()) {
        const QByteArray topic = subscribeQueue.front().toUtf8();
        qWarning() << "Subscribe topic:" << topic;

        int rc = MQTTClient_subscribe(client, topic.data(), 0);
        if (rc != MQTTCLIENT_SUCCESS) {
            qWarning() << "Failed to subscribe topic:" << topic << ", code:" << rc;
        }

        subscribeQueue.pop();
    }
}

void MqttAgent::run()
{
    while (checkConnected()) {
        subscribeAll();
        publishAll();
        receive();
    }
}

bool MqttAgent::isConnected()
{
    return connected;
}

bool MqttAgent::checkConnected()
{
    if (client && shutdown) {
        qDebug() << "Deinitng MQTT agent";
        MQTTClient_disconnect(client, 10000);
        MQTTClient_destroy(&client);
        client = nullptr;
        std::queue<Message>().swap(msgQueue);
        std::queue<QString>().swap(subscribeQueue);
        shutdown = false;
    }

    bool c = client && MQTTClient_isConnected(client);
    if (connected != c) {
        connected = c;
        qDebug() << "isConnected connected:" << connected;
        emit connectedChanged();
    }

    return connected;
}

void MqttAgent::deInit()
{
    shutdown = true;
}

void MqttAgent::publish(const Message &msg)
{
    msgQueue.push(msg);
}

void MqttAgent::publishAll()
{
    while (!msgQueue.empty()) {
        auto& msg = msgQueue.front();

        MQTTClient_message mqttMsg = MQTTClient_message_initializer;
        mqttMsg.payload = msg.payload.data();
        mqttMsg.payloadlen = msg.payload.length();
        mqttMsg.qos = 0;
        mqttMsg.retained = 0;

        MQTTClient_deliveryToken token;

        int rc;

        rc = MQTTClient_publishMessage(client, msg.topic.data(), &mqttMsg, &token);
        if (rc != MQTTCLIENT_SUCCESS) {
            qWarning() << "Error in MQTT publish";
        } else {
            rc = MQTTClient_waitForCompletion(client, token, 500);
            if (rc != MQTTCLIENT_SUCCESS)
                qWarning() << "Error in MQTT waitForCompletion";
        }

        msgQueue.pop();
    }
}

void MqttAgent::receive()
{
    char* topic;
    int topicLen;
    MQTTClient_message* mqttMsg = nullptr;
    MQTTClient_receive(client, &topic, &topicLen, &mqttMsg, 5);

    if (mqttMsg) {
        qDebug() << "New MQTT message:" << topic << "msg size:" << mqttMsg->payloadlen;
        ++id;
        Message msg;
        msg.id = id;
        msg.payload = QByteArray(static_cast<char*>(mqttMsg->payload), mqttMsg->payloadlen);
        msg.topic = QByteArray(topic, topicLen);
        emit message(msg);
        MQTTClient_freeMessage(&mqttMsg);
    }
}
