/* Copyright (C) 2018 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QThread>
#include <QByteArray>

#include "mqttagent.h"
#include "audioserver.h"
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
    QObject(parent)
{
    //init();
}

bool MqttAgent::init()
{
    qDebug() << "Init thread:" << QThread::currentThreadId();

    deInit();

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

    MQTTClient_create(&client, url.data(), AudioServer::mqttClientId,
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

    QByteArray topic;

    // play bytes subscription
    topic = AudioServer::mqttPlayBytesTopic.arg(settings->getSite()).toUtf8() + "/#";
    rc = MQTTClient_subscribe(client, topic.data(), 0);
    if (rc != MQTTCLIENT_SUCCESS) {
        qWarning() << "Failed to subscribe PlayBytes, return code:" << rc;
        MQTTClient_destroy(&client);
        emit error(E_Conn);
        return false;
    }

    // session started subscription
    topic = AudioServer::mqttSessionStartedTopic;
    rc = MQTTClient_subscribe(client, topic.data(), 0);
    if (rc != MQTTCLIENT_SUCCESS) {
        qWarning() << "Failed to subscribe SessionStarted, return code:" << rc;
        MQTTClient_destroy(&client);
        emit error(E_Conn);
        return false;
    }

    // session ended subscription
    topic = AudioServer::mqttSessionEndedTopic;
    rc = MQTTClient_subscribe(client, topic.data(), 0);
    if (rc != MQTTCLIENT_SUCCESS) {
        qWarning() << "Failed to subscribe SessionEnded, return code:" << rc;
        MQTTClient_destroy(&client);
        emit error(E_Conn);
        return false;
    }

    return true;
}

bool MqttAgent::isConnected()
{
    bool c = client && MQTTClient_isConnected(client);
    if (connected != c) {
        connected = c;
        emit connectedChanged(connected);
    }

    return connected;
}

void MqttAgent::deInit()
{
    if (client) {
        MQTTClient_disconnect(client, 10000);
        MQTTClient_destroy(&client);
    }
}

void MqttAgent::publish(Message& msg)
{
    //qDebug() << "Publish thread:" << QThread::currentThreadId();

    if (!isConnected()) {
        //qWarning() << "MQTT not connected";
        // TODO: Handle MQTT disconnection
        /*if (!init())
            return;*/
        return;
    }

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
        return;
    }

    rc = MQTTClient_waitForCompletion(client, token, 500);
    if (rc != MQTTCLIENT_SUCCESS) {
        qWarning() << "Error in MQTT waitForCompletion";
        return;
    }
}

void MqttAgent::receive()
{
    //qDebug() << "Receive thread:" << QThread::currentThreadId();

    if (!isConnected()) {
        //qWarning() << "MQTT not connected";
        // TODO: Handle MQTT disconnection
        return;
    }

    char* topic;
    int topicLen;
    MQTTClient_message* mqttMsg = nullptr;
    MQTTClient_receive(client, &topic, &topicLen, &mqttMsg, 5);

    if (mqttMsg) {
        qDebug() << "New MQTT message:" << topic << "msg size:" << mqttMsg->payloadlen;
        qDebug() << "msg id:" << mqttMsg->msgid;

        ++id;

        Message& msg = AudioServer::instance()->message(id);
        msg.id = id;
        msg.payload = QByteArray(static_cast<char*>(mqttMsg->payload), mqttMsg->payloadlen);
        msg.topic = QByteArray(topic, topicLen);

        emit message(id);

        MQTTClient_freeMessage(&mqttMsg);
    }
}
