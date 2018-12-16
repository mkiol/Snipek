/* Copyright (C) 2018 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef MESSAGE_H
#define MESSAGE_H

#include <QByteArray>

struct Message {
    int id = 0;
    QByteArray topic;
    QByteArray payload;
};

#endif // MESSAGE_H
