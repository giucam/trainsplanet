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

#ifndef DATAFETCHER_H
#define DATAFETCHER_H

#include <QObject>
#include <QMutex>
#include <QQueue>

class Terrain;
class QuadTreeNode;

class DataFetcher : public QObject
{
    Q_OBJECT
public:
    explicit DataFetcher(Terrain *terrain);

    void fetchNode(QuadTreeNode *node);

public slots:
    void run();

private:
    Terrain *m_terrain;
    QMutex m_mutex;
    bool m_running;
    QQueue<QuadTreeNode *> m_queue;
};

#endif
