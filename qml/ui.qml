/*
 * Copyright 2014 Giulio Camuffo <giuliocamuffo@gmail.com>
 *
 * This file is part of TrainsPlanet
 *
 * TrainsPlanet is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TrainsPlanet is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TrainsPlanet.  If not, see <http://www.gnu.org/licenses/>.
 */

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
                spacing: 5
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
                Button {
                    width: 100
                    height: 20
                    text: Game.paused ? "Run" : "Pause"
                    onClicked: Game.paused = !Game.paused
                }
            }
        }
    }
}
