/* Copyright (C) 2018 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef AUDIOSERVER_H
#define AUDIOSERVER_H

#include <memory>
#include <queue>
#include <unordered_map>

#include <QObject>
#include <QAudio>
#include <QAudioInput>
#include <QAudioFormat>
#include <QIODevice>
#include <QByteArray>
#include <QThread>
#include <QBuffer>
#include <QStringList>
#include <QMediaPlayer>

#include "taskexecutor.h"
#include "mqttagent.h"
#include "message.h"

class AudioProcessor : public QIODevice
{
Q_OBJECT

public:
    AudioProcessor(QObject* parent = nullptr);
    void init();
    void setActive(bool active);

protected:
    virtual qint64 readData(char* data, qint64 maxSize);
    virtual qint64 writeData(const char* data, qint64 maxSize);

private slots:
    void processBuffer();

private:
    static const int maxBuffer;
    static const int numSamples;

    bool active = false;
    QByteArray buffer;
    QByteArray header;
    qint32 inPayloadSize;
    qint32 outPayloadSize;

    void makeHeader();
};

class AudioServer : public QThread
{
Q_OBJECT
    Q_PROPERTY (bool listening READ getListening NOTIFY listeningChanged)
    Q_PROPERTY (bool playing READ getPlaying NOTIFY playingChanged)
    Q_PROPERTY (bool insession READ getInsession NOTIFY insessionChanged)
    Q_PROPERTY (bool connected READ isConnected NOTIFY connectedChanged)

public:
    enum ErrorType {
        E_Unknown = 0,
        E_Mqtt_NoAddr,
        E_Mqtt_Conn
    };
    Q_ENUM(ErrorType)

    static const QString mqttAudioFrameTopic;
    static const QString mqttPlayBytesTopic;
    static const QString mqttPlayFinishedTopic;
    static const QByteArray mqttSessionEndedTopic;
    static const QByteArray mqttSessionStartedTopic;
    static const QByteArray mqttFeedbackOnTopic;
    static const QByteArray mqttFeedbackOffTopic;

    static QAudioFormat inFormat;
    static QAudioFormat outFormat;
    static AudioServer* instance();

    void init();
    void deInit();
    void setFeedback(bool enabled);
    Q_INVOKABLE QStringList getInAudioDevices();
    Q_INVOKABLE QStringList getOutAudioDevices();
    Message& message(int id);

signals:
    void processorInited();
    void error(ErrorType error);

    // props
    void listeningChanged();
    void playingChanged();
    void insessionChanged();
    void connectedChanged();

public slots:
    void startListening();
    void suspendListening();
    void resumeListening();
    void play(int id);
    void processMessage(int id);

    // props
    bool getListening();
    bool getPlaying();
    bool getInsession();
    bool isConnected();

private slots:
    void playFinishedHandler(int id);
    void playerStateHandler(QMediaPlayer::State state);
    void mqttConnectedHandler();
    void mqttError(MqttAgent::ErrorType err);

protected:
    void run();

private:
    static AudioServer* inst;
    std::unique_ptr<AudioProcessor> processor;
    std::unique_ptr<QAudioInput> input;
    std::unordered_map<int, Message> messages;
    std::queue<int> playQueue;
    QBuffer buffer;
    QMediaPlayer player;
    QStringList inAudioNames;
    QStringList outAudioNames;

    bool playing = false;
    bool listening = false;
    bool insession = false;
    bool connected = false;

    AudioServer(QObject* parent = nullptr);
    void playNext();
    void sendPlayFinished(int id);
    void setInsession(bool value);
    bool checkSiteId(const QByteArray& data);
};

#endif // AUDIOSERVER_H
