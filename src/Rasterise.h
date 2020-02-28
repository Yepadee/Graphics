#pragma once

#include <glm/glm.hpp>
#include <ModelTriangle.h>
#include <CanvasTriangle.h>

#include "Drawing3D.h"
#include "Camera.h"

using namespace glm;

/**
 * Project a 3D vector in world space to a 2D CanvasPoint in screen space.
 *
 * @param pointWorldSpace A 3D point in world space.
 * @param worldToCamera A 4x4 matrix that maps points from the world space to the camera space.
 * @param focalLength The focal length of the camera.
 * @param canvasWidth The width of the canvas points are projected to in scale relative to values in world space.
 * @param canvasHeight The height of the canvas points are projected to in scale relative to values in world space.
 * @param imageWidth The width of the window points are to be drawn on to.
 * @param imageHeight The height of the window points are to be drawn on to.
 * @return 2D projection of the provided 3D point.
 */
CanvasPoint rasterisePoint(const vec3& pointWorldSpace, const mat4x4& worldToCamera, float focalLength,
                      float canvasWidth, float canvasHeight,
                      float imageWidth , float imageHeight)
{
    // Translate point in world space to point in camera space.
    vec4 pointCamSpace = worldToCamera * vec4(pointWorldSpace, 1.0f);

    // Perspective Projection.
    vec2 pointScreenSpace;
    pointScreenSpace.x = focalLength * pointCamSpace.x / -pointCamSpace.z;
    pointScreenSpace.y = focalLength * pointCamSpace.y / -pointCamSpace.z;

    // Normalize coordinates (range [0.0, 1.0]).
    vec2 pNDC;
    pNDC.x = (pointScreenSpace.x + canvasWidth  / 2.0f) / canvasWidth;
    pNDC.y = (pointScreenSpace.y + canvasHeight / 2.0f) / canvasHeight;

    // Convert to pixel coords.
    CanvasPoint pRaster;
    pRaster.x = std::floor(pNDC.x * imageWidth);
    pRaster.y = std::floor((1.0f - pNDC.y) * imageHeight);
    pRaster.depth = -1.0f/pointCamSpace.z;

    return pRaster;
}

/**
 * Project a 3D vector in world space to a 2D CanvasPoint in screen space.
 *
 * @param modelTriangle A 3D Triangle in world space.
 * @param worldToCamera 4x4 matrix that maps points from the world space to the camera space.
 * @param focalLength The focal length of the camera.
 * @param canvasWidth The width of the canvas points are projected to in scale relative to values in world space.
 * @param canvasHeight The height of the canvas points are projected to in scale relative to values in world space.
 * @param imageWidth The width of the window points are to be drawn on to.
 * @param imageHeight The height of the window points are to be drawn on to.
 * @return canvasTriangle The 2D projection of the provided 3D triangle.
 */
CanvasTriangle rasteriseTriangle(const ModelTriangle& modelTriangle, const mat4x4& worldToCamera, float focalLength,
                               float canvasWidth, float canvasHeight,
                               float imageWidth, float imageHeight)
{
    CanvasTriangle canvasTriangle;
    for (int i = 0; i < 3; ++i)
    {
        CanvasPoint projected = rasterisePoint(modelTriangle.vertices[i], worldToCamera, focalLength, canvasWidth, canvasHeight, imageWidth, imageHeight);
        canvasTriangle.vertices[i] = projected;
        canvasTriangle.colour = modelTriangle.colour;
    }

    return canvasTriangle;
}

void rasteriseObjects(const std::vector<Object>& objects, const mat4x4& cameraToWorld, float focalLength, DrawingWindow& window)
{
    mat4x4 worldToCamera = inverse(cameraToWorld);
    for (Object obj : objects)
    {
        for (ModelTriangle m : obj.triangles)
        {
          CanvasTriangle t = rasteriseTriangle(m, worldToCamera, focalLength, window.width, window.height, window.width, window.height);
          fillTriangle(t, window);
        }
    }
}

void rasteriseObjectsWireframe(const std::vector<Object>& objects, const mat4x4& cameraToWorld, float focalLength, DrawingWindow& window)
{
    mat4x4 worldToCamera = inverse(cameraToWorld);
    for (Object obj : objects)
    {
        for (ModelTriangle m : obj.triangles)
        {
          CanvasTriangle t = rasteriseTriangle(m, worldToCamera, focalLength, window.width, window.height, window.width, window.height);
          uint32_t colour = packRGB(t.colour);
          drawTriangle(t, colour, window);
        }
    }
}