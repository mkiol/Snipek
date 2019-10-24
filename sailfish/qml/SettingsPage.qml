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
        contentHeight: column.height

        Column {
            id: column

            width: root.width
            spacing: Theme.paddingMedium

            PageHeader {
                title: qsTr("Settings")
            }

            TextField {
                width: parent.width
                inputMethodHints: Qt.ImhEmailCharactersOnly | Qt.ImhNoPredictiveText
                placeholderText: qsTr("Enter Snips IP address")
                label: qsTr("Snips IP address (e.g. 192.168.1.5)")
                validator: RegExpValidator { regExp: /^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$|^(([a-zA-Z]|[a-zA-Z][a-zA-Z0-9\-]*[a-zA-Z0-9])\.)*([A-Za-z]|[A-Za-z][A-Za-z0-9\-]*[A-Za-z0-9])$|^\s*((([0-9A-Fa-f]{1,4}:){7}([0-9A-Fa-f]{1,4}|:))|(([0-9A-Fa-f]{1,4}:){6}(:[0-9A-Fa-f]{1,4}|((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3})|:))|(([0-9A-Fa-f]{1,4}:){5}(((:[0-9A-Fa-f]{1,4}){1,2})|:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3})|:))|(([0-9A-Fa-f]{1,4}:){4}(((:[0-9A-Fa-f]{1,4}){1,3})|((:[0-9A-Fa-f]{1,4})?:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){3}(((:[0-9A-Fa-f]{1,4}){1,4})|((:[0-9A-Fa-f]{1,4}){0,2}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){2}(((:[0-9A-Fa-f]{1,4}){1,5})|((:[0-9A-Fa-f]{1,4}){0,3}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){1}(((:[0-9A-Fa-f]{1,4}){1,6})|((:[0-9A-Fa-f]{1,4}){0,4}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(:(((:[0-9A-Fa-f]{1,4}){1,7})|((:[0-9A-Fa-f]{1,4}){0,5}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:)))(%.+)?\s*$/ }

                onTextChanged: {
                    settings.mqttAddress = text
                }

                Component.onCompleted: {
                    text = settings.mqttAddress
                }
            }

            TextField {
                width: parent.width
                inputMethodHints: Qt.ImhDigitsOnly | Qt.ImhNoPredictiveText
                placeholderText: qsTr("Enter Snips port number")
                label: qsTr("Snips port number (e.g. 1883)")
                validator: IntValidator { bottom: 1; top: 65535 }

                onTextChanged: {
                    settings.mqttPort = parseInt(text)
                }

                Component.onCompleted: {
                    text = settings.mqttPort
                }
            }

            TextField {
                width: parent.width
                inputMethodHints: Qt.ImhEmailCharactersOnly | Qt.ImhNoPredictiveText
                placeholderText: qsTr("Enter site ID")
                label: qsTr("Site ID")

                onTextChanged: {
                    settings.site = text
                }

                Component.onCompleted: {
                    text = settings.site
                }
            }

            TextSwitch {
                automaticCheck: false
                checked: settings.audioFeedback
                text: qsTr("Audio feedback")
                onClicked: {
                    settings.audioFeedback = !settings.audioFeedback
                }
            }

            ComboBox {
                width: parent.width
                label: qsTr("Snips language")
                currentIndex: {
                    if (settings.snipsLang === "de")
                        return 1;
                    if (settings.snipsLang === "en")
                        return 2;
                    if (settings.snipsLang === "es")
                        return 3;
                    if (settings.snipsLang === "fr")
                        return 4;
                    if (settings.snipsLang === "it")
                        return 5;
                    if (settings.snipsLang === "ja")
                        return 6;
                    if (settings.snipsLang === "pt_br")
                        return 7;
                    return 0;
                }

                menu: ContextMenu {
                    MenuItem { text: qsTr("Auto") }
                    MenuItem { text: qsTr("German") }
                    MenuItem { text: qsTr("English") }
                    MenuItem { text: qsTr("Spanish") }
                    MenuItem { text: qsTr("French") }
                    MenuItem { text: qsTr("Italian") }
                    MenuItem { text: qsTr("Japanese") }
                    MenuItem { text: qsTr("Portuguese (Brazil)") }
                }

                onCurrentIndexChanged: {
                    switch (currentIndex) {
                    case 1:
                        settings.snipsLang = "de"; break
                    case 2:
                        settings.snipsLang = "en"; break
                    case 3:
                        settings.snipsLang = "es"; break
                    case 4:
                        settings.snipsLang = "fr"; break
                    case 5:
                        settings.snipsLang = "it"; break
                    case 6:
                        settings.snipsLang = "ja"; break
                    case 7:
                        settings.snipsLang = "pt_br"; break
                    default:
                        settings.snipsLang = "auto"
                    }
                }
            }

            /*SectionHeader {
                text: qsTr("Experimental features")
            }*/

            Spacer {}
        }

        VerticalScrollDecorator {}
    }
}
