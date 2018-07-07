/* Copyright (C) 2018 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <algorithm>

#include <QDebug>
#include <QAudioFormat>
#include <QDataStream>
#include <QEventLoop>
#include <QTimer>
#include <QThread>

#include "audioserver.h"
#include "settings.h"

AudioServer* AudioServer::inst = nullptr;

const char* AudioServer::mqttClientId = "Rubi";
const QString AudioServer::mqttAudioFrameTopic = "hermes/audioServer/%1/audioFrame";
const QString AudioServer::mqttPlayBytesTopic = "hermes/audioServer/%1/playBytes";
const QString AudioServer::mqttPlayFinishedTopic = "hermes/audioServer/%1/playFinished";
const QByteArray AudioServer::mqttSessionEndedTopic = "hermes/dialogueManager/sessionEnded";
const QByteArray AudioServer::mqttSessionStartedTopic = "hermes/dialogueManager/sessionStarted";

const int AudioProcessor::numSamples = 256;
const int AudioProcessor::maxBuffer = 5;
QAudioFormat AudioServer::inFormat = QAudioFormat();

qint64 AudioProcessor::readData(char* data, qint64 maxSize)
{
    qDebug() << "readData";

    Q_UNUSED(data)
    Q_UNUSED(maxSize)
    return 0;
}

qint64 AudioProcessor::writeData(const char* data, qint64 maxSize)
{
    if (active) {
        buffer.append(data, maxSize);
        processBuffer();
    }

    auto mqtt = MqttAgent::instance();
    mqtt->receive();

    return maxSize;
}

void AudioProcessor::setActive(bool active)
{
    if (active != this->active) {
        if (active)
            buffer.clear();
        this->active = active;
    }
}

AudioProcessor::AudioProcessor(QObject* parent) :
    QIODevice(parent)
{
}

void AudioProcessor::init()
{
    qDebug() << "Thread:" << QThread::currentThreadId();
    makeHeader();
    open(QIODevice::WriteOnly);
}

void AudioProcessor::processBuffer()
{
    int size = buffer.length();

    if (size > maxBuffer * payloadSize) {
        qWarning() << "Delay detected, clearing audio buffer";
        buffer.clear();
        return;
    }

    auto mqtt = MqttAgent::instance();

    while (size >= payloadSize) {
        Message msg;
        msg.topic = AudioServer::mqttAudioFrameTopic.arg(
                    Settings::instance()->getSite()).toUtf8();
        msg.payload.append(header);
        msg.payload.append(buffer.data(), payloadSize);

        buffer.remove(0, payloadSize);
        size -= payloadSize;

        mqtt->publish(msg);
    }
}

void AudioProcessor::makeHeader()
{
    /*qDebug() << "Format:";
    qDebug() << " codec:" << AudioServer::inFormat.codec();
    qDebug() << " sampleSize:" << AudioServer::inFormat.sampleSize();
    qDebug() << " sampleRate:" << AudioServer::inFormat.sampleRate();
    qDebug() << " channelCount:" << AudioServer::inFormat.channelCount();
    qDebug() << " numSamples:" << numSamples;*/

    header.clear();

    payloadSize = numSamples *
            AudioServer::inFormat.channelCount() *
            AudioServer::inFormat.sampleSize() / 8;

    qDebug() << "payload size:" << payloadSize;

    // RIFF header
    QDataStream out(&header, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::LittleEndian);
    out.writeRawData("RIFF", 4);
    out << quint32(payloadSize + 36);
    out.writeRawData("WAVE", 4);

    // fmt sub-chunk
    out.writeRawData("fmt ", 4);
    out << quint32(16);
    out << quint16(1);
    out << quint16(AudioServer::inFormat.channelCount());
    out << quint32(AudioServer::inFormat.sampleRate());
    out << quint32(AudioServer::inFormat.sampleRate() *
                   AudioServer::inFormat.channelCount() *
                   AudioServer::inFormat.sampleSize() / 8);
    out << quint16(AudioServer::inFormat.channelCount() *
                   AudioServer::inFormat.sampleSize() / 8);
    out << quint16(AudioServer::inFormat.sampleSize());

    // data sub-chunk
    out.writeRawData("data", 4);
    out << quint32(payloadSize);
}

