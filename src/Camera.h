#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

mat4x4 constructCameraSpace(vec3 pos, vec3 angle)
{
    float X = angle.x;
    float Y = angle.y;
    float Z = angle.z;

    float values[16] = {
     cos(Y)*cos(Z), -cos(X)*sin(Z) + sin(X)*sin(Y)*cos(Z),  sin(X)*sin(Z) + cos(X)*sin(Y)*cos(Z), pos.x,
     cos(Y)*sin(Z),  cos(X)*cos(Z) + sin(X)*sin(Y)*sin(Z), -sin(X)*cos(Z) + cos(X)*sin(Y)*sin(Z), pos.y,
    -sin(Y)       ,  sin(X)*cos(Y)                       ,  cos(X)*cos(Y)                       , pos.z,
     0.0f         ,  0.0f                                ,  0.0f                                , 1.0f
    };

    return  transpose(make_mat4(values));
}

vec2 project2D(vec3 pointWorldSpace, mat4x4 worldToCamera, float focalLength,
               float canvasWidth, float canvasHeight,
               float imageWidth , float imageHeight)
{
    // Translate point in world space to point in camera space:
    //mat4x4 cameraToWorld = inverse(worldToCamera);
    vec4 pointCamSpace = worldToCamera * vec4(pointWorldSpace, 1.0f);

    // Perspective Projection:
    vec2 pointScreenSpace;
    pointScreenSpace.x = pointCamSpace.x / -pointCamSpace.z;
    pointScreenSpace.y = pointCamSpace.y / -pointCamSpace.z;

    // Normalize coordinates (range [0.0, 1.0])
    vec2 pNDC;
    pNDC.x = (pointScreenSpace.x + canvasWidth  / 2.0f) / canvasWidth;
    pNDC.y = (pointScreenSpace.y + canvasHeight / 2.0f) / canvasHeight;


    // Convert to pixel coords.
    vec2 pRaster;
    pRaster.x = std::floor(pNDC.x * imageWidth);
    pRaster.y = std::floor((1.0f - pNDC.y) * imageHeight);

    return pRaster;
}