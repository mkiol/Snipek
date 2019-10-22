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
const QString AudioServer::mqttLoadedTopic = "hermes/audioServer/%1/loaded";
const QString AudioServer::mqttStreamFinishedTopic = "hermes/audioServer/%1/streamFinished";
const QString AudioServer::mqttPlayBytesStreamingTopic = "hermes/audioServer/%1/playBytesStreaming";
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
        buffer.append(data, int(maxSize));
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
    makeRiffHeader();
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

void AudioProcessor::makeRiffHeader()
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

AudioServer::AudioDetails AudioServer::audioDetailsFromRiff(const QByteArray &data)
{
    AudioDetails details;

    QDataStream in(data); // read-only
    in.setByteOrder(QDataStream::LittleEndian);

    char buf[4];
    in.readRawData(buf, 4);
    if (strcmp(buf, "RIFF") != 0) {
        qWarning() << "Data is not RIFF";
        details.invalid = true;
        return details;
    }

    in.skipRawData(16);
    quint16 audioFormat, channelCount, sampleSize;
    quint32 sampleRate, dataSize;
    in >> audioFormat >> channelCount >> sampleRate;
    in.skipRawData(6);
    in >> sampleSize;
    in.skipRawData(4);
    in >> dataSize;
    qDebug() << "audioFormat:" << audioFormat
             << "channelCount:" << channelCount
             << "sampleRate:" << sampleRate
             << "sampleSize:" << sampleSize
             << "dataSize:" << dataSize;

    details.invalid = audioFormat != 1; // 1 = PCM
    details.format.setChannelCount(channelCount);
    details.format.setSampleRate(sampleRate);
    details.format.setSampleSize(sampleSize);
    details.format.setCodec("audio/pcm");
    details.format.setByteOrder(QAudioFormat::LittleEndian);
    details.format.setSampleType(QAudioFormat::UnSignedInt);
    details.start = 44;
    details.size = dataSize;

    return details;
}

AudioServer::MessageDetails AudioServer::makeDetails(const Message &msg)
{
    const auto stype = QString(msg.topic).split('/');

    if (stype.at(0) == "hermes") {
        if (stype.size() == 7 && stype.at(3) == "playBytesStreaming") {
            return MessageDetails(MSG_STREAMBYTES, stype.at(2),
                        stype.at(4), stype.at(5).toInt(),
                        stype.at(6).toInt() == 0 ? false : true);
        }

        if (stype.size() == 5 && stype.at(3) == "playBytes") {
            return MessageDetails(MSG_PLAYBYTES, stype.at(2), stype.at(4), 0, true);
        }

        if (stype.size() == 3) {
            if (stype.at(2) == "sessionStarted") {
                return MessageDetails(MSG_SESSIONSTARTED);
            }
            if (stype.at(2) == "sessionEnded") {
                return MessageDetails(MSG_SESSIONENDED);
            }
        }
    } else {
        qWarning() << "Not hermes message received";
    }

    qWarning() << "Unknown message received";
    return MessageDetails(MSG_UNKNOWN);
}

void AudioServer::processMessage(const Message &msg)
{
    qDebug() << "Processing message id:" << msg.id << msg.topic;

    auto md = makeDetails(msg);

    switch (md.type) {
    case MSG_STREAMBYTES:
        qDebug() << "Play bytes stream received";
        qDebug() << "siteId:" << md.siteId;
        qDebug() << "reqId:" << md.reqId;
        qDebug() << "chunk:" << md.chunk;
        qDebug() << "lastChunk:" << md.lastChunk;
        play(md, msg);
        break;
    case MSG_PLAYBYTES:
        qDebug() << "Play bytes received";
        qDebug() << "siteId:" << md.siteId;
        qDebug() << "reqId:" << md.reqId;
        play(md, msg);
        break;
    case MSG_SESSIONSTARTED:
        qDebug() << "Session started received";
        qDebug() << "payload:" << msg.payload;
        if (checkSiteId(msg.payload))
            setInsession(true);
        break;
    case MSG_SESSIONENDED:
        qDebug() << "Session ended received";
        qDebug() << "payload:" << msg.payload;
        if (checkSiteId(msg.payload))
            setInsession(false);
        break;
    default:
        qWarning() << "Unknown message received";
    }
}

AudioServer* AudioServer::instance()
{
    if (AudioServer::inst == nullptr) {
        AudioServer::inst = new AudioServer();
    }

    return AudioServer::inst;
}

AudioServer::AudioServer(QObject* parent) :
    QThread(parent)
{
    connect(this, &AudioServer::audioWriteNeeded, this,
            &AudioServer::writeAudio, Qt::QueuedConnection);
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

    handleMqttConnected();

    QThread::exec();
    qDebug() << "Event loop exit, thread:" << QThread::currentThreadId();
}

