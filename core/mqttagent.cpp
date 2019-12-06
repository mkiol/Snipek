/* Copyright (C) 2018-2019 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QThread>
#include <QCoreApplication>

#include "mqttagent.h"
#include "settings.h"
#include "snipslocalagent.h"

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
    connect(settings, &Settings::snipsLocalChanged, [this, settings]{
        if (!settings->getSnipsLocal())
            initWithReconnect();
    });

    auto snips = SnipsLocalAgent::instance();
    connect(snips, &SnipsLocalAgent::snipsChanged, [this, snips]{
        if (snips->getSnipsStatus() == SnipsLocalAgent::SnipsStarted)
            initWithReconnect();
    });

    reconTimer.setSingleShot(true);
    reconTimer.setTimerType(Qt::VeryCoarseTimer);
    connect(&reconTimer, &QTimer::timeout, this, &MqttAgent::reconnect);
    connect(this, &MqttAgent::doReconnect, this, &MqttAgent::reconnect);
}

void MqttAgent::initWithReconnect()
{
    qDebug() << "MQTT init with reconnect";
    emit doReconnect();
}

QByteArray MqttAgent::makeUrl()
{
    auto s = Settings::instance();
    bool local = s->getSnipsLocal();

    auto addr = local ? "127.1.0.0" : s->getMqttAddress();
    int port = local ? 1883 : s->getMqttPort();

    return addr.isEmpty() || port < 1 ?
                QByteArray() :
                QString("tcp://%1:%2").arg(addr).arg(port).toLatin1();
}

bool MqttAgent::init()
{
    qDebug() << "MQTT init"
             << "connected:" << connected
             << "shutdown:" << shutdowning
             << "tid:" << QThread::currentThreadId();

    if (shutdowning) {
        qDebug() << "Shutdown already requested, so ignoring init request";
        return true;
    }

    auto newUrl = makeUrl();

    qDebug() << "MQTT new URL:" << newUrl << "current URL:" << url;
    if (connected && url == newUrl) {
        qDebug() << "No need to init MQTT because it is already inited with the same URL";
        return true;
    }

    url = newUrl;
    deInit();

    if (url.isEmpty()) {
        qWarning() << "No MQTT addres or port defined";
        emit error(E_NoAddr);
        return false;
    }

    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

    QByteArray id = Settings::instance()->getMqttId().toUtf8();
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
        if (reconCounter == 0)
            emit error(E_Conn);
        return false;
    }

    start(QThread::HighPriority);

    reconCounter = 0;
    return true;
}

void MqttAgent::subscribe(const QString& topic)
{
    if (!shutdowning)
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

void MqttAgent::unsubscribe(const QString& topic)
{
    if (!shutdowning)
        unsubscribeQueue.push(topic);
}

void MqttAgent::unsubscribeAll()
{
    while (!unsubscribeQueue.empty()) {
        const QByteArray topic = unsubscribeQueue.front().toUtf8();
        qWarning() << "Unsubscribe topic:" << topic;

        int rc = MQTTClient_unsubscribe(client, topic.data());
        if (rc != MQTTCLIENT_SUCCESS) {
            qWarning() << "Failed to unsubscribe topic:" << topic << ", code:" << rc;
        }

        unsubscribeQueue.pop();
    }
}

void MqttAgent::run()
{
    while (checkConnected()) {
        unsubscribeAll();
        subscribeAll();
        publishAll();
        receive();
    }
}

void MqttAgent::reconnect()
{
    qDebug() << "Reconnecting attempt:" << reconCounter;

    if (reconCounter == 0) {
        reconCounter++;
        reconTimer.setInterval(100);
        reconTimer.start();
    } else if (!init()) {
        if (reconCounter < 5) {
            reconCounter++;
            reconTimer.setInterval(1000);
            reconTimer.start();
        } else {
            qDebug() << "Reconnect attempts limit reached";
            reconCounter = 0;
        }
    }
}

bool MqttAgent::isConnected()
{
    return connected;
}

bool MqttAgent::checkConnected()
{
    if (client && shutdowning && unsubscribeQueue.empty()) {
        qDebug() << "Shutdown MQTT agent";
        MQTTClient_disconnect(client, 10000);
        MQTTClient_destroy(&client);
        client = nullptr;
        std::queue<Message>().swap(msgQueue);
        std::queue<QString>().swap(subscribeQueue);
        std::queue<QString>().swap(unsubscribeQueue);
        shutdowning = false;
        connected = false;
        emit connectedChanged();
        return true;
    }

    bool c = client && MQTTClient_isConnected(client);
    if (connected != c) {
        connected = c;
        emit connectedChanged();
        if (!connected) {
            reconCounter = 0;
            emit doReconnect();
        }
    }

    return connected;
}

void MqttAgent::deInit()
{
    qDebug() << "MQTT shutdown start";
    shutdowning = true;
    wait();
    shutdowning = false;
    qDebug() << "MQTT shutdown end";
}

void MqttAgent::publish(const Message &msg)
{
    if (!shutdowning) {
        if (!msg.topic.endsWith("audioFrame"))
            qDebug() << "Adding to publish queue:" << msg.topic << msg.payload;
        msgQueue.push(msg);
    }
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

        /*if (!msg.topic.endsWith("audioFrame"))
            qDebug() << "Published:" << msg.topic;*/

        msgQueue.pop();
    }
}

void MqttAgent::receive()
{
    if (shutdowning)
        return;

    char* topic;
    int topicLen;
    MQTTClient_message* mqttMsg = nullptr;

    while (true) {
        MQTTClient_receive(client, &topic, &topicLen, &mqttMsg, 5);
        if (mqttMsg) {
            qDebug() << "New MQTT message:" << topic;

            ++id;

            Message msg;
            msg.id = id;
            msg.payload = QByteArray(static_cast<char*>(mqttMsg->payload), mqttMsg->payloadlen);
            msg.topic = QByteArray(topic, topicLen);

            emit message(msg);
            QList<QByteArray> st = msg.topic.split('/');
            if (st.length() > 1) {
                const auto& type = st.at(1);
                if (type == "audioServer")
                    emit audioServerMessage(msg);
                else if (type == "dialogueManager")
                    emit dialogueManagerMessage(msg);
                else if (type == "tts")
                    emit ttsMessage(msg);
                else if (type == "asr")
                    emit asrMessage(msg);
                else if (type == "intent")
                    emit intentMessage(msg);
            }

            MQTTClient_freeMessage(&mqttMsg);
            MQTTClient_free(topic);
        } else {
            break;
        }
    }
}
