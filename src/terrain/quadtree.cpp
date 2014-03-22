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

#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QOpenGLFunctions>
#include <QRect>
#include <QVector3D>
#include <QDebug>

#include "quadtree.h"
#include "heightmap.h"
#include "miscutils.h"

static const int MESHSIZE = 33;
static const double RANGEMULTIPLIER = 150.;

QuadTreeNode::QuadTreeNode(QuadTreeNode *p, HeightMapChunk *map, int l)
    : tree(p ? p->tree : nullptr)
    , parent(p)
    , chunk(map)
    , lod(l)
    , mapData(nullptr)
    , m_dataFetched(false)
    , buffer(nullptr)
{
    children[0] = nullptr;

    if (tree) {
        tree->m_dataFetcher->fetchNode(this);
    }
}

bool QuadTreeNode::dataFetched() const
{
    return dataUploaded() || m_dataFetched;
}

bool QuadTreeNode::dataUploaded() const
{
    return buffer;
}

void QuadTreeNode::fetchData()
{
    int size = MESHSIZE + 4;
    mapData = new float[size * size];
    chunk->fetchData(MESHSIZE, mapData);
    for (int i = 0; i < size * size; ++i) {
        float h = mapData[i];
        if (i == 0 || h > maxHeight) {
            maxHeight = h;
        }
        if (i == 0 || h < minHeight) {
            minHeight = h;
        }
    }

    static const double M = double(MESHSIZE - 1) / (double)MESHSIZE;
    geometry = QVector4D(chunk->x() * M, chunk->y() * M, chunk->size(), chunk->size());

    double k = 0.3;
    double start = RANGEMULTIPLIER * chunk->size() / (double)MESHSIZE;
    double end = start - start  * k;
    end = end + (start - end) * 0.01f;
    morphData[0] = end / (end - start);
    morphData[1] = 1. / (end - start);

    m_dataFetched = true;
}

