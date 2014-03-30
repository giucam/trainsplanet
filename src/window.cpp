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

#include <QOpenGLContext>
#include <QOpenGLPaintDevice>
#include <QOpenGLDebugLogger>
#include <QCoreApplication>
#include <QPainter>
#include <QMatrix4x4>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QDebug>
#include <QQmlContext>
#include <qmath.h>

#include "window.h"
#include "frustum.h"
#include "miscutils.h"
#include "terrain/terrain.h"

Window::Window()
      : QQuickView()
      , m_terrain(nullptr)
      , m_mouseDown(false)
      , m_speed(0.01)
      , m_needsUpdate(true)
      , m_generate(false)
      , m_curTimeId(0)
{
    updateUi();

    setSurfaceType(QWindow::OpenGLSurface);
    setClearBeforeRendering(false);
    setPersistentOpenGLContext(true);
    setResizeMode(QQuickView::SizeRootObjectToView);
    setSource(QUrl::fromLocalFile(FileFinder::findFile(FileFinder::Type::Qml, "ui.qml")));

    QSurfaceFormat format = requestedFormat();
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setMajorVersion(3);
    format.setMinorVersion(3);
    format.setDepthBufferSize(24);
    format.setOption(QSurfaceFormat::DebugContext);
    setFormat(format);

    rootContext()->setContextProperty("Game", this);

    resize(1024, 768);

    connect(this, &QQuickWindow::beforeRendering, this, &Window::renderNow, Qt::DirectConnection);
    connect(this, &QQuickWindow::beforeSynchronizing, this, &Window::sync, Qt::DirectConnection);
    connect(this, &QQuickWindow::afterRendering, this, &Window::updateUi);
    connect(this, &QQuickWindow::afterRendering, this, &Window::update);

    show();
}

Window::~Window()
{
    delete m_terrain;
}

void Window::mousePressEvent(QMouseEvent *event)
{
    QQuickView::mousePressEvent(event);
    if (event->isAccepted()) {
        return;
    }

    if (event->button() == Qt::RightButton) {
        m_mouseDown = true;
        m_mousePos = event->localPos();
    }
}

void Window::mouseReleaseEvent(QMouseEvent *event)
{
    QQuickView::mouseReleaseEvent(event);
    m_mouseDown = false;
}

void Window::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_mouseDown) {
        m_terrain->pick(QPointF(event->localPos().x() / 1024, event->localPos().y() / 768), m_projection, m_view);
        return;
    }

    QPointF delta = event->localPos() - m_mousePos;

    QMatrix4x4 mat;
    mat.rotate(m_camera.orientation);

    QVector3D pitchAxis = QVector3D(mat(0, 0), mat(0, 1), mat(0, 2));
    pitchAxis.normalize();

    m_camera.orientation *= QQuaternion::fromAxisAndAngle(pitchAxis, perSecond(delta.y() / 200));
    m_camera.orientation.normalize();

    QVector3D yawAxis = QVector3D(mat(1, 0), mat(1, 1), mat(1, 2));
    yawAxis.normalize();

    m_camera.orientation *= QQuaternion::fromAxisAndAngle(yawAxis, perSecond(delta.x() / 200));
    m_camera.orientation.normalize();


    m_needsUpdate = true;

    m_mousePos = event->localPos();
}

void Window::wheelEvent(QWheelEvent *event)
{
    double d = perSecond((double)event->angleDelta().y() * m_speed / 5);

    QVector3D vec(m_view(2, 0), m_view(2, 1), m_view(2, 2));
    m_camera.pos += vec * d;

    m_camera.distance -= d;

    m_needsUpdate = true;
}

void Window::keyPressEvent(QKeyEvent *event)
{
    QQuickView::keyPressEvent(event);
    if (event->isAccepted()) {
        return;
    }

    if (event->key() == Qt::Key_M) {
        m_terrain->cycleRenderMode();
        return;
    }

    m_keysPressed << event->key();
    if (event->key() == Qt::Key_PageUp) {
        m_speed *= 2;
    } else if (event->key() == Qt::Key_PageDown) {
        m_speed /= 2;
    } else if (event->key() == Qt::Key_P) {
        qDebug() << m_camera.orientation << m_camera.rotation << m_camera.distance;
    }
}

void Window::keyReleaseEvent(QKeyEvent *event)
{
    QQuickView::keyReleaseEvent(event);

    m_keysPressed.removeOne(event->key());
}

