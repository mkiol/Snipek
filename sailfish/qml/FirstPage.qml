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
                text: app.configured ? qsTr("Settings") : qsTr("Configure")
                onClicked: pageStack.push(Qt.resolvedUrl("SettingsPage.qml"))
            }

            MenuItem {
                visible: app.configured
                text: mqtt.connected ? qsTr("Disconnect") : qsTr("Connect")
                onClicked: mqtt.connected ? mqtt.deInit() : mqtt.init()
            }
        }

        Mic {
            visible: app.configured
            anchors.centerIn: parent
            size: root.width/2
            enabled: settings.sessionStart < 2 &&
                     aserver.connected && !aserver.insession
            onClicked: aserver.startSession()
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
            text: aserver.connected ? aserver.insession ?
                          qsTr("Listening") :
                          settings.sessionStart == 0 ? qsTr("Say wake word or tap") :
                          settings.sessionStart == 1 ? qsTr("Tap to wake up") :
                          settings.sessionStart == 2 ? qsTr("Say wake word") :
                          "" : qsTr("Disconnected")
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
