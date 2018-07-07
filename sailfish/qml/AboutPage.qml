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
        anchors.fill: parent

        contentHeight: column.height

        Column {
            id: column

            width: root.width
            spacing: Theme.paddingLarge

            PageHeader {
                title: qsTr("About")
            }

            Image {
                anchors.horizontalCenter: parent.horizontalCenter
                source: "image://icons/icon-i-snipek"
            }

            InfoLabel {
                text: APP_NAME
            }

            PaddedLabel {
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: Theme.fontSizeMedium
                color: Theme.highlightColor
                text: qsTr("Version %1").arg(APP_VERSION);
            }

            /*Button {
                text: qsTr("Changelog")
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: pageStack.push(Qt.resolvedUrl("ChangelogPage.qml"))
            }*/

            SectionHeader {
                text: qsTr("Authors & license")
            }

            PaddedLabel {
                horizontalAlignment: Text.AlignLeft
                textFormat: Text.RichText
                text: "Copyright &copy; 2018 Michal Kosciesza"
            }

            PaddedLabel {
                horizontalAlignment: Text.AlignLeft
                text: qsTr("%1 is a free application. The source code is " +
                           "subject to the terms of the Mozilla Public License, v. 2.0. " +
                           "If a copy of the MPL was not distributed with this " +
                           "app, You can obtain one at %2.").arg(APP_NAME).arg("http://mozilla.org/MPL/2.0/")
            }

            SectionHeader {
                text: qsTr("Third party components")
            }

            PaddedLabel {
                horizontalAlignment: Text.AlignLeft
                textFormat: Text.RichText
                text: "Eclipse Paho MQTT lib"
            }

            Spacer {}
        }

        VerticalScrollDecorator {}
    }
}