void AudioServer::sendPlayFinished(const MessageDetails& md)
{
    qDebug() << "Sending PlayFinished";

    Message respMsg;
    respMsg.topic = AudioServer::mqttPlayFinishedTopic.arg(md.siteId).toUtf8();
    respMsg.payload = QString("{\"id\":\"%1\",\"siteId\":\"%2\",\"sessionId\":null}")
            .arg(md.reqId).arg(md.siteId).toUtf8();

    auto mqtt = MqttAgent::instance();
    mqtt->publish(respMsg);
}

void AudioServer::sendStreamFinished(const MessageDetails& md)
{
    qDebug() << "Sending StreamFinished";

    Message respMsg;
    respMsg.topic = AudioServer::mqttStreamFinishedTopic.arg(md.siteId).toUtf8();
    respMsg.payload = QString("{\"id\":\"%1\",\"siteId\":\"%2\"}")
            .arg(md.reqId).arg(md.siteId).toUtf8();
    qDebug() << "data:" << respMsg.payload;

    auto mqtt = MqttAgent::instance();
    mqtt->publish(respMsg);
}

void AudioServer::playFinishedHandler()
{
    const auto& md = reqIdToDetailsMap[currReqId];
    qDebug() << "Play finished for reqId:" << md.reqId;

    if (md.type == MSG_STREAMBYTES) {
        sendStreamFinished(md);
    } else {
        sendPlayFinished(md);
    }

    reqIdToDetailsMap.remove(currReqId);
    reqIdToDataMap.remove(currReqId);
    currReqId.clear();

    playQueue.pop();
    if (playQueue.empty())
        resumeListening();
    else
        playNext();
}

void AudioServer::subscribe()
{
    auto mqtt = MqttAgent::instance();
    QString siteId = Settings::instance()->getSite();
    // play bytes
    mqtt->subscribe(AudioServer::mqttPlayBytesTopic.arg(siteId) + "/#");
    mqtt->subscribe(AudioServer::mqttPlayBytesStreamingTopic.arg(siteId) + "/#");
    // session started/ended
    mqtt->subscribe(AudioServer::mqttSessionStartedTopic);
    mqtt->subscribe(AudioServer::mqttSessionEndedTopic);
}

void AudioServer::handleMqttConnected()
{
    bool mqttConnected = MqttAgent::instance()->isConnected();
    qDebug() << "MQTT connected changed:" << mqttConnected;

    if (mqttConnected) {
        loaded();
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
    output.reset();
    buffer = nullptr;
}

void AudioServer::init()
{
    qDebug() << "Initing audio server";

    connect(this, &AudioServer::processorInited, this, &AudioServer::startListening);

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
            this, &AudioServer::handleMqttConnected);
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

    QString siteId = Settings::instance()->getSite();

    Message msg;
    msg.topic = enabled ? AudioServer::mqttFeedbackOnTopic.toUtf8() :
                          AudioServer::mqttFeedbackOffTopic.toUtf8();
    msg.payload = QString("{\"siteId\":\"%1\"}").arg(siteId).toUtf8();
    qDebug() << "data:" << msg.payload;

    auto mqtt = MqttAgent::instance();
    mqtt->publish(msg);
}

void AudioServer::loaded()
{
    QString siteId = Settings::instance()->getSite();

    Message msg;
    msg.topic = AudioServer::mqttLoadedTopic.arg(siteId).toUtf8();
    msg.payload = QString("{\"id\":null,\"reloaded\":false,\"siteId\":\"%1\"}").arg(siteId).toUtf8();
    qDebug() << "data:" << msg.payload;

    auto mqtt = MqttAgent::instance();
    mqtt->publish(msg);
}

void AudioServer::handleAudioOutputStateChanged(QAudio::State newState)
{
    qDebug() << "handleAudioOutputStateChanged";

    switch (newState) {
    case QAudio::IdleState:
        qDebug() << "State: idle";
        if (isPlayingFinished()) {
            setPlaying(false);
            playFinishedHandler();
            output.reset();
            buffer = nullptr;
        } else {
            emit audioWriteNeeded();
        }
        break;
    case QAudio::StoppedState:
        qDebug() << "State: stopped";
        break;
    case QAudio::SuspendedState:
        qDebug() << "State: suspend";
        break;
    case QAudio::ActiveState:
        qDebug() << "State: active";
        setPlaying(true);
        suspendListening();
        break;
    default:
        qDebug() << "State: other";
    }
}

void AudioServer::updateMd(const MessageDetails& newMd, MessageDetails& md)
{
    md.chunk = newMd.chunk;
    md.lastChunk = newMd.lastChunk;
}

