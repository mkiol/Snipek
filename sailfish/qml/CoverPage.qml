/* Copyright (C) 2018 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import QtQuick 2.0
import Sailfish.Silica 1.0

CoverBackground {
    id: root

    Mic {
        anchors.centerIn: parent
        size: root.width/2
    }

    CoverActionList {
        enabled: !server.connected
        CoverAction {
            iconSource: "image://theme/icon-cover-refresh"
            onTriggered: server.connectToMqtt()
        }
    }
}
