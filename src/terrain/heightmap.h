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

#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

#include <QVector>

#include "NoisePerlin.h"
#include "NoiseSelect.h"
#include "NoiseConstant.h"
#include "NoiseScalePoint.h"
#include "NoiseRidgedMulti.h"
#include "NoiseScaleBias.h"
#include "NoiseBillow.h"

class HeightMapChunk;
class Generator;

class HeightMap
{
public:
    enum class Face {
        Top,
        Bottom,
        Front,
        Right,
        Left,
        Back
    };

    HeightMap(Generator *generator);

    HeightMapChunk *chunk(Face face, int x, int y, int size);
    inline int size() const { return m_size; }

private:
    int m_size;
    QVector<float> m_data;
    Generator *m_generator;

    friend HeightMapChunk;
};

class HeightMapChunk
{
public:
    /**
     * A padding of one sample will be added all around the chunk, so the data pointer
     * MUST be of size (size + 2) * (size + 2)
     */
    bool fetchData(int size, float *data);
    HeightMapChunk *chunk(int x, int y, int w, int h);

    inline int x() const { return m_x; }
    inline int y() const { return m_y; }
    inline int size() const { return m_size; }
    inline HeightMap::Face face() const { return m_face; }

// private:
    HeightMap *map;
    HeightMap::Face m_face;
    int m_x, m_y, m_size;

    friend class HeightMap;
};

class Generator
{
public:
    Generator() {}

    virtual bool fetchData(int destSize, HeightMap::Face face, const QPoint &pos, int size, float *data) = 0;
    virtual int size() const = 0;

protected:
    HeightMap *map;

    friend HeightMap;
};

class RandomGenerator : public Generator
{
public:
    RandomGenerator(int size, int seed);

    bool fetchData(int destSize, HeightMap::Face face, const QPoint &pos, int size, float *data) override;
    int size() const override;

private:
    int m_size;
    noisepp::PerlinModule m_continents;
    noisepp::SelectModule m_continentSelect;
    noisepp::ConstantModule m_ocean;
    noisepp::PerlinModule m_mountainDefinition;
    noisepp::ScalePointModule m_mountainDefinitionScalePoint;
    noisepp::SelectModule m_mountainSelect;
    noisepp::ScaleBiasModule m_mountainSelectScaleBias;
    noisepp::RidgedMultiModule m_mountains;
    noisepp::ScaleBiasModule m_mountainsScaleBias;
    noisepp::ScalePointModule m_mountainsScalePoint;
    noisepp::BillowModule m_lowlands;
    noisepp::ScaleBiasModule m_lowlandsScaleBias;
    noisepp::ScalePointModule m_lowlandsScalePoint;

    noisepp::Pipeline3D *m_pipeline;
    noisepp::Cache *m_cache;
    noisepp::PipelineElement3D *m_element;
};

#endif
