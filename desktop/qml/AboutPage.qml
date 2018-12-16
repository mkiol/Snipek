/* Copyright (C) 2018 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import QtQuick 2.0
import QtQuick.Controls 2.2
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

            PaddedLabel {
                textFormat: Text.RichText
                text: "Copyright &copy; 2018 Michal Kosciesza"
            }

            PaddedLabel {
                text: qsTr("%1 is developed as an open source project under %2.")
                .arg(APP_NAME)
                .arg("<a href=\"" + LICENSE_URL + "\">" + LICENSE + "</a>")
            }

            Label {
                text: qsTr("Libraries") + ":"
            }

            PaddedLabel {
                text: "Eclipse Paho MQTT lib"
            }
        }

        ScrollIndicator.vertical: ScrollIndicator {}
    }
}
