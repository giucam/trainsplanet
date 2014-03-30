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

#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QDebug>

#include "heightmap.h"
#include "quadtree.h"
#include "terrain.h"
#include "miscutils.h"
#include "datafetcher.h"
#include "gl/glprogram.h"

static const int FACESIZE = 8192;
static const int MESHSIZE = 33;

Terrain::Terrain(QObject *parent)
        : QObject(parent)
        , m_heightMap(nullptr)
        , m_renderMode(2)
{
    m_heightScale = 50;
    m_waterLevel = m_heightScale * 0.25;

    m_dataFetcher = new DataFetcher(this);
    m_dataFetcher->moveToThread(&m_dataFetcherThread);

    memset(m_tree, 0, sizeof(m_tree));

    int seed = 2;//rand();
    generateMap(seed);
}

void Terrain::generateMap(int seed)
{
    m_dataFetcherThread.quit();
    m_dataFetcherThread.wait();
    m_dataFetcherThread.start();

    for (int i = 0; i < 6; ++i) {
        delete m_tree[i];
    }

    delete m_heightMap;
    m_heightMap = new HeightMap(new RandomGenerator(FACESIZE, m_heightScale, seed));

    m_tree[0] = new QuadTree(m_dataFetcher, HeightMap::Face::Top, m_heightMap, 2);
    m_tree[1] = new QuadTree(m_dataFetcher, HeightMap::Face::Front, m_heightMap, 2);
    m_tree[2] = new QuadTree(m_dataFetcher, HeightMap::Face::Right, m_heightMap, 2);
    m_tree[3] = new QuadTree(m_dataFetcher, HeightMap::Face::Left, m_heightMap, 2);
    m_tree[4] = new QuadTree(m_dataFetcher, HeightMap::Face::Back, m_heightMap, 2);
    m_tree[5] = new QuadTree(m_dataFetcher, HeightMap::Face::Bottom, m_heightMap, 2);

    float d = (FACESIZE / 2) * double(MESHSIZE - 1) / (float)MESHSIZE;

    QMatrix4x4 model;
    model.translate(-d, -d, d);
    m_tree[0]->m_transform = model;

    model.setToIdentity();
    model.rotate(90, QVector3D(1, 0, 0));
    model.translate(-d, -d, d);
    m_tree[1]->m_transform = model;

    model.setToIdentity();
    model.rotate(90, QVector3D(1, 0, 0));
    model.rotate(90, QVector3D(0, 1, 0));
    model.translate(-d, -d, d);
    m_tree[2]->m_transform = model;

    model.setToIdentity();
    model.rotate(90, QVector3D(1, 0, 0));
    model.rotate(270, QVector3D(0, 1, 0));
    model.translate(-d, -d, d);
    m_tree[3]->m_transform = model;

    model.setToIdentity();
    model.rotate(90, QVector3D(1, 0, 0));
    model.rotate(180, QVector3D(0, 1, 0));
    model.translate(-d, -d, d);
    m_tree[4]->m_transform = model;

    model.setToIdentity();
    model.rotate(180, QVector3D(0, 1, 0));
    model.rotate(-90, QVector3D(0, 0, 1));
    model.translate(-d, -d, d);
    m_tree[5]->m_transform = model;
}

Terrain::~Terrain()
{
    m_dataFetcherThread.quit();
    m_dataFetcherThread.wait();

    for (int i = 0; i < 6; ++i) {
        delete m_tree[i];
    }
    delete m_heightMap;
}

