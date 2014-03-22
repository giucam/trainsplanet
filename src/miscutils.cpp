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

#include <assert.h>

#include <QFile>
#include <QString>
#include <QRegularExpression>
#include <QDebug>
#include <qmath.h>

#include "miscutils.h"

/*
 * Many many thanks to petrocket for this function!
 * http://stackoverflow.com/questions/2656899/mapping-a-sphere-to-a-cube
 */
QVector3D MiscUtils::mapSphereToCube(const QVector3D &pos)
{
    QVector3D position = pos;
    double x,y,z;
    x = position.x();
    y = position.y();
    z = position.z();

    double fx, fy, fz;
    fx = fabsf(x);
    fy = fabsf(y);
    fz = fabsf(z);

    const double inverseSqrt2 = 0.70710676908493042;

    if (fy >= fx && fy >= fz) {
        double a2 = x * x * 2.0;
        double b2 = z * z * 2.0;
        double inner = -a2 + b2 -3;
        double innersqrt = -sqrtf(qMax((inner * inner) - 12.0 * a2, 0.));

        if(x == 0.0 || x == -0.0) {
            position[0] = 0.0;
        }
        else {
            position[0] = sqrtf(qMax(innersqrt + a2 - b2 + 3.0, 0.)) * inverseSqrt2;
        }

        if(z == 0.0 || z == -0.0) {
            position[2] = 0.0;
        }
        else {
            position[2] = sqrtf(qMax(innersqrt - a2 + b2 + 3.0, 0.)) * inverseSqrt2;
        }

        if(position.x() > 1.0) position[0] = 1.0;
        if(position.z() > 1.0) position[2] = 1.0;

        if(x < 0) position[0] = -position.x();
        if(z < 0) position[2] = -position.z();

        if (y > 0) {
            // top face
            position[1] = 1.0;
        }
        else {
            // bottom face
            position[1] = -1.0;
        }
    }
    else if (fx >= fy && fx >= fz) {
        double a2 = y * y * 2.0;
        double b2 = z * z * 2.0;
        double inner = -a2 + b2 -3;
        double innersqrt = -sqrtf(qMax((inner * inner) - 12.0 * a2, 0.));

        if(y == 0.0 || y == -0.0) {
            position[1] = 0.0;
        }
        else {
            position[1] = sqrtf(qMax(innersqrt + a2 - b2 + 3.0, 0.)) * inverseSqrt2;
        }

        if(z == 0.0 || z == -0.0) {
            position[2] = 0.0;
        }
        else {
            position[2] = sqrtf(qMax(innersqrt - a2 + b2 + 3.0, 0.)) * inverseSqrt2;
        }

        if(position.y() > 1.0) position[1] = 1.0;
        if(position.z() > 1.0) position[2] = 1.0;

        if(y < 0) position[1] = -position.y();
        if(z < 0) position[2] = -position.z();

        if (x > 0) {
            // right face
            position[0] = 1.0;
        }
        else {
            // left face
            position[0] = -1.0;
        }
    }
    else {
        double a2 = x * x * 2.0;
        double b2 = y * y * 2.0;
        double inner = -a2 + b2 -3;
        double innersqrt = -sqrtf(qMax((inner * inner) - 12.0 * a2, 0.));

        if(x == 0.0 || x == -0.0) {
            position[0] = 0.0;
        }
        else {
            position[0] = sqrtf(qMax(innersqrt + a2 - b2 + 3.0, 0.)) * inverseSqrt2;
        }

        if(y == 0.0 || y == -0.0) {
            position[1] = 0.0;
        }
        else {
            position[1] = sqrtf(qMax(innersqrt - a2 + b2 + 3.0, 0.)) * inverseSqrt2;
        }

        if(position.x() > 1.0) position[0] = 1.0;
        if(position.y() > 1.0) position[1] = 1.0;

        if(x < 0) position[0] = -position.x();
        if(y < 0) position[1] = -position.y();

        if (z > 0) {
            // front face
            position[2] = 1.0;
        }
        else {
            // back face
            position[2] = -1.0;
        }
    }
    return position;
}

QString MiscUtils::shaderCode(const QString &fileName, const QString &shaderName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        qWarning() << "MiscUtils::shaderCode: Unable to open file" << fileName;
        return "";
    }

    QString shaderCode;
    QRegularExpression rx("\\[\\s*(\\w*)\\s*\\]");
    QTextStream stream(&file);
    QString line;
    bool inShader = false;
    do {
        line = stream.readLine();
        QRegularExpressionMatch match = rx.match(line);
        if (match.hasMatch()) {
            QString name = match.captured(1);
            if (!inShader && name == shaderName) {
                inShader = true;
            } else if (inShader) {
                break;
            }
        } else if (inShader) {
            shaderCode.append(line);
            shaderCode.append("\n");
        }
    } while (!line.isNull());

    return shaderCode;
}


QString FileFinder::findFile(FileFinder::Type type, const QString &name)
{
    // dumb implementation for now
    switch (type) {
        case Type::Texture:
            return QString("textures/%1").arg(name);
        case Type::Shader:
            return QString("shaders/%1").arg(name);
        case Type::Qml:
            return QString("qml/%1").arg(name);
    }

    return QString();
}

Ray::Ray(const QVector3D &o, const QVector3D &d)
   : origin(o)
   , direction(d.normalized())
{
}
