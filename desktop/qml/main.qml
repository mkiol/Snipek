import QtQuick 2.0
import QtQuick.Controls 2.2

ApplicationWindow {
    id: app

    property bool configured: settings.mqttAddress.length > 0

    visible: true
    minimumWidth: 640
    minimumHeight: 480
    width: minimumWidth
    height: minimumHeight
    title: APP_NAME

    /*Connections {
        target: server
        onError: {
            switch (error) {
            case 1:
                notification.show("Can't connect because IP address is not defined")
                break;
            case 2:
                notification.show("Connection problem")
                break;
            default:
                notification.show("Whoops, something went wrong")
            }
        }
    }*/

    onClosing: {
        server.close()
        close.accepted = false
    }

    footer: Label {
        text: configured ? server.connected ? server.insession ?
                                     qsTr("Listening") :
                                     qsTr("Idle") :
                                     qsTr("Disconnected") :
                                     qsTr("Not configured")
    }

    StackView {
        id: pageStack

        anchors.fill: parent

        initialItem: Component {
            FirstPage {}
        }
    }
}
