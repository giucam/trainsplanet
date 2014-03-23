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

#include <qmath.h>

#include "frustum.h"

Plane::Plane(const QVector3D &p, const QVector3D &n)
{
    m_coeffs = QVector4D(n.x(), n.y(), n.z(), QVector3D::dotProduct(-n, p));
}

Plane::Plane(double a, double b, double c, double d)
{
    QVector3D n(a, b, c);
    double l = n.length();
    m_coeffs = QVector4D(a / l, b / l, c / l, d / l);
}

Plane::Plane(const QVector4D &coeffs)
     : Plane(coeffs.x(), coeffs.y(), coeffs.z(), coeffs.w())
{
}

double Plane::distancePoint(const QVector3D &p) const
{
    return m_coeffs.x() * p.x() + m_coeffs.y() * p.y() + m_coeffs.z() * p.z() + m_coeffs.w();
}

Frustum::Frustum(const QMatrix4x4 &view, const QMatrix4x4 &proj)
{
    QMatrix4x4 mvp = proj * view;

    QVector4D r0 = mvp.row(0);
    QVector4D r1 = mvp.row(1);
    QVector4D r2 = mvp.row(2);
    QVector4D r3 = mvp.row(3);

    m_nearPlane = Plane(r2 + r3);
    m_farPlane = Plane(r3 - r2);
    m_rightPlane = Plane(r3 - r0);
    m_leftPlane = Plane(r3 + r0);
    m_topPlane = Plane(r3 - r1);
    m_bottomPlane = Plane(r3 + r1);
}

bool Frustum::testSphere(const QVector3D &p, double radius) const
{
    if (m_nearPlane.distancePoint(p) < -radius) {
        return false;
    }
    if (m_farPlane.distancePoint(p) < -radius) {
        return false;
    }
    if (m_rightPlane.distancePoint(p) < -radius) {
        return false;
    }
    if (m_topPlane.distancePoint(p) < -radius) {
        return false;
    }
    if (m_bottomPlane.distancePoint(p) < -radius) {
        return false;
    }
    if (m_leftPlane.distancePoint(p) < -radius) {
        return false;
    }

    return true;
}
