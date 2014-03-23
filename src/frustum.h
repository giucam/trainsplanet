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

#ifndef FRUSTUM_H
#define FRUSTUM_H

#include <QMatrix4x4>

class Plane
{
public:
    Plane() {}
    Plane(double a, double b, double c, double d);
    Plane(const QVector4D &coeffs);
    Plane(const QVector3D &pos, const QVector3D &normal);

    double distancePoint(const QVector3D &pos) const;

private:
    QVector4D m_coeffs;
};


class Frustum
{
public:
    Frustum(const QMatrix4x4 &view, const QMatrix4x4 &proj);

    bool testSphere(const QVector3D &pos, double radius) const;

private:
    QMatrix4x4 m_view;
    QMatrix4x4 m_mvp;
    Plane m_nearPlane;
    Plane m_farPlane;
    Plane m_rightPlane;
    Plane m_topPlane;
    Plane m_leftPlane;
    Plane m_bottomPlane;
};

#endif
