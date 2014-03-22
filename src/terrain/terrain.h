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

#ifndef TERRAIN_H
#define TERRAIN_H

#include <QObject>
#include <QOpenGLFunctions_3_3_Core>
#include <QVector3D>
#include <QThread>

class QOpenGLShaderProgram;
class QOpenGLBuffer;
class QOpenGLVertexArrayObject;
class QOpenGLTexture;
class QMatrix4x4;

class HeightMap;
class QuadTree;
class QuadTreeNode;
class Frustum;
class DataFetcher;
class GlProgram;

class Terrain : public QObject, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT
public:
    struct Statistics {
        int numDrawCalls;
        int numTriangles;
    };

    Terrain(QObject *parent = nullptr);
    ~Terrain();

    void init();
    void update(const QVector3D &camera, const Frustum &frustum);
    void pick(const QPointF &mouse, const QMatrix4x4 &proj, const QMatrix4x4 &view);
    Statistics render(const QMatrix4x4 &proj, const QMatrix4x4 &view);
    void cycleRenderMode();

private:
    void renderTerrain(const QMatrix4x4 &proj, const QMatrix4x4 &view);
    void renderTerrainWf(const QMatrix4x4 &proj, const QMatrix4x4 &view);
    void renderWater(const QMatrix4x4 &proj, const QMatrix4x4 &view);

    GlProgram *m_program;
    GlProgram *m_wfprogram;
    GlProgram *m_waterProgram;
    QOpenGLVertexArrayObject *m_vao;
    QOpenGLVertexArrayObject *m_wfvao;
    QOpenGLVertexArrayObject *m_waterVao;
    QOpenGLBuffer *m_nodesData;

    QOpenGLTexture *m_grass;
    QOpenGLTexture *m_grass2;

    HeightMap *m_heightMap;
    QuadTree *m_tree[6];
    QList<QuadTreeNode *> m_nodes[6];

    int m_numIndices;
    int m_renderMode;
    QVector3D m_cameraPos;
    float m_heightScale;
    float m_waterLevel;

    Statistics m_statistics;

    QThread m_dataFetcherThread;
    DataFetcher *m_dataFetcher;
};

#endif
