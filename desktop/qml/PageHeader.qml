/* Copyright (C) 2018 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import QtQuick 2.0
import QtQuick.Controls 2.2
import "."

ToolBar {
    id: root

    property alias title: label.text

    height: Math.max(back_but.height, label.height, menu_but.height) +
            Theme.paddingSmall

    ToolButton {
        id: back_but
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        visible: pageStack.depth > 1
        text: "‹"
        font.bold: true
        onClicked: pageStack.pop()
    }

    Label {
        id: label
        anchors.centerIn: parent
        elide: Label.ElideRight
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter
    }

    ToolButton {
        id: menu_but
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        visible: pageStack.currentItem.objectName === "first_page"
        text: "⋮"
        font.bold: true
        onClicked: menu.open()
    }

    Menu {
        id: menu

        title: qsTr("Options")
        x: parent.width-width
        y: 0

        MenuItem {
            text: server.connected ? qsTr("Disconnect") : qsTr("Connect")
            onTriggered: {
                if (server.connected)
                    server.disconnectFromMqtt()
                else
                    server.connectToMqtt()
            }
        }

        MenuItem {
            text: qsTr("Settings")
            enabled: pageStack.currentItem.objectName === "first_page"
            onTriggered: {
                pageStack.push(Qt.resolvedUrl("SettingsPage.qml"))
            }
        }

        MenuItem {
            text: qsTr("About")
            enabled: pageStack.currentItem.objectName === "first_page"
            onTriggered: {
                pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
            }
        }
    }
}
