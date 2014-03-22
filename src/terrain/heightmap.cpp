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

#include <QSize>
#include <QRect>
#include <QDebug>
#include <QVector3D>

#include "heightmap.h"
#include "terrain.h"

HeightMap::HeightMap(Generator *gen)
         : m_size(gen->size())
         , m_generator(gen)
{
    gen->map = this;
}

HeightMapChunk *HeightMap::chunk(Face face, int x, int y, int size)
{
    HeightMapChunk *chunk = new HeightMapChunk;
    chunk->map = this;
    chunk->m_x = x;
    chunk->m_y = y;
    chunk->m_size = size;
    chunk->m_face = face;
    return chunk;
}

bool HeightMapChunk::fetchData(int size, float *data)
{
    return map->m_generator->fetchData(size, m_face, QPoint(m_x, m_y), m_size, data);
}


static QVector3D mapToSphere(const QVector3D &pos, double faceSize)
{
    QVector3D p = pos;
    double d = faceSize / 2.;
    double x = p.x() / d;
    double y = p.y() / d;
    double z = p.z() / d;
    p[0] *= sqrt(1.0 - y * y * 0.5 - z * z * 0.5 + y * y * z * z / 3.0);
    p[1] *= sqrt(1.0 - z * z * 0.5 - x * x * 0.5 + z * z * x * x / 3.0);
    p[2] *= sqrt(1.0 - x * x * 0.5 - y * y * 0.5 + x * x * y * y / 3.0);

    return p;
}

RandomGenerator::RandomGenerator(int size, int seed)
               : m_size(size)
{
    m_ocean.setValue(-1.0);

    m_mountains.setOctaveCount(12);
    m_mountains.setSeed(seed);

    m_mountainsScaleBias.setSourceModule(0, m_mountains);
    m_mountainsScaleBias.setScale(3.0);
    m_mountainsScaleBias.setBias(0.5);
    m_mountainsScalePoint.setSourceModule(0, m_mountainsScaleBias);
    m_mountainsScalePoint.setScaleX(25);
    m_mountainsScalePoint.setScaleY(25);
    m_mountainsScalePoint.setScaleZ(25);

    m_lowlands.setOctaveCount(7);
    m_lowlands.setSeed(seed);
    m_lowlandsScaleBias.setSourceModule(0, m_lowlands);
    m_lowlandsScaleBias.setScale(0.2);
    m_lowlandsScaleBias.setBias(-0.8);
    m_lowlandsScalePoint.setSourceModule(0, m_lowlandsScaleBias);
    m_lowlandsScalePoint.setScaleX(50);
    m_lowlandsScalePoint.setScaleY(50);
    m_lowlandsScalePoint.setScaleZ(50);

    m_mountainDefinition.setOctaveCount(12);
    m_mountainDefinitionScalePoint.setSourceModule(0, m_mountainDefinition);
    m_mountainDefinitionScalePoint.setScaleX(10);
    m_mountainDefinitionScalePoint.setScaleY(10);
    m_mountainDefinitionScalePoint.setScaleZ(10);

    m_mountainSelect.setControlModule(m_mountainDefinitionScalePoint);
    m_mountainSelect.setSourceModule(0, m_lowlandsScalePoint);
    m_mountainSelect.setSourceModule(1, m_mountainsScalePoint);
    m_mountainSelect.setEdgeFalloff(0.1);
    m_mountainSelect.setLowerBound(0.5);
    m_mountainSelectScaleBias.setSourceModule(0, m_mountainSelect);
    m_mountainSelectScaleBias.setScale(0.5);
    m_mountainSelectScaleBias.setBias(0.5);

    m_continents.setSeed(seed);
    m_continents.setOctaveCount(12);
    m_continents.setFrequency(1.0);
    m_continents.setLacunarity(2.0);
    m_continents.setPersistence(0.625);

    m_continentSelect.setControlModule(m_continents);
    m_continentSelect.setSourceModule(0, m_ocean);
    m_continentSelect.setSourceModule(1, m_mountainSelectScaleBias);
    m_continentSelect.setLowerBound(0.0);
    m_continentSelect.setEdgeFalloff(0.1);

    m_pipeline = new noisepp::Pipeline3D;
    noisepp::ElementID id = m_continentSelect.addToPipeline(m_pipeline);
    m_element = m_pipeline->getElement(id);
    m_cache = m_pipeline->createCache();
}

bool RandomGenerator::fetchData(int destSize, HeightMap::Face face, const QPoint &pos, int size, float *data)
{
    const double multiplier = 1. / 8000.;

    double fw = (double)(size) * multiplier;

    const double faceSize = map->size() * multiplier;
    const double stepSize = fw / (double)(destSize - 1);
    double s = faceSize / 2.;

    double fx = pos.x() * multiplier - 2*stepSize;
    double fy = pos.y() * multiplier - 2*stepSize;

    QVector3D start = face == HeightMap::Face::Bottom ? QVector3D(s - fy,  s - fx,  -s) :
                        face == HeightMap::Face::Front  ? QVector3D(-s + fx, -s,      -s + fy) :
                        face == HeightMap::Face::Right  ? QVector3D(s,       -s + fx, -s + fy) :
                        face == HeightMap::Face::Back   ? QVector3D(s - fx,  s,       -s + fy) :
                        face == HeightMap::Face::Left   ? QVector3D(-s,      s - fx,  -s + fy) :
                                                        QVector3D(-s + fx, -s + fy, s);


    QVector3D step = face == HeightMap::Face::Bottom ? QVector3D(0, -stepSize, 0) :
                        face == HeightMap::Face::Front  ? QVector3D(stepSize, 0, 0) :
                        face == HeightMap::Face::Right  ? QVector3D(0, stepSize, 0) :
                        face == HeightMap::Face::Back   ? QVector3D(-stepSize, 0, 0) :
                        face == HeightMap::Face::Left   ? QVector3D(0, -stepSize, 0) :
                                                        QVector3D(stepSize, 0, 0);

    QVector3D lineStep = face == HeightMap::Face::Bottom ? QVector3D(-stepSize, 0, 0) :
                            face == HeightMap::Face::Front  ? QVector3D(0, 0, stepSize) :
                            face == HeightMap::Face::Right  ? QVector3D(0, 0, stepSize) :
                            face == HeightMap::Face::Back   ? QVector3D(0, 0, stepSize) :
                            face == HeightMap::Face::Left   ? QVector3D(0, 0, stepSize) :
                                                            QVector3D(0, stepSize, 0);



    float *ptr = data;
    QVector3D line = start;
    for (int i = 0; i < destSize + 4; ++i) {
        QVector3D point = line;
        for (int j = 0; j < destSize + 4; ++j) {
            QVector3D p = mapToSphere(point, faceSize);
            *ptr++ = (m_element->getValue(p.x(), p.y(), p.z(), m_cache) + 1.) / 2.;
            point += step;
        }
        line += lineStep;
    }

    return true;
}

int RandomGenerator::size() const
{
    return m_size;
}