void AudioServer::play(const MessageDetails& md, const Message& msg)
{
    auto ad = audioDetailsFromRiff(msg.payload);
    if (ad.invalid) {
        qDebug() << "Invalid audio format";
        return;
    }

    if (currReqId == md.reqId) {
        updateMd(md, reqIdToDetailsMap[md.reqId]);
        reqIdToDataMap[md.reqId].append(msg.payload.data()+ad.start, ad.size);
    } else if (reqIdToDataMap.contains(md.reqId)) {
        updateMd(md, reqIdToDetailsMap[md.reqId]);
        reqIdToDataMap[md.reqId].append(msg.payload.data()+ad.start, ad.size);
    } else {
        reqIdToDetailsMap[md.reqId] = md;
        reqIdToDetailsMap[md.reqId].audioDetails = ad;
        reqIdToDataMap[md.reqId].append(msg.payload.data()+ad.start, ad.size);

        bool play = playQueue.empty();
        playQueue.push(md.reqId);
        if (play)
            playNext();
    }
}

void AudioServer::playNext()
{
    qDebug() << "Play next";

    if (playQueue.empty()) {
        qWarning() << "Out queue is empty";
        return;
    }

    auto& md = reqIdToDetailsMap[playQueue.front()];

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    if (!info.isFormatSupported(md.audioDetails.format)) {
        qWarning() << "Audio format not supported";
        if (output) {
            output.reset();
            buffer = nullptr;
        }
        playQueue.pop();
        return;
    }

    currReqId = md.reqId;

    /*if (output && output->format() == md.audioDetails.format) {
        qDebug() << "Reusing already existing audio output";
    } else {
        qDebug() << "Creating new audio output";
        output = std::unique_ptr<QAudioOutput>(new QAudioOutput(md.audioDetails.format));
        output->setBufferSize(10000);
        qDebug() << "output->setBufferSize:" << output->bufferSize();
        connect(output.get(), &QAudioOutput::stateChanged,
                this, &AudioServer::handleAudioOutputStateChanged);
        buffer = output->start();
    }*/

    qDebug() << "Creating new audio output";
    output = std::unique_ptr<QAudioOutput>(new QAudioOutput(md.audioDetails.format));
    //output->setBufferSize(10000);
    connect(output.get(), &QAudioOutput::stateChanged,
            this, &AudioServer::handleAudioOutputStateChanged);
    buffer = output->start();
}

bool AudioServer::isPlayingFinished()
{
    if (reqIdToDataMap.contains(currReqId)) {
        return reqIdToDetailsMap[currReqId].lastChunk &&
                reqIdToDataMap[currReqId].isEmpty();
    }

    qWarning() << "Unknown reqId";
    return true;
}

void AudioServer::setPlaying(bool playing)
{
    if (playing != this->playing) {
        this->playing = playing;
        emit playingChanged();
    }
}

void AudioServer::writeAudio()
{
    if (buffer && reqIdToDataMap.contains(currReqId)) {
        int size = reqIdToDataMap[currReqId].size();
        //qDebug() << "writeAudio size:" << size;

        if (size > 0) {
            int wrote = buffer->write(reqIdToDataMap[currReqId]);
            reqIdToDataMap[currReqId].remove(0, wrote);
            /*if (wrote > 0) {
                qDebug() << "size:" << size << "wrote:" << wrote
                         << "left:" << size - wrote << "error:" << output->error();
            }*/
            if (output->error() == QAudio::NoError) {
                size -= wrote;
            } else {
                qWarning() << "Audio output error, so ending playing";
                size = 0;
                reqIdToDataMap[currReqId].clear();
            }
        }

        if (size == 0) {
            qDebug() << "All data was written";
            if (!reqIdToDetailsMap[currReqId].lastChunk) {
                qDebug() << "Didn't receive last chunk, so waiting";
                //TODO Make timeout for waiting
                emit audioWriteNeeded();
            }
        } else {
            emit audioWriteNeeded();
        }
    } else {
        qWarning() << "Unknown id, so ignoring";
    }
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

    //TODO Clear audio streams when session is ended
    if (!value && !reqIdToDetailsMap.isEmpty()) {
        qWarning() << "Session ended but some streams are not finished";
        //reqIdToDetailsMap.clear();
        //output.reset();
        //buffer = nullptr;
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

AudioServer::MessageDetails::MessageDetails(MessageType type, QString siteId,
               QString reqId, int chunk, bool lastChunk) :
    type(type), siteId(siteId), reqId(reqId), chunk(chunk), lastChunk(lastChunk)
{
}

AudioServer::MessageDetails::MessageDetails() :
    type(MSG_UNKNOWN), chunk(0), lastChunk(0)
{
}
