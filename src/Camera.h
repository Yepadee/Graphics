#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ModelTriangle.h>
#include <CanvasTriangle.h>

using namespace glm;

mat4x4 constructCameraSpace(const vec3& pos, const vec3& angle)
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
    
    return transpose(make_mat4(values));
}

//Naive projection. 
CanvasPoint project2D(const vec3& pointWorldSpace, const mat4x4& worldToCamera, float focalLength,
                      float canvasWidth, float canvasHeight,
                      float imageWidth , float imageHeight)
{
    // Translate point in world space to point in camera space:
    //mat4x4 cameraToWorld = inverse(worldToCamera);
    vec4 pointCamSpace = worldToCamera * vec4(pointWorldSpace, 1.0f);

    // Perspective Projection:
    vec2 pointScreenSpace;
    pointScreenSpace.x = focalLength * pointCamSpace.x / -pointCamSpace.z;
    pointScreenSpace.y = focalLength * pointCamSpace.y / -pointCamSpace.z;

    // Normalize coordinates (range [0.0, 1.0])
    vec2 pNDC;
    pNDC.x = (pointScreenSpace.x + canvasWidth  / 2.0f) / canvasWidth;
    pNDC.y = (pointScreenSpace.y + canvasHeight / 2.0f) / canvasHeight;


    // Convert to pixel coords.
    CanvasPoint pRaster;
    pRaster.x = std::floor(pNDC.x * imageWidth);
    pRaster.y = std::floor((1.0f - pNDC.y) * imageHeight);
    pRaster.depth = pointCamSpace.z;

    return pRaster;
}

CanvasTriangle projectTriangle(const ModelTriangle& modelTriangle, const mat4x4& worldToCamera, float focalLength,
                               float canvasWidth, float canvasHeight,
                               float imageWidth, float imageHeight)
{
    CanvasTriangle canvasTriangle;
    for (int i = 0; i < 3; ++i)
    {
        CanvasPoint projected = project2D(modelTriangle.vertices[i], worldToCamera, focalLength, canvasWidth, canvasHeight, imageWidth, imageHeight);
        canvasTriangle.vertices[i] = projected;
        canvasTriangle.colour = modelTriangle.colour;
    }

    return canvasTriangle;
}