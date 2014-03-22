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

#include <QDebug>
#include <QMutexLocker>

#include "datafetcher.h"
#include "terrain.h"
#include "quadtree.h"

DataFetcher::DataFetcher(Terrain *terrain)
           : QObject()
           , m_terrain(terrain)
           , m_running(false)
{
}

void DataFetcher::fetchNode(QuadTreeNode *node)
{
    QMutexLocker lock(&m_mutex);
    m_queue.enqueue(node);

    if (!m_running) {
        QMetaObject::invokeMethod(this, "run", Qt::QueuedConnection);
    }
}

void DataFetcher::run()
{
    m_mutex.lock();
    m_running = true;

    while (!m_queue.isEmpty()) {
        QuadTreeNode *node = m_queue.dequeue();
        m_mutex.unlock();

        node->fetchData();

        m_mutex.lock();
    }

    m_running = false;
    m_mutex.unlock();
}
