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
#include <QMediaService>
#include <QAudioOutputSelectorControl>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QCoreApplication>

#include "audioserver.h"
#include "settings.h"

AudioServer* AudioServer::inst = nullptr;

const QString AudioServer::mqttAudioFrameTopic = "hermes/audioServer/%1/audioFrame";
const QString AudioServer::mqttPlayBytesTopic = "hermes/audioServer/%1/playBytes";
const QString AudioServer::mqttPlayFinishedTopic = "hermes/audioServer/%1/playFinished";
const QString AudioServer::mqttSessionEndedTopic = "hermes/dialogueManager/sessionEnded";
const QString AudioServer::mqttSessionStartedTopic = "hermes/dialogueManager/sessionStarted";
const QString AudioServer::mqttFeedbackOnTopic = "hermes/feedback/sound/toggleOn";
const QString AudioServer::mqttFeedbackOffTopic = "hermes/feedback/sound/toggleOff";

const int AudioProcessor::numSamples = 256;
const int AudioProcessor::maxBuffer = 5;
QAudioFormat AudioServer::inFormat = QAudioFormat();
QAudioFormat AudioServer::outFormat = QAudioFormat();

qint64 AudioProcessor::readData(char* data, qint64 maxSize)
{
    Q_UNUSED(data)
    Q_UNUSED(maxSize)
    return 0;
}

qint64 AudioProcessor::writeData(const char* data, qint64 maxSize)
{
    //qDebug() << "writeData:" << active;

    if (active) {
        buffer.append(data, static_cast<int>(maxSize));
        processBuffer();
    }

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
    makeHeader();
    open(QIODevice::WriteOnly);
}

void AudioProcessor::processBuffer()
{
    int size = buffer.length();

    if (size > maxBuffer * inPayloadSize) {
        qWarning() << "Delay detected, clearing audio buffer";
        buffer.clear();
        return;
    }

    auto mqtt = MqttAgent::instance();

    while (size >= inPayloadSize) {
        Message msg;
        msg.topic = AudioServer::mqttAudioFrameTopic.arg(
                    Settings::instance()->getSite()).toUtf8();
        msg.payload.append(header);
        msg.payload.append(buffer.data(), inPayloadSize);

        buffer.remove(0, inPayloadSize);
        size -= inPayloadSize;

        mqtt->publish(msg);
    }
}

void AudioProcessor::makeHeader()
{
    /*qDebug() << "Required output format:";
    qDebug() << " codec:" << AudioServer::outFormat.codec();
    qDebug() << " sampleSize:" << AudioServer::outFormat.sampleSize();
    qDebug() << " sampleRate:" << AudioServer::outFormat.sampleRate();
    qDebug() << " channelCount:" << AudioServer::outFormat.channelCount();
    qDebug() << " numSamples:" << numSamples;
    qDebug() << "Input format:";
    qDebug() << " codec:" << AudioServer::inFormat.codec();
    qDebug() << " sampleSize:" << AudioServer::inFormat.sampleSize();
    qDebug() << " sampleRate:" << AudioServer::inFormat.sampleRate();
    qDebug() << " channelCount:" << AudioServer::inFormat.channelCount();*/

    header.clear();

    outPayloadSize = numSamples *
            AudioServer::outFormat.channelCount() *
            AudioServer::outFormat.sampleSize() / 8;
    inPayloadSize = numSamples *
            AudioServer::inFormat.channelCount() *
            AudioServer::inFormat.sampleSize() / 8;

    //qDebug() << "in payload size:" << inPayloadSize;
    //qDebug() << "out payload size:" << outPayloadSize;

    // RIFF header
    QDataStream out(&header, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::LittleEndian);
    out.writeRawData("RIFF", 4);
    out << quint32(inPayloadSize + 36);
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
    out << quint32(inPayloadSize);
}

