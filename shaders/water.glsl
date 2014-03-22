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

[vertex]
#version 330

uniform highp mat4 pv;
uniform highp mat4 model;
uniform highp int meshSize;
uniform highp vec4 nodeData;
uniform highp vec2 morphData;
uniform highp float waterLevel;
uniform highp float heightScale;
uniform highp int faceSize;

in highp vec2 vertex;

vec3 mapToSphere(vec3 pos, float height)
{
    vec3 p = pos;
    float d = faceSize / 2. * (meshSize - 1) / meshSize;
    float x = p.x / d;
    float y = p.y / d;
    float z = p.z / d;
    p.x *= sqrt(1.0 - y * y * 0.5 - z * z * 0.5 + y * y * z * z / 3.0);
    p.y *= sqrt(1.0 - z * z * 0.5 - x * x * 0.5 + z * z * x * x / 3.0);
    p.z *= sqrt(1.0 - x * x * 0.5 - y * y * 0.5 + x * x * y * y / 3.0);

    p += normalize(p).xyz * height;
    return p;
}

vec2 vertexGridPos(vec2 p)
{
    vec2 relPos = p * nodeData.zw / meshSize;
    return nodeData.xy + relPos;
}

void main(void)
{
    vec4 pos = vec4(vertexGridPos(vertex), 0., 1.);
    gl_Position = pv * vec4(mapToSphere((model * pos).xyz, waterLevel), 1.);
}

[fragment]
#version 330

void main(void)
{
    gl_FragColor = vec4(0, 0.3, 0.8, 0.7);
}
