
import QtQuick 2.1

Item {
    Rectangle {
        width: parent.width
        height: 50
        color: "#AA64C2D4"

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton

            onClicked: console.log("click")
        }

        Row {
            anchors.fill: parent
            anchors.margins: 5
            Text {
                width: 100
                height: parent.height
                text: "fps: " + Fps.toFixed(2) + "\nms/f: " + (1000. / Fps).toFixed(2)
            }
            Text {
                width: 100
                height: parent.height
                text: "Num triangles: " + NumTriangles + "\nNum draw calls: " + NumDrawCalls
            }
        }
    }

}