void AudioServer::processMessage(const Message &msg)
{
    qDebug() << "Processing message id:" << msg.id;

    QString site = Settings::instance()->getSite();
    QByteArray pbt = AudioServer::mqttPlayBytesTopic.arg(site).toUtf8();

    qDebug() << "topic:" << msg.topic;

    if (msg.topic.startsWith(pbt)) {
        qDebug() << "Play bytes received";
        suspendListening();
        play(msg);
        return;
    }

    if (msg.topic == AudioServer::mqttSessionStartedTopic.toUtf8()) {
        qDebug() << "Session started received";
        qDebug() << "payload:" << msg.payload;
        if (checkSiteId(msg.payload))
            setInsession(true);
        return;
    }

    if (msg.topic == AudioServer::mqttSessionEndedTopic.toUtf8()) {
        qDebug() << "Session ended received";
        qDebug() << "payload:" << msg.payload;
        if (checkSiteId(msg.payload))
            setInsession(false);
        return;
    }

    qWarning() << "Unknown message received";
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

    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, [this](){
        processor->setActive(false);
        QThread::quit();
    });

    input = std::unique_ptr<QAudioInput>(new QAudioInput(inFormat));

    // Processor
    processor = std::unique_ptr<AudioProcessor>(new AudioProcessor());
    processor->init();

    qDebug() << "Start listening";
    input->start(processor.get());

    mqttConnectedHandler();

    QThread::exec();
    qDebug() << "Event loop exit, thread:" << QThread::currentThreadId();
}

void AudioServer::sendPlayFinished(const Message& msg)
{
    qDebug() << "Sending PlayFinished";

    QString site = Settings::instance()->getSite();

    Message respMsg;
    respMsg.topic = AudioServer::mqttPlayFinishedTopic.arg(site).toUtf8();
    QString topic(msg.topic);
    QString mqttId = topic.split('/').last();
    respMsg.payload = QString("{\"id\":\"%1\",\"siteId\":\"%2\",\"sessionId\":null}")
            .arg(mqttId).arg(site).toUtf8();
    //qDebug() << "data:" << respMsg.payload;

    auto mqtt = MqttAgent::instance();
    mqtt->publish(respMsg);
}

void AudioServer::playFinishedHandler(const Message& msg)
{
    qDebug() << "Play finished for id:" << msg.id;

    sendPlayFinished(msg);

    playQueue.pop();
    if (playQueue.empty())
        resumeListening();
    else
        playNext();
}

void AudioServer::subscribe()
{
    auto mqtt = MqttAgent::instance();
    // play bytes
    mqtt->subscribe(AudioServer::mqttPlayBytesTopic.arg(
                        Settings::instance()->getSite()).toUtf8() + "/#");
    // session started
    mqtt->subscribe(AudioServer::mqttSessionStartedTopic);
    // session ended
    mqtt->subscribe(AudioServer::mqttSessionEndedTopic);
}

void AudioServer::mqttConnectedHandler()
{
    bool mqttConnected = MqttAgent::instance()->isConnected();
    qDebug() << "MQTT connected changed:" << mqttConnected;

    if (mqttConnected) {
        subscribe();
        setFeedback();
        resumeListening();
    } else {
        setInsession(false);
        suspendListening();
    }

    this->connected = mqttConnected;
    emit connectedChanged();
}

void AudioServer::deInit()
{
    qDebug() << "De-initing audio server";
    QThread::quit();
    suspendListening();
    disconnect(this);
    player.stop();
}

void AudioServer::init()
{
    qDebug() << "Initing audio server";

    connect(this, &AudioServer::processorInited, this, &AudioServer::startListening);
    connect(&player, &QMediaPlayer::stateChanged, this, &AudioServer::playerStateHandler);

    // audio format supported by snips
    outFormat.setSampleRate(16000);
    outFormat.setChannelCount(1);
    outFormat.setSampleSize(16);
    outFormat.setCodec("audio/pcm");
    outFormat.setByteOrder(QAudioFormat::LittleEndian);
    outFormat.setSampleType(QAudioFormat::SignedInt);
    inFormat = outFormat;

    // audio input
    qDebug() << "Required format:";
    qDebug() << " codec:" << AudioServer::inFormat.codec();
    qDebug() << " sampleSize:" << AudioServer::inFormat.sampleSize();
    qDebug() << " sampleRate:" << AudioServer::inFormat.sampleRate();
    qDebug() << " channelCount:" << AudioServer::inFormat.channelCount();

    auto inDev = QAudioDeviceInfo::defaultInputDevice();

    // Set input device
    /*QString inDevName = settings->getInAudio();
    qDebug() << "inDevName:" << inDevName;

    if (!inDevName.isEmpty()) {
        auto inDevs = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);

        for (auto& dev : inDevs) {
            if (dev.deviceName() == inDevName) {
                qDebug() << "inDev found:" << dev.deviceName();
                inDev = dev;
                break;
            }
        }
    }*/

    if (!inDev.isFormatSupported(outFormat)) {
        qWarning() << "Default format not supported, trying to use the nearest.";
        inFormat = inDev.nearestFormat(outFormat);
        //TODO Handle default format not supported

        qDebug() << "Nerest format:";
        qDebug() << " codec:" << AudioServer::inFormat.codec();
        qDebug() << " sampleSize:" << AudioServer::inFormat.sampleSize();
        qDebug() << " sampleRate:" << AudioServer::inFormat.sampleRate();
        qDebug() << " channelCount:" << AudioServer::inFormat.channelCount();
    }

    auto mqtt = MqttAgent::instance();
    this->connected = mqtt->isConnected();
    connect(mqtt, &MqttAgent::audioServerMessage,
            this, &AudioServer::processMessage);
    connect(mqtt, &MqttAgent::dialogueManagerMessage,
            this, &AudioServer::processMessage);
    connect(mqtt, &MqttAgent::connectedChanged,
            this, &AudioServer::mqttConnectedHandler);
    connect(Settings::instance(), &Settings::audioFeedbackChanged, [this] {
        if (connected)
            setFeedback();
    });

    // creating input device in new thread
    start(QThread::IdlePriority);
}

