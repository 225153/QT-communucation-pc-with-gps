import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15

ApplicationWindow {
    visible: true
    width: 400
    height: 300
    title: "GPS via AT Commands"

    Column {
        anchors.centerIn: parent
        spacing: 20

        Text {
            id: gpsText
            text: "Waiting for GPS data..."
            font.pixelSize: 20
            horizontalAlignment: Text.AlignHCenter
        }

        Row {
            spacing: 10
            Button {
                text: "Request Position"
                onClicked: gps.requestPosition()
            }
            Button {
                text: "Stop GPS"
                onClicked: gps.stop()
            }
        }
    }

    Connections {
        target: gps
        function onPositionUpdated() {
            gpsText.text = "Latitude: " + gps.latitude.toFixed(10) +
                           "\nLongitude: " + gps.longitude.toFixed(10)+
                           "\nAltitude: " + gps.altitude.toFixed(3) + " m";
        }
        function onErrorOccurred(message) {
            console.error(message)
        }
    }
}
