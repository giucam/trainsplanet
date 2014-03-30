
import QtQuick 2.1

Item {
    id: root

    function resetFocus() {
        root.focus = true
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onPressed: {
            resetFocus()
            mouse.accepted = false
        }

        Rectangle {
            width: parent.width
            height: 50
            color: "#AA64C2D4"

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton | Qt.RightButton

                onPressed: resetFocus()
            }

            Row {
                anchors.fill: parent
                anchors.margins: 2
                Text {
                    width: 100
                    height: parent.height
                    text: "fps: " + Fps.toFixed(2) + "\nms/f: " + (1000. / Fps).toFixed(2)
                }
                Text {
                    width: 150
                    height: parent.height
                    text: "Num triangles: " + NumTriangles + "\nNum draw calls: " + NumDrawCalls
                }
                Column {
                    width: 100
                    Rectangle {
                        color: "white"
                        width: parent.width
                        height: 18
                        TextInput {
                            anchors.fill: parent
                            verticalAlignment: TextEdit.AlignVCenter
                            validator: IntValidator {}

                            onAccepted: Game.generateMap(text)
                        }
                    }
                }
            }
        }
    }
}