void AudioServer::setFeedback()
{
    bool enabled = Settings::instance()->getAudioFeedback();
    qDebug() << "Setting feedback:" << enabled;

    QString site = Settings::instance()->getSite();

    Message msg;
    msg.topic = enabled ? AudioServer::mqttFeedbackOnTopic.toUtf8() :
                          AudioServer::mqttFeedbackOffTopic.toUtf8();
    msg.payload = QString("{\"siteId\":\"%1\"}").arg(site).toUtf8();
    qDebug() << "data:" << msg.payload;

    auto mqtt = MqttAgent::instance();
    mqtt->publish(msg);
}

void AudioServer::play(const Message& msg)
{
    bool play = playQueue.empty();
    playQueue.push(msg);
    if (play)
        playNext();
}

void AudioServer::playerStateHandler(QMediaPlayer::State state)
{
    qDebug() << "Player new state:" << state;

    auto& msg = playQueue.front();

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
            playFinishedHandler(msg);
        }
    }
}

void AudioServer::playNext()
{
    qDebug() << "Play next";

    if (playQueue.empty()) {
        qWarning() << "Out queue is empty";
        return;
    }

    auto& msg = playQueue.front();

    QByteArray& payload = msg.payload;
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

bool AudioServer::isConnected()
{
    return connected;
}

QStringList AudioServer::getInAudioDevices()
{
    if (inAudioNames.isEmpty()) {
        QStringList& list = inAudioNames;

        auto devs = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
        //qDebug() << "In devices:";
        for (auto& dev : devs) {
            /*qDebug() << "Device:";
            qDebug() << " name:" << dev.deviceName();
            qDebug() << " null:" << dev.isNull();
            qDebug() << " supportedCodecs:" << dev.supportedCodecs();
            qDebug() << " sampleRates:" << dev.supportedSampleRates();
            qDebug() << " sampleSizes:" << dev.supportedSampleSizes();
            qDebug() << " channelCount:" << dev.supportedChannelCounts();*/
            if (!dev.supportedCodecs().isEmpty() &&
                    dev.isFormatSupported(outFormat))
                list.push_back(dev.deviceName());
        }
    }

    return inAudioNames;
}

QStringList AudioServer::getOutAudioDevices()
{
    if (outAudioNames.isEmpty()) {
        QStringList& list = outAudioNames;

        auto devs = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
        //qDebug() << "Out devices:";
        for (auto& dev : devs) {
            /*qDebug() << "Device:";
            qDebug() << " name:" << dev.deviceName();
            qDebug() << " null:" << dev.isNull();
            qDebug() << " supportedCodecs:" << dev.supportedCodecs();
            qDebug() << " sampleRates:" << dev.supportedSampleRates();
            qDebug() << " sampleSizes:" << dev.supportedSampleSizes();
            qDebug() << " channelCount:" << dev.supportedChannelCounts();*/
            if (!dev.supportedCodecs().isEmpty())
                list.push_back(dev.deviceName());
        }
    }

    return outAudioNames;
}

bool AudioServer::checkSiteId(const QByteArray& data)
{
    QJsonParseError err;
    auto json = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError) {
        qWarning() << "Error parsing json payload:" << err.errorString();
        return false;
    }

    if (!json.isObject()) {
        qWarning() << "Json is not a object";
        return false;
    }

    auto siteId = json.object().value("siteId");
    qDebug() << "Site ID:" << siteId.toString();

    if (siteId.isUndefined()) {
        qWarning() << "No siteId element";
        return false;
    }

    auto s = Settings::instance();

    if (siteId.toString() != s->getSite()) {
        qWarning() << "Message's site ID is not my side ID";
        return false;
    }

    return true;
}
