/* Copyright (C) 2018 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.snipek.Snips 1.0

Page {
    id: root

    allowedOrientations: Orientation.All

    Component.onCompleted: {
        if (settings.snipsLocal)
            snips.checkSnips()
    }

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

            ComboBox {
                width: parent.width
                label: qsTr("Wake-up method")
                description: qsTr("The way you can activate the voice assistant. The default wake word is \"Hey Snips\".")
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

            TextSwitch {
                automaticCheck: false
                checked: settings.audioFeedback
                text: qsTr("Audio feedback")
                description: qsTr("Enables sound notification when voice assistant is activated.")
                onClicked: {
                    settings.audioFeedback = !settings.audioFeedback
                }
            }

            SectionHeader {
                text: qsTr("Snips configuration")
            }

            ComboBox {
                visible: settings.isArm()
                width: parent.width
                label: qsTr("Installation option")
                description: qsTr("Snips can be installed on this device (Local) or on another device in your home network (Remote).")
                currentIndex: settings.snipsLocal ? 0 : 1

                menu: ContextMenu {
                    MenuItem { text: qsTr("Local") }
                    MenuItem { text: qsTr("Remote") }
                }

                onCurrentIndexChanged: {
                    settings.snipsLocal = currentIndex == 0
                }
            }

            ListItem {
                visible: settings.snipsLocal
                contentHeight: visible ? recflow.height + 2 * Theme.paddingLarge : 0

                Flow {
                    id: recflow
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: parent.left; right: parent.right
                        leftMargin: Theme.paddingLarge
                        rightMargin: Theme.paddingLarge
                    }
                    spacing: Theme.paddingMedium

                    Label {
                        text: qsTr("Installation directory")
                    }

                    Label {
                        color: Theme.highlightColor
                        text: settings.snipsLocalDir.replace(/^.*[\\\/]/, '')
                    }
                }

                onClicked: openMenu()

                menu: ContextMenu {
                    MenuItem {
                        text: qsTr("Change")
                        onClicked: {
                            var obj = pageStack.push(Qt.resolvedUrl("DirPage.qml"));
                            obj.accepted.connect(function() {
                                settings.snipsLocalDir = obj.selectedPath
                            })
                        }
                    }
                    MenuItem {
                        text: qsTr("Set default")
                        onClicked: {
                            settings.snipsLocalDir = ""
                        }
                    }
                }
            }

            TextField {
                visible: !settings.snipsLocal
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
                visible: !settings.snipsLocal
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
                visible: !settings.snipsLocal
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

            ListItem {
                visible: settings.snipsLocal
                contentHeight: visible ? snipsLabel.height + 2 * Theme.paddingLarge : 0

                Rectangle {
                    anchors.fill: parent
                    color: {
                        switch(snips.snipsStatus) {
                        case Snips.SnipsStopped:
                            return Theme.rgba("red", 0.3)
                        case Snips.SnipsStarted:
                            return Theme.rgba("green", 0.3)
                        case Snips.SnipsNotInstalled:
                        case Snips.SnipsOutdated:
                        default:
                            return Theme.rgba("grey", 0.3)
                        }
                    }
                }

                Label {
                    id: snipsLabel
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: parent.left; right: parent.right
                        leftMargin: Theme.paddingLarge
                        rightMargin: Theme.paddingLarge
                    }
                    horizontalAlignment: Text.AlignHCenter
                    text: {
                        switch(snips.snipsStatus) {
                        case Snips.SnipsNotInstalled:
                            return qsTr("Snips is not installed")
                        case Snips.SnipsOutdated:
                            return qsTr("Snips is outdated")
                        case Snips.SnipsStopped:
                            return qsTr("Snips is not running")
                        case Snips.SnipsStarted:
                            return qsTr("Snips is running")
                        default:
                            return qsTr("Snips status is unknown")
                        }
                    }
                }

                onClicked: openMenu()

                menu: ContextMenu {
                    MenuItem {
                        visible: snips.snipsStatus === Snips.SnipsStopped
                        text: qsTr("Start")
                        onClicked: snips.startSnips()
                    }
                    MenuItem {
                        visible: snips.snipsStatus === Snips.SnipsStarted
                        text: qsTr("Stop")
                        onClicked: snips.stopSnips()
                    }
                    MenuItem {
                        text: qsTr("Refresh status")
                        onClicked: snips.checkSnips()
                    }
                }
            }

            PaddedLabel {
                visible: settings.snipsLocal && snips.snipsStatus === Snips.SnipsNotInstalled
                text: qsTr("To make %1 works, Snips has to be installed. " +
                           "Use <a href=\"%2\">this</a> guide to download and set up Snips.")
                            .arg(APP_NAME).arg(PAGE_SNIPS_INSTALL)
            }

            PaddedLabel {
                visible: settings.snipsLocal && snips.snipsStatus === Snips.SnipsOutdated
                text: qsTr("Snips has to be re-installed. " +
                           "Use <a href=\"%1\">this</a> guide to download and set up Snips.")
                            .arg(PAGE_SNIPS_INSTALL)
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

            SectionHeader {
                text: qsTr("Advanced options")
            }

            Slider {
                width: parent.width
                minimumValue: 1
                maximumValue: 10
                stepSize: 1
                handleVisible: true
                value: settings.volume
                valueText: value
                label: qsTr("Volume boost")

                onValueChanged: {
                    settings.volume = value
                }
            }

            ComboBox {
                width: parent.width
                label: qsTr("Language")
                description: qsTr("Language used for UI and built-in skills. Assistant that supports selected language has to be installed as well.")
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

            TextField {
                width: parent.width
                inputMethodHints: Qt.ImhLatinOnly | Qt.ImhLowercaseOnly | Qt.ImhNoPredictiveText
                placeholderText: qsTr("Enter namespace for built-in skills (default is \"%1\")").arg("muki")
                label: qsTr("Namespace for built-in skills")

                onTextChanged: {
                    settings.intentNs = text
                }

                Component.onCompleted: {
                    text = settings.intentNs
                }
            }

            TextSwitch {
                automaticCheck: false
                checked: settings.logToFile
                text: qsTr("Enable logging")
                description: qsTr("Needed for troubleshooting purposes. " +
                                  "The log data is stored in %1 file.").arg("/home/nemo/snipek.log")
                onClicked: {
                    settings.logToFile = !settings.logToFile
                }
            }

            Spacer {}
        }

        VerticalScrollDecorator {}
    }
}
