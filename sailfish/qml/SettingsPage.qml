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

    allowedOrientations: Orientation.All

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

            TextSwitch {
                automaticCheck: false
                checked: settings.audioFeedback
                text: qsTr("Audio feedback")
                description: qsTr("Enables notification sound when voice assistant is activated (e.g. a wake word is detected).")
                onClicked: {
                    settings.audioFeedback = !settings.audioFeedback
                }
            }

            ComboBox {
                width: parent.width
                label: qsTr("Wake-up method")
                description: qsTr("The way you can activate the voice assistant.")
                currentIndex: settings.sessionStart

                menu: ContextMenu {
                    MenuItem { text: qsTr("Wake word or tap gesture") }
                    MenuItem { text: qsTr("Tap gesture only") }
                    MenuItem { text: qsTr("Wake word only") }
                }

                onCurrentIndexChanged: {
                    settings.sessionStart = currentIndex
                }
            }

            ComboBox {
                width: parent.width
                label: qsTr("Language")
                description: qsTr("Language used for UI and built-in skills. Remember to install Snips assistant that supports selected language as well.")
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
                    MenuItem { text: "Deutsch" }
                    MenuItem { text: "English" }
                    MenuItem { text: "Español" }
                    MenuItem { text: "Français" }
                    MenuItem { text: "Italiano" }
                    MenuItem { text: "日本語" }
                    MenuItem { text: "Português (do Brasil)" }
                }

                onCurrentIndexChanged: {
                    switch (currentIndex) {
                    case 1:
                        settings.lang = "de"; break
                    case 2:
                        settings.lang = "en"; break
                    case 3:
                        settings.lang = "es"; break
                    case 4:
                        settings.lang = "fr"; break
                    case 5:
                        settings.lang = "it"; break
                    case 6:
                        settings.lang = "ja"; break
                    case 7:
                        settings.lang = "pt_br"; break
                    default:
                        settings.lang = "auto"
                    }
                }
            }

            SectionHeader {
                text: qsTr("Snips configuration")
            }

            TextField {
                width: parent.width
                inputMethodHints: Qt.ImhEmailCharactersOnly | Qt.ImhNoPredictiveText
                placeholderText: qsTr("Enter MQTT IP address (e.g. 192.168.1.5)")
                label: qsTr("IP address for MQTT")
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
                placeholderText: qsTr("Enter MQTT port number (e.g. 1883)")
                label: qsTr("MQTT port number")
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
                placeholderText: qsTr("Enter unique site ID")
                label: qsTr("Site ID")

                onTextChanged: {
                    settings.site = text
                }

                Component.onCompleted: {
                    text = settings.site
                }
            }

            TextField {
                width: parent.width
                inputMethodHints: Qt.ImhLatinOnly | Qt.ImhLowercaseOnly | Qt.ImhNoPredictiveText
                placeholderText: qsTr("Enter intents namespace for built-in skills (default is \"%1\")").arg("muki")
                label: qsTr("Intents namespace for built-in skills")

                onTextChanged: {
                    settings.intentNs = text
                }

                Component.onCompleted: {
                    text = settings.intentNs
                }
            }

            SectionHeader {
                text: qsTr("Skills")
            }

            Repeater {
                model: skills
                delegate: TextSwitch {
                    text: model.friendlyName
                    description: model.description
                    checked: settings.isSkillEnabled(model.name)
                    onCheckedChanged: {
                        settings.setSkillEnabled(model.name, checked)
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