void AudioServer::processMessage(int id)
{
    qDebug() << "Processing message id:" << id;
    qDebug() << "Thread:" << QThread::currentThreadId();

    // TODO: Handle more types of messages

    if (id == 0) {
        qWarning() << "Message id is invalid";
        messages.erase(id);
        return;
    }

    QString site = Settings::instance()->getSite();
    QByteArray& topic = messages[id].topic;
    QByteArray pbt = AudioServer::mqttPlayBytesTopic.arg(site).toUtf8();

    qDebug() << "topic:" << topic;

    if (topic.startsWith(pbt)) {
        qDebug() << "Play bytes received";
        suspendListening();
        play(id);
        return;
    }

    if (topic == AudioServer::mqttSessionStartedTopic) {
        qDebug() << "Session started received";
        messages.erase(id);
        setInsession(true);
        return;
    }

    if (topic == AudioServer::mqttSessionEndedTopic) {
        qDebug() << "Session ended received";
        messages.erase(id);
        setInsession(false);
        return;
    }

    qWarning() << "Unknown message received";
    messages.erase(id);
    return;
}

AudioServer* AudioServer::instance()
{
    if (AudioServer::inst == nullptr) {
        AudioServer::inst = new AudioServer();
    }

    return AudioServer::inst;
}


AudioServer::AudioServer(QObject* parent) :
    QThread(parent),
    player(parent)
{
}

void AudioServer::run()
{
    qDebug() << "Starting audio server";
    qDebug() << "Thread:" << QThread::currentThreadId();

    // MQTT agent
    auto mqtt = MqttAgent::instance();
    /*if (!mqtt->init()) {
        qWarning() << "Can't connect to MQTT broker";
        //TODO Handle can't connect to MQTT
    }*/
    connect(mqtt, &MqttAgent::message,
            AudioServer::instance(), &AudioServer::processMessage);
    connect(mqtt, &MqttAgent::connectedChanged,
            AudioServer::instance(), &AudioServer::mqttConnectedHandler);
    connect(mqtt, &MqttAgent::error,
            AudioServer::instance(), &AudioServer::mqttError);
    connect(AudioServer::instance(), &AudioServer::initMqtt,
            mqtt, &MqttAgent::init);

    input = std::unique_ptr<QAudioInput>(new QAudioInput(inFormat));

    // Processor
    processor = std::unique_ptr<AudioProcessor>(new AudioProcessor());
    processor->init();

    qDebug() << "Start listening";
    processor->setActive(true);
    input->start(processor.get());
    //emit processorInited();

    // TODO: Loop exit
    exec();

    qDebug() << "Event loop exit, thread:" << QThread::currentThreadId();
}

void AudioServer::sendPlayFinished(int id)
{
    qDebug() << "Sending PlayFinished";

    QString site = Settings::instance()->getSite();

    Message msg;
    msg.topic = AudioServer::mqttPlayFinishedTopic.arg(site).toUtf8();
    QString topic(messages[id].topic);
    QString mqttId = topic.split('/').last();
    msg.payload = QString("{\"id\":\"%1\",\"siteId\":\"%2\",\"sessionId\":null}")
            .arg(mqttId).arg(site).toUtf8();
    qDebug() << "data:" << msg.payload;

    auto mqtt = MqttAgent::instance();
    mqtt->publish(msg);
}

void AudioServer::playFinishedHandler(int id)
{
    qDebug() << "Play finished for id:" << id;
    qDebug() << "Thread:" << QThread::currentThreadId();

    if (id) {
        sendPlayFinished(id);
        messages.erase(id);
    }

    playQueue.pop();
    if (playQueue.empty())
        resumeListening();
    else
        playNext();
}

void AudioServer::mqttConnectedHandler(bool connected)
{
    qDebug() << "MQTT connected:" << connected;
    this->connected = connected;
    emit connectedChanged();
}