void QuadTreeNode::uploadData()
{
    assert(QOpenGLContext::currentContext());
    if (buffer) {
        return;
    }

    texture = new QOpenGLTexture(QOpenGLTexture::TargetRectangle);
    texture->create();
    texture->bind();
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf( GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R16F, MESHSIZE + 4, MESHSIZE + 4, 0, GL_RED, GL_FLOAT, mapData);
    texture->release();

    delete[] mapData;
    mapData = 0;





    {
        QVector<float> data;
        for (int i = 0; i < MESHSIZE+4; ++i) {
            for (int j = 0; j < MESHSIZE+4; ++j) {
//                     data << (i == j ? 0.1 : 0);
                data << 0.;
            }
        }

        overlayTexture = new QOpenGLTexture(QOpenGLTexture::TargetRectangle);
        overlayTexture->create();
        overlayTexture->bind();
        glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf( GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R16F, MESHSIZE + 4, MESHSIZE + 4, 0, GL_RED, GL_FLOAT, data.constData());
        overlayTexture->release();

    }


    QVector<float> data;
    for (int i = 0; i < MESHSIZE; ++i) {
        for (int j = 0; j < MESHSIZE; ++j) {
            data << i << j;
        }
    }

    buffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    buffer->create();
    buffer->bind();
    buffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
    buffer->allocate(data.data(), MESHSIZE * MESHSIZE * 3 * sizeof(float));
    QOpenGLFunctions glFuncs(QOpenGLContext::currentContext());
    buffer->release();


    QVector<unsigned short> in;
    QVector<unsigned short> win;
    for (int i = 0; i < MESHSIZE - 1; ++i) {
        in << 0xffff;
        win << 0xffff;
        int off = i * MESHSIZE;
        for (int j = 0; j < MESHSIZE; ++j) {
            in << off + j << off + MESHSIZE + j;
            if (j < MESHSIZE - 1) {
                win << off + MESHSIZE + j << off + j + 1 << off + j << off + MESHSIZE + j;
            }
        }
        win << off + 2 * MESHSIZE - 1 << off + MESHSIZE - 1;
    }
    mesh.indices = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    mesh.indices->create();
    mesh.indices->bind();
    mesh.indices->setUsagePattern(QOpenGLBuffer::StaticDraw);
    mesh.numIndices = in.size();
    mesh.numPrimitives = (MESHSIZE - 1) * (MESHSIZE - 1) *2;
    mesh.indices->allocate(in.constData(), in.size() * sizeof(short));
    mesh.indices->release();

    mesh.wireframeIndices = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    mesh.wireframeIndices->create();
    mesh.wireframeIndices->bind();
    mesh.wireframeIndices->setUsagePattern(QOpenGLBuffer::StaticDraw);
    mesh.wireframeIndices->allocate(win.constData(), win.size() * sizeof(short));
    mesh.wireframeIndices->release();
    mesh.numWireframeIndices = win.size();






    in.clear();
    win.clear();
    int half = MESHSIZE / 2 + 1;
    for (int i = 0; i < half - 1; ++i) {
        in << 0xffff;
        win << 0xffff;
        int off = i * MESHSIZE;
        for (int j = 0; j < half; ++j) {
            in << off + j << off + MESHSIZE + j;
            if (j < half - 1) {
                win << off + MESHSIZE + j << off + j + 1 << off + j << off + MESHSIZE + j;
            }
        }
        win << off + MESHSIZE + half - 1 << off + half - 1;
    }
    subMesh[0].indices = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    subMesh[0].indices->create();
    subMesh[0].indices->bind();
    subMesh[0].indices->setUsagePattern(QOpenGLBuffer::StaticDraw);
    subMesh[0].indices->allocate(in.constData(), in.size() * sizeof(short));
    subMesh[0].indices->release();
    subMesh[0].numPrimitives = (MESHSIZE - 1) * (MESHSIZE - 1) *2;
    subMesh[0].numIndices = in.size();

    subMesh[0].wireframeIndices = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    subMesh[0].wireframeIndices->create();
    subMesh[0].wireframeIndices->bind();
    subMesh[0].wireframeIndices->setUsagePattern(QOpenGLBuffer::StaticDraw);
    subMesh[0].wireframeIndices->allocate(win.constData(), win.size() * sizeof(short));
    subMesh[0].wireframeIndices->release();
    subMesh[0].numWireframeIndices = win.size();



    in.clear();
    win.clear();
    for (int i = 0; i < half - 1; ++i) {
        in << 0xffff;
        win << 0xffff;
        int off = MESHSIZE / 2 + i * MESHSIZE;
        for (int j = 0; j < half; ++j) {
            in << off + j << off + MESHSIZE + j;
            if (j < half - 1) {
                win << off + MESHSIZE + j << off + j + 1 << off + j << off + MESHSIZE + j;
            }
        }
        win << off + MESHSIZE + half - 1 << off + half - 1;
    }
    subMesh[1].indices = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    subMesh[1].indices->create();
    subMesh[1].indices->bind();
    subMesh[1].indices->setUsagePattern(QOpenGLBuffer::StaticDraw);
    subMesh[1].indices->allocate(in.constData(), in.size() * sizeof(short));
    subMesh[1].indices->release();
    subMesh[1].numPrimitives = (MESHSIZE - 1) * (MESHSIZE - 1) *2;
    subMesh[1].numIndices = in.size();

    subMesh[1].wireframeIndices = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    subMesh[1].wireframeIndices->create();
    subMesh[1].wireframeIndices->bind();
    subMesh[1].wireframeIndices->setUsagePattern(QOpenGLBuffer::StaticDraw);
    subMesh[1].wireframeIndices->allocate(win.constData(), win.size() * sizeof(short));
    subMesh[1].wireframeIndices->release();
    subMesh[1].numWireframeIndices = win.size();

    in.clear();
    win.clear();
    for (int i = 0; i < half - 1; ++i) {
        in << 0xffff;
        win << 0xffff;
        int off = MESHSIZE / 2 + (i + MESHSIZE / 2) * MESHSIZE;
        for (int j = 0; j < half; ++j) {
            in << off + j << off + MESHSIZE + j;
            if (j < half - 1) {
                win << off + MESHSIZE + j << off + j + 1 << off + j << off + MESHSIZE + j;
            }
        }
        win << off + MESHSIZE + half - 1 << off + half - 1;
    }
    subMesh[2].indices = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    subMesh[2].indices->create();
    subMesh[2].indices->bind();
    subMesh[2].indices->setUsagePattern(QOpenGLBuffer::StaticDraw);
    subMesh[2].indices->allocate(in.constData(), in.size() * sizeof(short));
    subMesh[2].indices->release();
    subMesh[2].numPrimitives = (MESHSIZE - 1) * (MESHSIZE - 1) *2;
    subMesh[2].numIndices = in.size();

    subMesh[2].wireframeIndices = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    subMesh[2].wireframeIndices->create();
    subMesh[2].wireframeIndices->bind();
    subMesh[2].wireframeIndices->setUsagePattern(QOpenGLBuffer::StaticDraw);
    subMesh[2].wireframeIndices->allocate(win.constData(), win.size() * sizeof(short));
    subMesh[2].wireframeIndices->release();
    subMesh[2].numWireframeIndices = win.size();

    in.clear();
    win.clear();
    for (int i = 0; i < half - 1; ++i) {
        in << 0xffff;
        win << 0xffff;
        int off = (i + MESHSIZE / 2) * MESHSIZE;
        for (int j = 0; j < half; ++j) {
            in << off + j << off + MESHSIZE + j;
            if (j < half - 1) {
                win << off + MESHSIZE + j << off + j + 1 << off + j << off + MESHSIZE + j;
            }
        }
        win << off + MESHSIZE + half - 1 << off + half - 1;
    }
    subMesh[3].indices = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    subMesh[3].indices->create();
    subMesh[3].indices->bind();
    subMesh[3].indices->setUsagePattern(QOpenGLBuffer::StaticDraw);
    subMesh[3].indices->allocate(in.constData(), in.size() * sizeof(short));
    subMesh[3].indices->release();
    subMesh[3].numPrimitives = (MESHSIZE - 1) * (MESHSIZE - 1) *2;
    subMesh[3].numIndices = in.size();

    subMesh[3].wireframeIndices = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    subMesh[3].wireframeIndices->create();
    subMesh[3].wireframeIndices->bind();
    subMesh[3].wireframeIndices->setUsagePattern(QOpenGLBuffer::StaticDraw);
    subMesh[3].wireframeIndices->allocate(win.constData(), win.size() * sizeof(short));
    subMesh[3].wireframeIndices->release();
    subMesh[3].numWireframeIndices = win.size();

}



