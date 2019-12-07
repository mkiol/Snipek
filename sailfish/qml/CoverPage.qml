/* Copyright (C) 2018 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.snipek.Mqtt 1.0

CoverBackground {
    id: root

    Mic {
        anchors.centerIn: parent
        size: root.width/2
    }

    CoverActionList {
        enabled: !aserver.connected ||
                 (settings.sessionStart < 2 && !aserver.insession)
        CoverAction {
            iconSource: aserver.connected ?
                            "image://theme/icon-cover-unmute" :
                            "image://theme/icon-cover-refresh"
            onTriggered: {
                if (aserver.connected) {
                    if (!aserver.insession)
                        aserver.startSession()
                } else {
                    if (mqtt.state === Mqtt.MqttConnected ||
                            mqtt.state === Mqtt.MqttDisconnected) {
                        mqtt.init()
                    }
                }
            }
        }
    }
}
