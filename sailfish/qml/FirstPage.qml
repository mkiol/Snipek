/* Copyright (C) 2018 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: root

    SilicaFlickable {
        id: flick
        anchors.fill: parent
        contentHeight: parent.height

        PullDownMenu {
            MenuItem {
                text: qsTr("About")
                onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
            }

            MenuItem {
                text: qsTr("Settings")
                onClicked: pageStack.push(Qt.resolvedUrl("SettingsPage.qml"))
            }

            MenuItem {
                text: server.connected ? qsTr("Disconnect") : qsTr("Connect")
                onClicked: {
                    if (server.connected)
                        server.disconnectFromMqtt()
                    else
                        server.connectToMqtt()
                }
            }
        }

        Mic {
            visible: app.configured
            anchors.centerIn: parent
            size: root.width/2
        }

        Label {
            anchors {
                horizontalCenter: parent.horizontalCenter
                bottom: parent.bottom
                bottomMargin: Theme.paddingLarge
            }
            visible: app.configured
            color: Theme.highlightColor
            font.pixelSize: Theme.fontSizeLarge
            text: server.connected ? server.insession ?
                                         qsTr("Listening") :
                                         qsTr("Idle") :
            qsTr("Disconnected")
        }

        ViewPlaceholder {
            enabled: !app.configured
            text: qsTr("Not configured")
        }
    }

    VerticalScrollDecorator {
        flickable: flick
    }
}