QuadTree::QuadTree(DataFetcher *fetcher, HeightMap::Face face, HeightMap *hmap, int lodLevels)
        : m_dataFetcher(fetcher)
        , m_heightMap(hmap)
        , m_lodLevels(lodLevels)
        , m_head(nullptr)
        , m_face(face)
{
    int w = hmap->size();

    m_head = new QuadTreeNode(nullptr, hmap->chunk(m_face, 0, 0, w), 0);
    m_head->tree = this;
    m_head->fetchData();
    m_head->uploadData();
}

inline double SQR(double x) { return x * x; }
bool boxIntersectsSphere(QVector3D min, QVector3D max, QVector3D p, double r)
{
    double dmin = 0;
    for(int i = 0; i < 3; i++ ) {
        if (p[i] < min[i]) {
            dmin += SQR(p[i] - min[i]);
        } else if(p[i] > max[i]) {
            dmin += SQR(p[i] - max[i]);
        }
    }
    return dmin <= SQR(r);
}

bool QuadTreeNode::selectNode(const QVector3D &pos, const Frustum &frustum, QList<QuadTreeNode *> &list)
{
    static const double multiplier = 150.;
    double range = multiplier * chunk->size() / (double)MESHSIZE;

    double M = double(MESHSIZE - 1)/ (double)MESHSIZE;

    QVector3D min(chunk->x()*M, chunk->y()*M, minHeight);
    QVector3D max(chunk->x()*M + chunk->size(), chunk->y()*M + chunk->size(), maxHeight);

    drawParts = 0;
    if (!boxIntersectsSphere(min, max, pos, range)) {
        return false;
    }

//     if (frustumculled) {
//         return true;
//     }

    if (chunk->size() <= MESHSIZE) {
        list << this;

        return true;
    }

    double nextRange = multiplier * chunk->size() / (double)(MESHSIZE * 2);
    if (boxIntersectsSphere(min, max, pos, nextRange)) {
        if (children[0]) {
            bool n= true;
            QList<QuadTreeNode *> l;
            for (int i = 0; i < 4; ++i) {
                QuadTreeNode *child = children[i];
                if (!child->dataUploaded() && child->dataFetched()) {
                    child->uploadData();
                }
                if (!child->dataFetched() || !child->selectNode(pos, frustum, list)) {
                    if (n) list << this;
                    n = false;

                    drawParts |= (1 << i);
                }
            }
            return true;
        } else {
            const int s = chunk->size() / 2;
            children[0] = new QuadTreeNode(this, chunk->map->chunk(chunk->face(), chunk->x(), chunk->y(), s), lod + 1);
            children[1] = new QuadTreeNode(this, chunk->map->chunk(chunk->face(), chunk->x(), chunk->y() + s, s), lod + 1);
            children[2] = new QuadTreeNode(this, chunk->map->chunk(chunk->face(), chunk->x() + s, chunk->y() + s, s), lod + 1);
            children[3] = new QuadTreeNode(this, chunk->map->chunk(chunk->face(), chunk->x() + s, chunk->y(), s), lod + 1);
        }
    }

    list << this;

    return true;
}

bool QuadTreeNode::findNearestPoint(QVector3D &p)
{
    double M = 32./33.;
    QVector3D min(chunk->x()*M, chunk->y()*M, minHeight);
    QVector3D max(chunk->x()*M + chunk->size(), chunk->y()*M + chunk->size(), maxHeight);

    if (boxIntersectsSphere(min, max, p, 1)) {
        return true;
    }

    return false;
}

QList<QuadTreeNode *> QuadTree::findNodes(const QVector3D &p, const Frustum &frustum)
{
    QList<QuadTreeNode *> nodes;

    // We need to account for the deformations the mapping to the sphere does to the
    // nodes. If we map the nodes' AABB to the sphere not only the AABB will not be AA
    // anymore, but it also won't be a box anymore, but some trapezoid. Instead of doing
    // that we map the camera position from the sphere space to the cube space, and we do
    // the same thing in the vertex shader for the morphing.
    // That means that when the camera is over a cube vertex the boundary between the most
    // refined nodes and the less ones on the three faces visible will not form a nice circle
    // but a triangle-like shape, which should not be a problem.
    QVector3D cam = MiscUtils::mapSphereToCube(p.normalized()) * p.length();
    QVector3D pos = m_transform.inverted().map(-cam);

    m_head->selectNode(pos, frustum, nodes);
    if (nodes.isEmpty()) {
        nodes << m_head;
    }

    return nodes;
}

QVector3D QuadTree::findNearestPoint(const QVector3D &point)
{
    QVector3D pos = m_transform.inverted().map(point);

    if (m_head->findNearestPoint(pos)) {
        return m_transform * pos;
    }

    return point;
}
