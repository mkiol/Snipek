/* Copyright (C) 2019 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "log.h"

#include <QStandardPaths>
#include <QDir>
#include <QByteArray>
#include <cstdlib>

FILE * logFile = nullptr;

void qtLog(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (!logFile) {
        QDir home(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
        auto file = home.filePath(LOG_FILE).toLatin1();
        logFile = fopen(file.data(), "w");
    }

    QByteArray localMsg = msg.toLocal8Bit();
    //const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";

    char t = '-';
    switch (type) {
    case QtDebugMsg:
        t = 'D';
        break;
    case QtInfoMsg:
        t = 'I';
        break;
    case QtWarningMsg:
        t = 'W';
        break;
    case QtCriticalMsg:
        t = 'C';
        break;
    case QtFatalMsg:
        t = 'F';
        break;
    }

    fprintf(logFile, "[%c] %s:%u - %s\n", t, function, context.line, localMsg.constData());
    fflush(logFile);
}
