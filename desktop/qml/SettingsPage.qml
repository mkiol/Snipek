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

    objectName: "settings"

    header: PageHeader {
        title: qsTr("Settings")
    }

    Flickable {
        anchors.fill: parent
        anchors.margins: Theme.paddingMedium
        contentHeight: grid.height

        GridLayout {
            id: grid

            columns: 2
            rows: 12
            columnSpacing: Theme.paddingMedium
            rowSpacing: Theme.paddingSmall
            anchors.fill: parent

            Label {
                text: qsTr("MQTT broker address")
            }

            TextField {
                width: parent.width
                inputMethodHints: Qt.ImhEmailCharactersOnly | Qt.ImhNoPredictiveText
                placeholderText: qsTr("Enter IP address")
                validator: RegExpValidator { regExp: /^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$|^(([a-zA-Z]|[a-zA-Z][a-zA-Z0-9\-]*[a-zA-Z0-9])\.)*([A-Za-z]|[A-Za-z][A-Za-z0-9\-]*[A-Za-z0-9])$|^\s*((([0-9A-Fa-f]{1,4}:){7}([0-9A-Fa-f]{1,4}|:))|(([0-9A-Fa-f]{1,4}:){6}(:[0-9A-Fa-f]{1,4}|((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3})|:))|(([0-9A-Fa-f]{1,4}:){5}(((:[0-9A-Fa-f]{1,4}){1,2})|:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3})|:))|(([0-9A-Fa-f]{1,4}:){4}(((:[0-9A-Fa-f]{1,4}){1,3})|((:[0-9A-Fa-f]{1,4})?:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){3}(((:[0-9A-Fa-f]{1,4}){1,4})|((:[0-9A-Fa-f]{1,4}){0,2}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){2}(((:[0-9A-Fa-f]{1,4}){1,5})|((:[0-9A-Fa-f]{1,4}){0,3}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){1}(((:[0-9A-Fa-f]{1,4}){1,6})|((:[0-9A-Fa-f]{1,4}){0,4}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(:(((:[0-9A-Fa-f]{1,4}){1,7})|((:[0-9A-Fa-f]{1,4}){0,5}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:)))(%.+)?\s*$/ }

                onTextChanged: {
                    settings.mqttAddress = text
                }

                Component.onCompleted: {
                    text = settings.mqttAddress
                }
            }

            Label {
                text: qsTr("MQTT broker port")
            }

            TextField {
                width: parent.width
                inputMethodHints: Qt.ImhDigitsOnly | Qt.ImhNoPredictiveText
                placeholderText: qsTr("Enter port number")
                validator: IntValidator { bottom: 1; top: 65535 }

                onTextChanged: {
                    settings.mqttPort = parseInt(text)
                }

                Component.onCompleted: {
                    text = settings.mqttPort
                }
            }

            Label {
                text: qsTr("Site ID")
            }

            TextField {
                width: parent.width
                inputMethodHints: Qt.ImhEmailCharactersOnly | Qt.ImhNoPredictiveText
                placeholderText: qsTr("Enter site ID")

                onTextChanged: {
                    settings.site = text
                }

                Component.onCompleted: {
                    text = settings.site
                }
            }

            /*Label {
                text: qsTr("Input audio device")
            }

            ComboBox {
                model: ListModel {
                    id: inAudioModel
                }

                Component.onCompleted: {
                    var list = server.getInAudioDevices()
                    var l = list.length

                    var cidx = 0;
                    var ctext = settings.inAudio

                    inAudioModel.append({text: qsTr("Auto")})
                    for (var i = 0; i < l; i++) {
                        inAudioModel.append({text: list[i]})
                        if (list[i] === ctext)
                            cidx = i + 1
                    }

                    currentIndex = cidx;
                }

                onActivated: {
                    settings.inAudio = (currentIndex === 0 ?
                                "" : currentText)
                }
            }

            Label {
                text: qsTr("Output audio device")
            }

            ComboBox {
                model: ListModel {
                    id: outAudioModel
                }

                Component.onCompleted: {
                    var list = server.getOutAudioDevices()
                    var l = list.length

                    var cidx = 0;
                    var ctext = settings.outAudio

                    outAudioModel.append({text: qsTr("Auto")})
                    for (var i = 0; i < l; i++) {
                        outAudioModel.append({text: list[i]})
                        if (list[i] === ctext)
                            cidx = i + 1
                    }

                    currentIndex = cidx;
                }

                onActivated: {
                    settings.outAudio = (currentIndex === 0 ?
                                "" : currentText)
                }
            }*/

            /*SectionHeader {
                text: qsTr("Experimental features")
            }*/

            //Spacer {}
        }

        ScrollIndicator.vertical: ScrollIndicator {}
    }
}
