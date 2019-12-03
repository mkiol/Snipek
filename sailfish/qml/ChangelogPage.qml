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
                title: "Support for local Snips installation"
                description: "To make Snipek works you have to separately install " +
                             "Snips software. This release provides support for " +
                             "Snips installation directly on Sailfish OS phone. " +
                             "When Snipek and Snips are running on the same device, " +
                             "no network connectivity is required. Your phone can " +
                             "be in airplane mode and voice assistant sill works. " +
                             "Installation options (Local or Remote) can be configured " +
                             "in the Settings. To install Snips on Sailfish OS follow " +
                             "<a href=\"https://github.com/mkiol/Snipek" +
                             "#snips-installation-on-sailfish-os\">this guide</a>. " +
                             "Keep in mind that Snips is not an " +
                             "open source software. The source code is not publicly " +
                             "available. The use of Snips is is governed by " +
                             "<a href=\"https://docs.snips.ai/additional-resources" +
                             "/legal-and-privacy/website-terms-of-use\">" +
                             "Snips Terms of Use</a>."
            }

            LogItem {
                title: "Built-in skills: Date and Time, Call history"
                description: "Snipek can be used as a remote mic/speaker but it also " +
                             "provides skills that let you control your phone. " +
                             "Skills are the capabilities of " +
                             "voice assistant i.e. the things that assistant " +
                             "can do with a voice command. Following skills are " +
                             "currently supported: Date and Time and Call history. " +
                             "More information are <a href=\"https://github.com/" +
                             "mkiol/Snipek#snipek-built-in-skills\">here</a>."
            }

            LogItem {
                title: "Tap wake-up method"
                description: "Wake word (\"Hey Snips\" - default) is a primary " +
                             "way of voice assistant activation. Additionally, " +
                             "assistant can be activated with tap gesture. " +
                             "What method is enabled can be configured in the Settings."
            }

            Spacer {}
        }
    }

    VerticalScrollDecorator {
        flickable: flick
    }
}
