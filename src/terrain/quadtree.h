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

#ifndef QUADTREE_H
#define QUADTREE_H

#include <QList>
#include <QMatrix4x4>

#include "heightmap.h"
#include "datafetcher.h"

class QRect;
class QOpenGLBuffer;
class QOpenGLTexture;
class QVector3D;

class HeightMap;
class HeightMapChunk;
class QuadTree;
class Frustum;

class QuadTreeNode {
public:
    QuadTreeNode(QuadTreeNode *p, HeightMapChunk *map, int l);

    void fetchData();
    void uploadData();
    bool selectNode(const QVector3D &pos, const Frustum &frustum, QList<QuadTreeNode *> &list);
    bool findNearestPoint(QVector3D &p);

    bool dataFetched() const;
    bool dataUploaded() const;

    QuadTree *tree;
    QuadTreeNode *parent;
    QuadTreeNode *children[4];
    HeightMapChunk *chunk;
    QVector4D geometry;
    int maxHeight;
    int minHeight;
    int lod;
    float *mapData;
    bool m_dataFetched;

    float morphData[2];

    enum class Parts {
        Entire = 0,
        TopLeft = 2,
        TopRight = 8,
        BottomLeft = 1,
        BottomRight = 4
    };
    int drawParts;

    struct Mesh {
        QOpenGLBuffer *indices;
        QOpenGLBuffer *wireframeIndices;
        int numIndices;
        int numWireframeIndices;
        int numPrimitives;
    };

    Mesh mesh;
    Mesh subMesh[4];
    QOpenGLBuffer *buffer;
    QOpenGLTexture *texture;
    QOpenGLTexture *overlayTexture;
};


class QuadTree
{
public:
    QuadTree(DataFetcher *fetcher, HeightMap::Face face, HeightMap *heightMap, int lodLevels);

    QList<QuadTreeNode *> findNodes(const QVector3D &pos, const Frustum &frustum);
    QVector3D findNearestPoint(const QVector3D &p);
// private:

    DataFetcher *m_dataFetcher;
    HeightMap *m_heightMap;
    int m_lodLevels;
    QuadTreeNode *m_head;
    QMatrix4x4 m_transform;
    HeightMap::Face m_face;
};

#endif
