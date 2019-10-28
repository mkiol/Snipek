/* Copyright (C) 2018 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import QtQuick 2.0
import Sailfish.Silica 1.0

MouseArea {
    id: root
    property alias size: root.width
    readonly property int size2: 0.9*size
    property bool active: aserver.insession
    property alias color: mic.color
    property color dotCol1: aserver.connected ? "#ffffff" : Qt.darker(root.color)
    property color dotCol2: Qt.darker(root.color)
    property int dotSize: size2/8

    height: size

    Rectangle {
        property int size: root.pressed ? root.size : size2
        Behavior on size {
            NumberAnimation { easing.type: Easing.OutCubic; duration: 200 }
        }
        visible: size != size2
        anchors.centerIn: parent
        color: Qt.lighter(root.color)
        radius: size/2
        height: size
        width: size
    }

    Rectangle {
        id: mic
        anchors.centerIn: parent
        radius: size2/2
        height: size2
        width: size2

        color: aserver.connected ?
                   "#1906d0" :
                   "#666666"

        Row {
            anchors.centerIn: parent
            spacing: size2/10
            Repeater {
                model: 3
                Column {
                    spacing: size2/10
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
}
