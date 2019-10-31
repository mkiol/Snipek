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
#include <QMediaResource>
#include <QAudioOutput>
#include <QHash>
#include <QTimer>

#include "mqttagent.h"
#include "message.h"

class AudioProcessor : public QIODevice
{
Q_OBJECT

public:
    AudioProcessor(QObject* parent = nullptr);
    void init();
    void setActive(bool active);
    bool getActive();

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
    void makeRiffHeader();
};

class AudioServer : public QThread
{
Q_OBJECT
    Q_PROPERTY (bool listening READ getListening NOTIFY listeningChanged)
    Q_PROPERTY (bool playing READ getPlaying NOTIFY playingChanged)
    Q_PROPERTY (bool insession READ getInsession NOTIFY insessionChanged)
    Q_PROPERTY (bool connected READ isConnected NOTIFY connectedChanged)

    struct MessageDetails;
    struct AudioDetails;

public:
    enum ErrorType {
        E_Unknown = 0
    };
    Q_ENUM(ErrorType)

    static const QString mqttAudioFrameTopic;
    static const QString mqttPlayBytesTopic;
    static const QString mqttPlayFinishedTopic;
    static const QString mqttStreamFinishedTopic;
    static const QString mqttPlayBytesStreamingTopic;
    static const QString mqttSessionEndedTopic;
    static const QString mqttSessionStartedTopic;
    static const QString mqttSessionStartTopic;
    static const QString mqttFeedbackOnTopic;
    static const QString mqttFeedbackOffTopic;
    static const QString mqttLoadedTopic;

    static QAudioFormat inFormat;
    static QAudioFormat outFormat;
    static AudioServer* instance();

    void init();
    void deInit();
    void setFeedback();
    Q_INVOKABLE QStringList getInAudioDevices();
    Q_INVOKABLE QStringList getOutAudioDevices();

signals:
    void processorInited();
    void error(ErrorType error);
    void audioWriteNeeded();

    // props
    void listeningChanged();
    void playingChanged();
    void insessionChanged();
    void connectedChanged();

public slots:
    void startListening();
    void suspendListening();
    void resumeListening();
    void processMessage(const  Message& msg);
    void startSession();

    // props
    bool getListening();
    bool getPlaying();
    bool getInsession();
    bool isConnected();

protected:
    void run();

private:
    enum MessageType {
        MSG_UNKNOWN = 0,
        MSG_PLAYBYTES,
        MSG_STREAMBYTES,
        MSG_SESSIONSTARTED,
        MSG_SESSIONENDED
    };
    struct AudioDetails {
        QAudioFormat format;
        int start = 0;
        int size = 0;
        bool invalid = true;
    };
    struct MessageDetails {
        MessageType type;
        QString siteId;
        QString reqId;
        int chunk = 0;
        bool lastChunk = false;
        AudioDetails audioDetails;
        MessageDetails();
        MessageDetails(MessageType type,
                       QString siteId = QString(),
                       QString reqId = QString(),
                       int chunk = 0, bool lastChunk = false);
    };

    static AudioServer* inst;
    std::unique_ptr<AudioProcessor> processor;
    std::unique_ptr<QAudioInput> input;
    std::unique_ptr<QAudioOutput> output;
    std::queue<QString> playQueue; // reqId
    QHash<QString, QByteArray> reqIdToDataMap;
    QHash<QString, MessageDetails> reqIdToDetailsMap;
    QIODevice* buffer;
    QString currReqId;
    QStringList inAudioNames;
    QStringList outAudioNames;
    QTimer writeTimer;
    QTimer clearTimer;

    bool playing = false;
    bool insession = false;
    bool connected = false;
    bool shouldListen = false;
    bool waitingForAudio = false;

    AudioServer(QObject* parent = nullptr);
    static MessageDetails makeDetails(const Message &msg);
    void playNext();
    void sendPlayFinished(const MessageDetails &md);
    void sendStreamFinished(const MessageDetails &md);
    void setInsession(bool value);
    bool checkSiteId(const QByteArray& data);
    void subscribe();
    void loaded();
    AudioDetails audioDetailsFromRiff(const QByteArray &data);
    void play(const MessageDetails &md, const Message &msg);
    void updateMd(const MessageDetails& newMd, MessageDetails& md);
    bool isPlayingFinished();
    void setPlaying(bool playing);
    void updateListening();

private slots:
    void playFinishedHandler();
    void handleAudioOutputStateChanged(QAudio::State newState);
    void handleMqttConnected();
    void writeAudio();
    void clearAudio();
};

#endif // AUDIOSERVER_H
