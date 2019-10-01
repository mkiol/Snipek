/* Copyright (C) 2018 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import QtQuick 2.0
import Sailfish.Silica 1.0

Rectangle {
    id: root

    property alias size: root.width
    property bool active: aserver.insession

    property color dotCol1: aserver.connected ? "#ffffff" : Qt.darker(root.color)
    property color dotCol2: Qt.darker(root.color)
    property int dotSize: size/8

    color: aserver.connected ?
               "#1906d0" :
               "#666666"

    radius: size/2
    height: size

    Row {
        anchors.centerIn: root
        spacing: size/10
        Repeater {
            model: 3
            Column {
                spacing: size/10
                Repeater {
                    model: 3
                    Rectangle {
                        width: root.dotSize
                        height: width
                        radius: width/2
                        color: root.dotCol1

                        Timer {
                            interval: Math.floor((Math.random() * 200) + 200);
                            running: root.active
                            repeat: true
                            onTriggered: {
                                if (parent.color === root.dotCol1)
                                    parent.color = Qt.binding(function() { return root.dotCol2 })
                                else
                                    parent.color = Qt.binding(function() { return root.dotCol1 })
                            }

                            onRunningChanged: {
                                if (!running)
                                    parent.color = Qt.binding(function() { return root.dotCol1 })
                            }
                        }
                    }
                }
            }
        }
    }
}
