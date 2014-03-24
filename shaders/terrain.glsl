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

uniform highp mat4 pvm;
uniform highp mat4 proj;
uniform highp mat4 view;
uniform highp mat4 model;
uniform highp int meshSize;
uniform highp vec4 nodeData;
uniform highp vec2 morphData;
uniform highp int faceSize;
uniform highp vec3 cameraPos;
uniform highp vec3 cursorPos;

uniform sampler2DRect heightmap;
uniform sampler2DRect overlay;
in highp vec2 vertex;

out mediump vec4 color;
out mediump vec2 texUV;
out mediump vec3 normal;
out mediump vec2 vertexPos;
out mediump float cursorDistance;
out mediump float vertexHeight;

vec2 morphVertex(vec2 gridPos, float morphK, int size)
{
    vec2 fracPart = fract(gridPos.xy * 0.5 ) * 2.0 / size;
    return -fracPart * morphK;
}

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

vec2 makeUV(vec2 p)
{
    return p + vec2(2.5, 2.5);
}

void main(void)
{
    vec4 pos = vec4(vertexGridPos(vertex), 0., 1.);

    float eyeDist = length((model * pos).xyz - cameraPos);
    float morphing = clamp(morphData.x - eyeDist * morphData.y, 0.0, 1.0 );
    float morphing2 = clamp(morphData.x - eyeDist * morphData.y / 2., 0.0, 1.0 );

    vec2 posInGrid = vertex;
    if (morphing > 0.) {
        vec2 displace = morphVertex(vertex, morphing, meshSize);
        pos.xy += displace * nodeData.zw ;
        posInGrid += displace * meshSize;
    }
    vec2 uv = makeUV(posInGrid);
    float height = texture(heightmap, uv).r;

    height += texture(overlay, uv).r;

//     if (morphing2 > 0.) {
//         vec2 displace = morphVertex(posInGrid.xy / 2, morphing2, meshSize / 2);
//         pos.xy += displace * nodeData.zw ;
//         posInGrid += displace * meshSize;
//
//         uv = posInGrid.xy + vec2(0.5, 0.5);
//         height = texture(heightmap, uv).r;
//     }

    texUV = pos.xy / meshSize;

    vec4 modelPos = model * pos;
    vertexPos = pos.xy;
    vertexHeight = height;

    gl_Position = proj * view * vec4(mapToSphere(modelPos.xyz, height), 1.);

    vec3 off = vec3(-1. - morphing, 0., 1. + morphing);
    float s01 = texture(heightmap, uv + off.xy).r;
    float s21 = texture(heightmap, uv + off.zy).r;
    float s10 = texture(heightmap, uv + off.yx).r;
    float s12 = texture(heightmap, uv + off.yz).r;

    vec3 v01 = mapToSphere((model * vec4(vertexGridPos(vec2(posInGrid + off.xy)), 0., 1.)).xyz, s01);
    vec3 v21 = mapToSphere((model * vec4(vertexGridPos(vec2(posInGrid + off.zy)), 0., 1.)).xyz, s21);
    vec3 v10 = mapToSphere((model * vec4(vertexGridPos(vec2(posInGrid + off.yx)), 0., 1.)).xyz, s10);
    vec3 v12 = mapToSphere((model * vec4(vertexGridPos(vec2(posInGrid + off.yz)), 0., 1.)).xyz, s12);

    vec3 va = v21 - v01;
    vec3 vb = v12 - v10;

    normal = vec3(cross(va, vb));

    cursorDistance = length(cursorPos - modelPos.xyz);
}



[fragment]
#version 330

uniform sampler2D grass;
uniform sampler2D grass2;

in mediump vec4 color;
in mediump vec2 texUV;
in mediump vec3 normal;
in mediump vec2 vertexPos;
in mediump float vertexHeight;
in mediump float cursorDistance;

//**************************************************************************
// Description : Array- and textureless GLSL 2D simplex noise.
// Author : Ian McEwan, Ashima Arts. Version: 20110822
// Copyright (C) 2011 Ashima Arts. All rights reserved.
// Distributed under the MIT License. See 3dparty/webgl-noise.LICENSE file.
// https://github.com/ashima/webgl-noise

float aastep ( float threshold , float value ) {
  float afwidth = 0.7 * length ( vec2 ( dFdx ( value ), dFdy ( value )));
  return smoothstep ( threshold - afwidth , threshold + afwidth , value );
}

vec2 mod289(vec2 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec3 mod289(vec3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec3 permute(vec3 x) { return mod289(((x*34.0)+1.0)*x); }

float snoise(vec2 v) {
  const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
                      0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
                     -0.577350269189626,  // -1.0 + 2.0 * C.x
                      0.024390243902439); // 1.0 / 41.0
// First corner
  vec2 i = floor(v + dot(v, C.yy) );
  vec2 x0 = v - i + dot(i, C.xx);
// Other corners
  vec2 i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;
// Permutations
  i = mod289(i); // Avoid truncation effects in permutation
  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
                           + i.x + vec3(0.0, i1.x, 1.0 ));
  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy),
                          dot(x12.zw,x12.zw)), 0.0);
  m = m*m; m = m*m;
// Gradients
  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 a0 = x - floor(x + 0.5);
// Normalise gradients implicitly by scaling m
  m *= 1.792843 - 0.853735 * ( a0*a0 + h*h );
// Compute final noise value at P
  vec3 g;
  g.x = a0.x * x0.x + h.x * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}
//**************************************************************************

void main(void)
{
    vec2 st = texUV / 128;
    vec3 rgb = texture2D(grass, texUV).rgb;
    float n = 0.1*snoise(st*100.0); // Fractal noise
    n += 0.1*snoise(vertexPos *0.01);
    n += 0.1*snoise(st*400.0);
    n -= 0.1*snoise(vertexPos*0.1);
    n += 0.05*snoise(st*800.0);

    vec4 texColor = vec4(mix(rgb, texture2D(grass2, texUV).rgb, clamp(n * 15, 0., 1.)), 1.0);

    texColor = mix(vec4(1, 0, 1,1), texColor, clamp(cursorDistance / 20., 0., 1.));

    float ambient = 0.5;
    vec3 lightDir = (normalize(vec4(0,0.5,0,0))).xyz;
    vec3 lightColor = vec3(1,1,1);
    float diffuseIntensity = max(0.0, dot(normalize(normal.xyz), -lightDir));
    gl_FragColor = texColor * vec4(1,1,1,1) * vec4(lightColor * (ambient + diffuseIntensity), 1.0);
}



[fragment_wf]
#version 330

void main(void)
{
    gl_FragColor = vec4(1, 1, 1, 1);
}


