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

#ifndef GLPROGRAM_H
#define GLPROGRAM_H

#include <QOpenGLShaderProgram>

class GlProgram : public QOpenGLShaderProgram
{
    Q_OBJECT
public:
    GlProgram(const QString &file, QObject *parent = nullptr);
    ~GlProgram();

    void setVertexShader(const QString &name);
    void setFragmentShader(const QString &name);

    bool link() override;

private:
    struct Shader
    {
        QString name;
        QString code;
        QOpenGLShader *shader;
    };

    QString m_file;
    Shader m_vsh;
    Shader m_fsh;
};

#endif
