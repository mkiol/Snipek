/* Copyright (C) 2019 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: root

    allowedOrientations: Orientation.All

    SilicaFlickable {
        id: flick
        anchors.fill: parent
        contentHeight: content.height

        Column {
            id: content

            width: root.width
            spacing: Theme.paddingMedium

            PageHeader {
                title: qsTr("Changelog")
            }

            SectionHeader {
                text: qsTr("Version %1").arg("2.0.0")
            }

            LogItem {
                title: "Built-in skills: Date and Time, Call history"
                description: "Snipek can be used as a remote mic/speaker but now it also " +
                             "provides skills for Snips assistant: " +
                             "(1) Date and Time skill that reads current date or time, " +
                             "(2) Call history skill that reads events from call history. " +
                             "To use these skills, Snipek assistant file has to be " +
                             "installed on Snips server. More information can " +
                             "be found on Snipek project website."
            }

            LogItem {
                title: "Tap wake-up method"
                description: "Next to wake word, now voice assistant can be " +
                             "activated with tap gesture. When Wake-up " +
                             "method is set to Tap gesture only, Snipek doesn't send " +
                             "any audio data to Snips server instance before " +
                             "assistant is activated."
            }

            LogItem {
                title: "Streaming a sound"
                description: "Latest Snips release introduced option to stream " +
                             "the sound to play instead of sending it all on one go. " +
                             "Support for this option was added in Snipek."
            }

            Spacer {}
        }
    }

    VerticalScrollDecorator {
        flickable: flick
    }
}
