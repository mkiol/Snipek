/* Copyright (C) 2018 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import QtQuick 2.0
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import "."

Page {
    id: root

    objectName: "about"

    header: PageHeader {
        title: qsTr("About")
    }

    Flickable {
        anchors.fill: parent
        anchors.margins: Theme.paddingMedium
        contentHeight: column.height
        contentWidth: parent.width - 2*Theme.paddingMedium

        ColumnLayout {
            id: column

            spacing: Theme.paddingMedium
            width: parent.width

            Label {
                font.pixelSize: Theme.fontSizeLarge
                text: APP_NAME
            }

            Label {
                font.pixelSize: Theme.fontSizeMedium
                text: qsTr("Version %1").arg(APP_VERSION);
            }

            /*Button {
                text: qsTr("Changelog")
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: pageStack.push(Qt.resolvedUrl("ChangelogPage.qml"))
            }*/

            PaddedLabel {
                textFormat: Text.RichText
                text: "Copyright &copy; 2018 Michal Kosciesza"
            }

            PaddedLabel {
                text: qsTr("%1 is a free application. The source code is " +
                           "subject to the terms of the Mozilla Public License, v. 2.0. " +
                           "If a copy of the MPL was not distributed with this " +
                           "app, You can obtain one at %2.").arg(APP_NAME).arg("http://mozilla.org/MPL/2.0/")
            }

            Label {
                text: qsTr("Third party components") + ":"
            }

            PaddedLabel {
                text: "Eclipse Paho MQTT lib"
            }
        }

        ScrollIndicator.vertical: ScrollIndicator {}
    }
}