void AudioServer::init()
{
    qDebug() << "Initing audio server";
    qDebug() << "Thread:" << QThread::currentThreadId();

    connect(this, &AudioServer::processorInited, this, &AudioServer::startListening);
    connect(&player, &QMediaPlayer::stateChanged, this, &AudioServer::playerStateHandler);

    // input audio format supported by snips
    inFormat.setSampleRate(16000);
    inFormat.setChannelCount(1);
    inFormat.setSampleSize(16);
    inFormat.setCodec("audio/pcm");
    inFormat.setByteOrder(QAudioFormat::LittleEndian);
    inFormat.setSampleType(QAudioFormat::UnSignedInt);

    // tasks
    TaskExecutor::instance(this);

    // audio input

    auto ddev = QAudioDeviceInfo::defaultInputDevice();
    if (!ddev.isFormatSupported(inFormat)) {
        qWarning() << "Default format not supported, trying to use the nearest.";
        inFormat = ddev.nearestFormat(inFormat);
        //TODO Handle default format not supported
    }
    //input = std::unique_ptr<QAudioInput>(new QAudioInput(inFormat));
    /*connect(input.get(), &QAudioInput::stateChanged, [this](QAudio::State state){
        qDebug() << "Input new state:" << state;
        if (state == QAudio::ActiveState) {
            if (!listening) {
                listening = true;
                emit listeningChanged();
            }
        } else {
            if (listening) {
                listening = false;
                emit listeningChanged();
            }
        }
    });*/

    // creating input device in new thread
    start(QThread::IdlePriority);
}

Message& AudioServer::message(int id)
{
    return messages[id];
}

void AudioServer::play(int id)
{
    bool play = playQueue.empty();

    playQueue.push(id);

    if (play)
        playNext();
}

void AudioServer::playerStateHandler(QMediaPlayer::State state)
{
    qDebug() << "Player new state:" << state;
    qDebug() << "Thread:" << QThread::currentThreadId();

    int id = playQueue.front();
    qDebug() << "Id:" << id;

    if (state == QMediaPlayer::PlayingState) {
        if (!playing) {
            playing = true;
            emit playingChanged();
        }
    } else {
        if (playing) {
            playing = false;
            emit playingChanged();

            buffer.close();
            playFinishedHandler(id);
        }
    }
}

void AudioServer::playNext()
{
    qDebug() << "Play next";
    qDebug() << "Thread:" << QThread::currentThreadId();

    if (playQueue.empty()) {
        qWarning() << "Out queue is empty";
        return;
    }

    int id = playQueue.front();
    qDebug() << "Id:" << id;

    if (!messages.count(id)) {
        qWarning() << "Messages doesn't contain id:" << id;
        return;
    }

    QByteArray& payload = messages[id].payload;
    buffer.setBuffer(&payload);
    buffer.open(QIODevice::ReadOnly);

    QMediaContent cnt;
    player.setMedia(cnt, &buffer);
    player.play();
}

void AudioServer::startListening()
{
    qDebug() << "Start listening";
    processor->setActive(true);
    input->start(processor.get());
}

void AudioServer::resumeListening()
{
    qDebug() << "Resume listening";

    processor->setActive(true);

    if (!listening) {
        listening = true;
        emit listeningChanged();
    }
}

void AudioServer::suspendListening()
{
    qDebug() << "Suspend listening";

    processor->setActive(false);

    if (listening) {
        listening = false;
        emit listeningChanged();
    }
}

bool AudioServer::getPlaying()
{
    return playing;
}

bool AudioServer::getListening()
{
    return listening;
}

bool AudioServer::getInsession()
{
    return insession;
}

void AudioServer::setInsession(bool value)
{
    if (insession != value) {
        insession = value;
        emit insessionChanged();
    }
}

bool AudioServer::getConnected()
{
    return connected;
}

void AudioServer::connectToMqtt()
{
    emit initMqtt();
}

void AudioServer::mqttError(MqttAgent::ErrorType err)
{
    switch (err) {
    case MqttAgent::E_NoAddr:
        emit error(E_Mqtt_NoAddr);
        break;
    case MqttAgent::E_Conn:
        emit error(E_Mqtt_Conn);
        break;
    default:
        emit error(E_Unknown);
    }
}
