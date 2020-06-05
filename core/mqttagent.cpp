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
            this, &MqttAgent::shutdown);

    auto settings = Settings::instance();
    connect(settings, &Settings::mqttChanged, this, &MqttAgent::shutdown);
    connect(settings, &Settings::siteChanged, this, &MqttAgent::shutdown);
    connect(settings, &Settings::snipsLocalChanged, [this, settings]{
        if (!settings->getSnipsLocal())
            init();
    });

    auto snips = SnipsLocalAgent::instance();
    connect(snips, &SnipsLocalAgent::snipsChanged, [this, snips]{
        if (snips->getSnipsStatus() == SnipsLocalAgent::SnipsStarted)
            init();
    });

    reconTimer.setSingleShot(true);
    reconTimer.setTimerType(Qt::VeryCoarseTimer);
    connect(&reconTimer, &QTimer::timeout, this, &MqttAgent::reconnect);
    connect(this, &MqttAgent::doReconnectLater, this, &MqttAgent::reconnectLater);
}

void MqttAgent::init()
{
    qDebug() << "MQTT init, state:" << state;

    /*if (state == Shutdowning || state == Connecting) {
        qDebug() << "Ignoring init request";
        return;
    }*/

    Action action(Action::Connect, makeUrl());

    qDebug() << "MQTT new URL:" << action.url;
    qDebug() << "MQTT current URL:" << url;

    /*if (state == Connected && url == action.url) {
        qDebug() << "No need to init MQTT because it is already inited with the same URL";
        return;
    }*/

    actionQueue.push(action);

    if (!isRunning())
        start(QThread::HighPriority);
}

QByteArray MqttAgent::makeUrl()
{
    auto s = Settings::instance();
    bool local = s->getSnipsLocal();

    auto addr = local ? "127.0.0.1" : s->getMqttAddress();
    int port = local ? 1883 : s->getMqttPort();

    return addr.isEmpty() || port < 1 ?
                QByteArray() :
                QString("tcp://%1:%2").arg(addr).arg(port).toLatin1();
}

void MqttAgent::subscribe(const QString& topic)
{
    if (state == MqttConnected)
        subscribeQueue.push(topic);
}

void MqttAgent::subscribeAll()
{
    if (state == MqttConnected) {
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
}

void MqttAgent::unsubscribe(const QString& topic)
{
    if (state == MqttConnected)
        unsubscribeQueue.push(topic);
}

void MqttAgent::unsubscribeAll()
{
    if (state == MqttConnected || state == MqttShutdowning) {
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
}

void MqttAgent::run()
{
    qDebug() << "Staring MQTT thread";
    while (state != MqttDisconnected || !actionQueue.empty()) {
        checkActions();
        unsubscribeAll();
        subscribeAll();
        publishAll();
        receive();
    }
    qDebug() << "Finishing MQTT thread";
}

void MqttAgent::reconnect()
{
    qDebug() << "Reconnecting attempt:" << reconCounter;

    if (state != MqttDisconnected) {
        qDebug() << "Reconnect not needed";
        return;
    }

    init();
}

void MqttAgent::reconnectLater()
{
    reconTimer.setInterval(1000);
    reconTimer.start();
}

MqttAgent::MqttState MqttAgent::getState()
{
    return state;
}

void MqttAgent::checkActions()
{
    if (!actionQueue.empty()) {
        auto& action = actionQueue.front();
        qDebug() << "New MQTT action:" << action.type;

        if (action.type == Action::Shutdown) {
            if (!unsubscribeQueue.empty()) {
                qDebug() << "Shutdown requested but need to send unsubscribe";
                if (state != MqttShutdowning) {
                    state = MqttShutdowning;
                    emit stateChanged();
                }
                return;
            }
            if (!client) {
                qWarning() << "Shutdown requested but MQTT client is null";
                if (state != MqttDisconnected) {
                    state = MqttDisconnected;
                    emit stateChanged();
                }
            } else {
                qDebug() << "Disconnecting MQTT agent";

                MQTTClient_disconnect(client, 10000);
                MQTTClient_destroy(&client);
                client = nullptr;
                std::queue<Message>().swap(msgQueue);
                std::queue<QString>().swap(subscribeQueue);
                std::queue<QString>().swap(unsubscribeQueue);
                if (state != MqttDisconnected) {
                    state = MqttDisconnected;
                    emit stateChanged();
                }
            }
        } else if (action.type == Action::Connect) {
            if (state != MqttConnecting) {
                state = MqttConnecting;
                emit stateChanged();
            }

            if (client) {
                qDebug() << "MQTT is connected, so disconnecting before new connect";
                MQTTClient_disconnect(client, 10000);
                MQTTClient_destroy(&client);
                client = nullptr;
                std::queue<Message>().swap(msgQueue);
                std::queue<QString>().swap(subscribeQueue);
                std::queue<QString>().swap(unsubscribeQueue);
                qDebug() << "MQTT disconnected";
            }

            qDebug() << "Connecting MQTT agent for URL:" << action.url;

            MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
            QByteArray id = Settings::instance()->getMqttId().toUtf8();
            qDebug() << "MQTT ID:" << id;

            MQTTClient_create(&client, action.url.data(), id.data(),
                MQTTCLIENT_PERSISTENCE_NONE, NULL);
            conn_opts.keepAliveInterval = 10;
            conn_opts.cleansession = 1;
            conn_opts.connectTimeout = 1;
            conn_opts.retryInterval = 5;

            int rc = MQTTClient_connect(client, &conn_opts);
            if (rc != MQTTCLIENT_SUCCESS) {
                qWarning() << "Failed to connect MQTT, return code:" << rc;
                MQTTClient_destroy(&client);
                client = nullptr;
                if (state != MqttDisconnected) {
                    state = MqttDisconnected;
                    emit stateChanged();
                }
                if (reconCounter < 3){
                    reconCounter++;
                    emit doReconnectLater();
                } else {
                    qDebug() << "Reconnect attempts limit reached";
                    emit error(E_Conn);
                    reconCounter = 0;
                }
            } else {
                if (state != MqttConnected) {
                    state = MqttConnected;
                    emit stateChanged();
                }
            }
            url = action.url;
        }

        actionQueue.pop();
        return;
    }

    bool connected = client && MQTTClient_isConnected(client);
    if (state == MqttConnected && !connected) {
        qWarning() << "MQTT is disconnected";
        state = MqttDisconnected;
        emit stateChanged();
        // reconnecting
        reconCounter = 0;
        emit doReconnectLater();
    }
}

void MqttAgent::shutdown()
{
    if (state != MqttDisconnected) {
        actionQueue.push(Action(Action::Shutdown));
    }
}

void MqttAgent::publish(const Message &msg)
{
    if (state == MqttConnected) {
        if (!msg.topic.endsWith("audioFrame"))
            qDebug() << "Adding to publish queue:" << msg.topic << msg.payload;
        msgQueue.push(msg);
    }
}

void MqttAgent::publishAll()
{
    if (state == MqttConnected) {
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
}

void MqttAgent::receive()
{
    if (state == MqttConnected) {
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
}
