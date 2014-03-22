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

#ifndef MISCUTILS_H
#define MISCUTILS_H

#include <QVector3D>
#include <QUrl>

class MiscUtils
{
public:
    /**
     * position must be normalized
     */
    static QVector3D mapSphereToCube(const QVector3D &position);

    static QString shaderCode(const QString &filename, const QString &shader);
};

class FileFinder
{
public:
    enum class Type {
        Texture,
        Shader,
        Qml
    };

    static QString findFile(Type type, const QString &name);
};

class Ray
{
public:
    Ray(const QVector3D &origin, const QVector3D &direction);

    QVector3D origin;
    QVector3D direction;
};

#endif
