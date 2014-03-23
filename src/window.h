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

#ifndef WINDOW_H
#define WINDOW_H

#include <QQuickView>
#include <QElapsedTimer>
#include <QMatrix4x4>
#include <QQuaternion>

class Terrain;

class Window : public QQuickView
{
    Q_OBJECT
public:
    Window();
    ~Window();

    void renderNow();

    void updateView();
    double perSecond(double n) const;

    void sync();
    void updateUi();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    Terrain *m_terrain;
    QElapsedTimer m_timer;
    QMatrix4x4 m_projection;
    QMatrix4x4 m_view;
    QMatrix4x4 m_world;
    bool m_mouseDown;
    QPointF m_mousePos;
    unsigned int m_frameTime;
    QList<int> m_keysPressed;
    double m_speed;
    bool m_needsUpdate;

    double m_fps;
    unsigned int m_times[20];
    unsigned int m_curTimeId;
    int m_numDrawCalls;
    int m_numTriangles;

    void buildView();

    struct {
        QQuaternion orientation;
        QVector3D pos;
        double distance;
        QQuaternion rotation;
    } m_camera;
};

#endif
