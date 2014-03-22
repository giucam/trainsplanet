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

#include <QOpenGLShader>

#include "glprogram.h"
#include "miscutils.h"

GlProgram::GlProgram(const QString &file, QObject *p)
         : QOpenGLShaderProgram(p)
         , m_file(FileFinder::findFile(FileFinder::Type::Shader, file))
{
    m_vsh.shader = nullptr;
    m_fsh.shader = nullptr;
}

GlProgram::~GlProgram()
{
}

void GlProgram::setVertexShader(const QString &name)
{
    m_vsh.name = name;
    m_vsh.code = MiscUtils::shaderCode(m_file, name);
}

void GlProgram::setFragmentShader(const QString &name)
{
    m_fsh.name = name;
    m_fsh.code = MiscUtils::shaderCode(m_file, name);
}

bool GlProgram::link()
{
    m_vsh.shader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    if (m_vsh.shader->compileSourceCode(m_vsh.code)) {
        addShader(m_vsh.shader);
    } else {
        qDebug("Failed compiling the vertex shader '%s' in file '%s'.", qPrintable(m_vsh.name), qPrintable(m_file));
    }

    m_fsh.shader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    if (m_fsh.shader->compileSourceCode(m_fsh.code)) {
        addShader(m_fsh.shader);
    } else {
        qDebug("Failed compiling the fragment shader '%s' in file '%s'.", qPrintable(m_fsh.name), qPrintable(m_file));
    }

    return QOpenGLShaderProgram::link();
}