void Terrain::init()
{
    assert(initializeOpenGLFunctions());

    m_program = new GlProgram("terrain.glsl", this);
    m_program->setVertexShader("vertex");
    m_program->setFragmentShader("fragment");
    m_program->link();
    m_program->bind();
    m_program->setUniformValue("meshSize", MESHSIZE);
    m_program->setUniformValue("faceSize", FACESIZE);

    m_vao = new QOpenGLVertexArrayObject(this);
    m_vao->create();
    m_vao->bind();

    m_grass = new QOpenGLTexture(QImage(FileFinder::findFile(FileFinder::Type::Texture, "grass2.jpg")));
    glActiveTexture(GL_TEXTURE1);
    m_grass->bind();
    glUniform1i(m_program->uniformLocation("grass"), 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    m_grass2 = new QOpenGLTexture(QImage(FileFinder::findFile(FileFinder::Type::Texture, "grass.jpg")));
    glActiveTexture(GL_TEXTURE2);
    m_grass2->bind();
    glUniform1i(m_program->uniformLocation("grass2"), 2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    m_vao->release();



    m_wfprogram = new GlProgram("terrain.glsl", this);
    m_wfprogram->setVertexShader("vertex");
    m_wfprogram->setFragmentShader("fragment_wf");
    m_wfprogram->link();
    m_wfprogram->bind();
    m_wfprogram->setUniformValue("meshSize", MESHSIZE);
    m_wfprogram->setUniformValue("faceSize", FACESIZE);

    m_wfvao = new QOpenGLVertexArrayObject(this);
    m_wfvao->create();
    m_wfvao->bind();
    m_wfvao->release();

    m_wfprogram->release();

    m_waterProgram = new GlProgram("water.glsl", this);
    m_waterProgram->setVertexShader("vertex");
    m_waterProgram->setFragmentShader("fragment");
    m_waterProgram->link();
    m_waterProgram->bind();
    m_waterProgram->setUniformValue("meshSize", MESHSIZE);
    m_waterProgram->setUniformValue("faceSize", FACESIZE);

    m_waterVao = new QOpenGLVertexArrayObject(this);
    m_waterVao->create();
    m_waterVao->bind();

    m_waterProgram->setUniformValue("waterLevel", m_waterLevel);

    m_waterVao->release();

    glUseProgram(0);
}

void Terrain::cycleRenderMode()
{
    m_renderMode = (m_renderMode + 1) % 3 + 1;
}

QVector3D unproject(const QPointF &winPos, double winZ, const QMatrix4x4 &modelView, const QMatrix4x4 &proj)
{
    QMatrix4x4 transformMatrix = (proj * modelView).inverted();

    // Transformation of normalized coordinates (-1 to 1).
    QVector4D inVector(winPos.x() * 2. - 1.,
                       (1. - winPos.y()) * 2. - 1.,
                       2. * winZ - 1., 1.0);

    QVector4D out = transformMatrix * inVector;
    if (out.w() != 0.0f) {
        out[3] = 1.0f / out.w();
    }

    /* calculate output */
    QVector3D result;
    result[0] = out.x() * out.w();
    result[1] = out.y() * out.w();
    result[2] = out.z() * out.w();

    return result;
};

static QVector3D pickPos;

static bool intersect(const Ray &ray, QVector3D *pos)
{
    //Compute A, B and C coefficients
    double a = QVector3D::dotProduct(ray.direction, ray.direction);
    double b = 2 * QVector3D::dotProduct(ray.direction, ray.origin);
    const double r = FACESIZE / 2. * double(MESHSIZE - 1) / (double)MESHSIZE;
    double c = QVector3D::dotProduct(ray.origin, ray.origin) - (r * r);

    //Find discriminant
    double disc = b * b - 4 * a * c;

    // if discriminant is negative there are no real roots, so return
    // false as ray misses sphere
    if (disc < 0) {
        return false;
    }

    // compute q as described above
    double distSqrt = sqrt(disc);
    double q;
    if (b < 0.) {
        q = (-b - distSqrt) / 2.0;
    } else {
        q = (-b + distSqrt) / 2.0;
    }

    // compute t0 and t1
    double t0 = q / a;
    double t1 = c / q;

    // make sure t0 is smaller than t1
    if (t0 > t1) {
        // if t0 is bigger than t1 swap them around
        double temp = t0;
        t0 = t1;
        t1 = temp;
    }

    // if t1 is less than zero, the object is in the ray's negative direction
    // and consequently the ray misses the sphere
    if (t1 < 0) {
        return false;
    }

    // We know we're not inside the sphere so the intersection will always be at t0,
    // and never at t1.
    *pos = ray.origin + ray.direction * t0;
    return true;
}

void Terrain::pick(const QPointF &mouse, const QMatrix4x4 &proj, const QMatrix4x4 &view)
{
    QVector3D result0 = unproject(mouse, 0, view, proj);
    QVector3D result1 = unproject(mouse, 1, view, proj);

    Ray ray(result0, result1 - result0);
//     pickPos = result0;
    intersect(ray, &pickPos);
    pickPos = MiscUtils::mapSphereToCube(pickPos.normalized()) * pickPos.length();
    QVector3D p = pickPos;
    for (QuadTree *tree: m_tree) {
//         pickPos = tree->findNearestPoint(pickPos);
    }
// qDebug()<<p<<pickPos<<p.lengthSquared();
}

bool Terrain::update(const QVector3D &camera, const Frustum &frustum)
{
    bool again = false;
    for (int i = 0; i < 6; ++i) {
        m_nodes[i] = m_tree[i]->findNodes(camera, frustum, again);
    }

    m_cameraPos = MiscUtils::mapSphereToCube(camera.normalized()) * camera.length();
    return again;
}

inline void renderMesh(QuadTreeNode::Mesh *mesh, Terrain::Statistics &stats)
{
    mesh->indices->bind();
    glDrawElements(GL_TRIANGLE_STRIP, mesh->numIndices, GL_UNSIGNED_SHORT, 0);
    mesh->indices->release();
    ++stats.numDrawCalls;
    stats.numTriangles += mesh->numPrimitives;
}
inline void renderWireFrameMesh(QuadTreeNode::Mesh *mesh, Terrain::Statistics &stats)
{
    mesh->wireframeIndices->bind();
    glDrawElements(GL_LINE_STRIP, mesh->numWireframeIndices, GL_UNSIGNED_SHORT, 0);
    mesh->wireframeIndices->release();
    ++stats.numDrawCalls;
}

void Terrain::renderTerrain(const QMatrix4x4 &proj, const QMatrix4x4 &view)
{
    m_program->bind();
    m_program->setUniformValue("view", view);
    m_program->setUniformValue("cameraPos", -m_cameraPos);
    m_program->setUniformValue("cursorPos", pickPos);
    m_program->setUniformValue("proj", proj);

    static const int vertexLoc = m_program->attributeLocation("vertex");
    static const int pvmLoc = m_program->uniformLocation("pvm");
    static const int modelLoc = m_program->uniformLocation("model");
    static const int nodeDataLoc = m_program->uniformLocation("nodeData");
    static const int morphDataLoc = m_program->uniformLocation("morphData");

    QOpenGLVertexArrayObject::Binder vao(m_vao);

    m_program->enableAttributeArray(vertexLoc);

    for (int face = 0; face < 6; ++face) {
        const QMatrix4x4 &model = m_tree[face]->m_transform;
        QMatrix4x4 modelView = view * model;
        QMatrix4x4 pvm = proj * modelView;

        m_program->setUniformValue(pvmLoc, pvm);
        m_program->setUniformValue(modelLoc, model);

        for (QuadTreeNode *node: m_nodes[face]) {
            m_program->setUniformValue(nodeDataLoc, node->geometry);
            node->buffer->bind();
            m_program->setAttributeBuffer(vertexLoc, GL_FLOAT, 0, 2);

            glActiveTexture(GL_TEXTURE0);
            glUniform1i(m_program->uniformLocation("heightmap"), 0);
            node->texture->bind();

            glActiveTexture(GL_TEXTURE3);
            glUniform1i(m_program->uniformLocation("overlay"), 3);
            node->overlayTexture->bind();

            m_program->setUniformValue(morphDataLoc, node->morphData[0], node->morphData[1]);

            if (node->drawParts == 0) {
                renderMesh(&node->mesh, m_statistics);
            } else {
                if (node->drawParts & (int)QuadTreeNode::Parts::BottomLeft) {
                    renderMesh(&node->subMesh[0], m_statistics);
                }
                if (node->drawParts & (int)QuadTreeNode::Parts::TopLeft) {
                    renderMesh(&node->subMesh[1], m_statistics);
                }
                if (node->drawParts & (int)QuadTreeNode::Parts::BottomRight) {
                    renderMesh(&node->subMesh[2], m_statistics);
                }
                if (node->drawParts & (int)QuadTreeNode::Parts::TopRight) {
                    renderMesh(&node->subMesh[3], m_statistics);
                }
            }

            node->texture->release();
            node->overlayTexture->release();
            node->buffer->release();
        }
    }

    m_grass2->release();
    m_grass->release();
    m_program->release();
}

void Terrain::renderTerrainWf(const QMatrix4x4 &proj, const QMatrix4x4 &view)
{
    m_wfprogram->bind();
    m_wfprogram->setUniformValue("proj", proj);
    m_wfprogram->setUniformValue("view", view);
    m_wfprogram->setUniformValue("cameraPos", -m_cameraPos);
    m_wfprogram->setUniformValue("cursorPos", pickPos);

    static const int vertexLoc = m_wfprogram->attributeLocation("vertex");
    static const int pvmLoc = m_wfprogram->uniformLocation("pvm");
    static const int modelLoc = m_wfprogram->uniformLocation("model");
    static const int nodeDataLoc = m_wfprogram->uniformLocation("nodeData");
    static const int morphDataLoc = m_wfprogram->uniformLocation("morphData");

    QOpenGLVertexArrayObject::Binder vao(m_wfvao);

    m_wfprogram->enableAttributeArray(vertexLoc);

    for (int face = 0; face < 6; ++face) {
        const QMatrix4x4 &model = m_tree[face]->m_transform;
        QMatrix4x4 modelView = view * model;
        QMatrix4x4 pvm = proj * modelView;

        m_wfprogram->setUniformValue(pvmLoc, pvm);
        m_wfprogram->setUniformValue(modelLoc, model);

        for (QuadTreeNode *node: m_nodes[face]) {
            m_wfprogram->setUniformValue(nodeDataLoc, node->geometry);
            node->buffer->bind();
            m_wfprogram->setAttributeBuffer(vertexLoc, GL_FLOAT, 0, 2);

            glActiveTexture(GL_TEXTURE0);
            glUniform1i(m_wfprogram->uniformLocation("heightmap"), 0);
            node->texture->bind();

            glActiveTexture(GL_TEXTURE3);
            glUniform1i(m_wfprogram->uniformLocation("overlay"), 3);
            node->overlayTexture->bind();

            m_wfprogram->setUniformValue(morphDataLoc, node->morphData[0], node->morphData[1]);

            if (node->drawParts == 0) {
                renderWireFrameMesh(&node->mesh, m_statistics);
            } else {
                if (node->drawParts & (int)QuadTreeNode::Parts::BottomLeft) {
                    renderWireFrameMesh(&node->subMesh[0], m_statistics);
                }
                if (node->drawParts & (int)QuadTreeNode::Parts::TopLeft) {
                    renderWireFrameMesh(&node->subMesh[1], m_statistics);
                }
                if (node->drawParts & (int)QuadTreeNode::Parts::BottomRight) {
                    renderWireFrameMesh(&node->subMesh[2], m_statistics);
                }
                if (node->drawParts & (int)QuadTreeNode::Parts::TopRight) {
                    renderWireFrameMesh(&node->subMesh[3], m_statistics);
                }
            }

            node->texture->release();
            node->buffer->release();
        }
    }

    m_wfprogram->release();
}

void Terrain::renderWater(const QMatrix4x4 &proj, const QMatrix4x4 &view)
{
    m_waterProgram->bind();
    m_waterProgram->setUniformValue("pv", proj * view);
    QOpenGLVertexArrayObject::Binder vao(m_waterVao);

    static const int vertexLoc = m_waterProgram->attributeLocation("vertex");
    static const int modelLoc = m_waterProgram->uniformLocation("model");
    static const int nodeDataLoc = m_waterProgram->uniformLocation("nodeData");
    static const int morphDataLoc = m_waterProgram->uniformLocation("morphData");
    m_waterProgram->enableAttributeArray(vertexLoc);

    for (int face = 0; face < 6; ++face) {
        const QMatrix4x4 &model = m_tree[face]->m_transform;
        m_waterProgram->setUniformValue(modelLoc, model);

        for (QuadTreeNode *node: m_nodes[face]) {
            m_waterProgram->setUniformValue(nodeDataLoc, node->geometry);
            node->buffer->bind();
            m_program->setAttributeBuffer(vertexLoc, GL_FLOAT, 0, 2);

            m_waterProgram->setUniformValue(morphDataLoc, node->morphData[0], node->morphData[1]);

            if (node->drawParts == 0) {
                renderMesh(&node->mesh, m_statistics);
            } else {
                if (node->drawParts & (int)QuadTreeNode::Parts::BottomLeft) {
                    renderMesh(&node->subMesh[0], m_statistics);
                }
                if (node->drawParts & (int)QuadTreeNode::Parts::TopLeft) {
                    renderMesh(&node->subMesh[1], m_statistics);
                }
                if (node->drawParts & (int)QuadTreeNode::Parts::BottomRight) {
                    renderMesh(&node->subMesh[2], m_statistics);
                }
                if (node->drawParts & (int)QuadTreeNode::Parts::TopRight) {
                    renderMesh(&node->subMesh[3], m_statistics);
                }
            }

            node->buffer->release();
        }
    }

    m_waterProgram->release();
}

Terrain::Statistics Terrain::render(const QMatrix4x4 &proj, const QMatrix4x4 &view)
{
    m_statistics.numDrawCalls = 0;
    m_statistics.numTriangles = 0;

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xffff);

    glDisable(GL_BLEND);
    if (m_renderMode & 2) {
        renderTerrain(proj, view);
    }
    if (m_renderMode & 1) {
        renderTerrainWf(proj, view);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    renderWater(proj, view);

    return m_statistics;
}
