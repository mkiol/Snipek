/* Copyright (C) 2018 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import QtQuick 2.0
import Sailfish.Silica 1.0

ApplicationWindow
{
    id: app

    property bool configured: settings.mqttAddress.length > 0

    allowedOrientations: Orientation.PortraitMask

    cover: Qt.resolvedUrl("CoverPage.qml")

    initialPage: Component {
        FirstPage {}
    }

    Notification {
        id: notification
    }

    Connections {
        target: server
        onError: {
            switch (error) {
            case 1:
                notification.show(qsTr("Cannot connect because Snips MQTT broker IP address is not defined"))
                break;
            case 2:
                notification.show(qsTr("Connection problem"))
                break;
            default:
                notification.show(qsTr("Whoops, something went wrong"))
            }
        }
    }
}