void Window::updateView()
{
    double amount = perSecond(m_speed * (m_keysPressed.contains(Qt::Key_Shift) ? 5 : 1));
//     if (m_keysPressed.contains(Qt::Key_W)) {
//         QVector3D vec(m_view(2, 0), m_view(2, 1), m_view(2, 2));
//         m_camera.pos += vec * amount;
//     }
//     if (m_keysPressed.contains(Qt::Key_S)) {
//         QVector3D vec(m_view(2, 0), m_view(2, 1), m_view(2, 2));
//         qDebug()<<vec<<amount<<m_view;
//         m_camera.pos -= vec * amount;
//     }
//     if (m_keysPressed.contains(Qt::Key_A)) {
//         QVector3D vec(m_view(0, 0), m_view(0, 1), m_view(0, 2));
//         m_camera.pos += vec * amount;
//     }
//     if (m_keysPressed.contains(Qt::Key_D)) {
//         QVector3D vec(m_view(0, 0), m_view(0, 1), m_view(0, 2));
//         m_camera.pos -= vec * amount;
//     }
//     if (m_keysPressed.contains(Qt::Key_R)) {
//         QVector3D vec(m_view(1, 0), m_view(1, 1), m_view(1, 2));
// //         QVector3D vec(0, 0, 1);
//         m_camera.pos -= vec * amount;
//     }
//     if (m_keysPressed.contains(Qt::Key_F)) {
// //         QVector3D vec(0, 0, 1);
//         QVector3D vec(m_view(1, 0), m_view(1, 1), m_view(1, 2));
//         m_camera.pos += vec * amount;
//     }




    QQuaternion quat;

    if (m_keysPressed.contains(Qt::Key_W)) {
        QVector3D vec(m_view(0, 0), m_view(0, 1), m_view(0, 2));
        quat *= QQuaternion::fromAxisAndAngle(vec, amount);
        m_needsUpdate = true;
    }
    if (m_keysPressed.contains(Qt::Key_S)) {
        QVector3D vec(m_view(0, 0), m_view(0, 1), m_view(0, 2));
        quat *= QQuaternion::fromAxisAndAngle(vec, -amount);
        m_needsUpdate = true;
    }
    if (m_keysPressed.contains(Qt::Key_A)) {
        QVector3D vec(m_view(1, 0), m_view(1, 1), m_view(1, 2));
        quat *= QQuaternion::fromAxisAndAngle(vec, amount);
        m_needsUpdate = true;
    }
    if (m_keysPressed.contains(Qt::Key_D)) {
        QVector3D vec(m_view(1, 0), m_view(1, 1), m_view(1, 2));
        quat *= QQuaternion::fromAxisAndAngle(vec, -amount);
        m_needsUpdate = true;
    }
    if (m_keysPressed.contains(Qt::Key_Q)) {
        QVector3D vec(m_view(2, 0), m_view(2, 1), m_view(2, 2));
        quat *= QQuaternion::fromAxisAndAngle(vec, -amount);
        m_needsUpdate = true;
    }
    if (m_keysPressed.contains(Qt::Key_E)) {
        QVector3D vec(m_view(2, 0), m_view(2, 1), m_view(2, 2));
        quat *= QQuaternion::fromAxisAndAngle(vec, amount);
        m_needsUpdate = true;
    }


    m_camera.rotation *= quat;
    m_camera.rotation.normalize();

    buildView();
}

void Window::buildView()
{
    m_view.setToIdentity();
    m_view.rotate(m_camera.orientation);
//     m_view.translate(m_camera.pos);
//     m_view.translate(QVector3D(m_camera.distance, m_camera.distance, m_camera.distance));
    m_view.translate(QVector3D(0,0,-m_camera.distance));
    m_view.rotate(m_camera.rotation);
}

double Window::perSecond(double n) const
{
    return n * m_frameTime;
}

void Window::generateMap(int seed)
{
    QMutexLocker lock(&m_mutex);
    m_generate = true;
    m_mapSeed = seed;
}

void Window::sync()
{
    QMutexLocker lock(&m_mutex);

    if (!m_terrain) {
        m_terrain = new Terrain;
        m_timer.start();

        QOpenGLDebugLogger *logger = new QOpenGLDebugLogger;
        logger->initialize();
        connect(logger, &QOpenGLDebugLogger::messageLogged, [](const QOpenGLDebugMessage &debugMessage) {
            qDebug() << debugMessage.message();
        });
        logger->startLogging(QOpenGLDebugLogger::SynchronousLogging);

        m_terrain->init();

        qreal aspect = qreal(size().width()) / qreal(size().height());
        const qreal zNear = 1., zFar = 7000.0, fov = 60.0;
        m_projection.perspective(fov, aspect, zNear, zFar);

        m_camera.distance = 4800;
//         m_camera.orientation = QQuaternion(0.354394, -0.24355, 0.825291, -0.366037);
//         m_camera.rotation = QQuaternion(0.593273, 0.507288, 0.114765, -0.614423);
//         m_camera.rotation = QQuaternion(0, 0, 1, 45);

        glClearColor(1, 1, 1, 1);
    }

    if (m_generate) {
        m_terrain->generateMap(m_mapSeed);
        m_needsUpdate = true;
        m_generate = false;
    }

    updateView();

    m_frameTime = m_timer.restart();

    m_times[m_curTimeId] = m_frameTime;
    if (++m_curTimeId >= 20) {
        m_curTimeId = 0;
    }

    unsigned int totTime = 0;
    for (unsigned int t: m_times) {
        totTime += t;
    }
    m_fps = 20 * 1000. / (double)totTime;
}

void Window::updateUi()
{
    rootContext()->setContextProperty("Fps", m_fps);
    rootContext()->setContextProperty("NumDrawCalls", m_numDrawCalls);
    rootContext()->setContextProperty("NumTriangles", m_numTriangles);
}

void Window::renderNow()
{


    glDepthMask(GL_TRUE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
//     projection.rotate(-80, QVector3D(1, 0, 0));
//     projection.rotate(30, QVector3D(0, 0, 1));
// qDebug()<<m_camera.orientation<<m_camera.pos;

//     projection.translate(-128, 138, -80);

    if (m_needsUpdate) {
        m_needsUpdate = m_terrain->update(-(m_view.inverted() * QVector3D(0,0,0)), Frustum(m_view, m_projection));
    }
    Terrain::Statistics stats = m_terrain->render(m_projection, m_view);
    m_numDrawCalls = stats.numDrawCalls;
    m_numTriangles = stats.numTriangles;
//     m_device->setSize(size());
//     QPainter painter(m_device);
//


//     qDebug()<<m_camera.orientation<<m_camera.rotation<<m_camera.distance;
//     qDebug()<<fps;
//     painter.setPen(Qt::white);
//     painter.drawText(QPoint(10, 10), QString::number(fps));

//     m_context->swapBuffers(this);
//     renderLater();



    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ZERO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

//     resetOpenGLState();
}
