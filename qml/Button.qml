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
    signal clicked
    property alias text: caption.text

    MouseArea {
        anchors.fill: parent
        onClicked: parent.clicked()
    }
    Rectangle {
        anchors.fill: parent
        border.color: "black"
        color: "transparent"

        Text {
            id: caption
            anchors.centerIn: parent
        }
    }
}
