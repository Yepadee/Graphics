#pragma once

#include <glm/glm.hpp>
#include <ModelTriangle.h>
#include <CanvasTriangle.h>
#include <DrawingWindow.h>

#include <RayTriangleIntersection.h>

#include <vector>

#include "PixelUtil.h"

#include "Object.h"

using namespace glm;

bool getClosestIntersection(const vec3& cameraPosition, const vec3& rayDirection,
                            const std::vector<Object>& object,
                            RayTriangleIntersection& result)
{
    bool found = false;
    float minPointDistance = -std::numeric_limits<float>::infinity();

    for (ModelTriangle triangle : triangles)
    {
        vec3 e0 = triangle.vertices[1] - triangle.vertices[0];
        vec3 e1 = triangle.vertices[2] - triangle.vertices[0];
        vec3 SPVector = cameraPosition-triangle.vertices[0];
        mat3 DEMatrix(-rayDirection, e0, e1);
        vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;

        float t = possibleSolution.x;
        float u = possibleSolution.y;
        float v = possibleSolution.z;

        if (u >=0.0f && u <= 1.0f &&
            v >=0.0f && v <= 1.0f &&
            u + v <= 1.0f &&
            t < minPointDistance)
        {
            minPointDistance = t;
            result.intersectionPoint = u * e0 + v * e1;
            result.distanceFromCamera = minPointDistance;
            result.intersectedTriangle = triangle;
            found = true;
        }
    }

    return found;
}


void renderFrame(const mat3x3& cameraSpace, const vec3& cameraPos,
                 float focalLength, DrawingWindow& window)
{
    vec3 rayWorldSpace;
    vec3 rayCameraSpace;
    RayTriangleIntersection rti;
    for (int j = 0; j < imageHeight; ++j)
    {
        for (int i = 0; i < imageWidth; ++i)
        {
            rayCameraSpace.x = i - window.width / 2.0f;
            rayCameraSpace.y = window.height / 2.0f - j;
            rayCameraSpace.z = -focalLength;

            rayWorldSpace = normalize(cameraSpace * rayCameraSpace);

            if (getClosestIntersection(cameraPos, rayWorldSpace, triangles, rti))
            {
                uint32_t colour = packRGB(rti.intersectedTriangle.colour);
                window.setPixelColour(i, j, colour);
            }
            else
            {
                window.setPixelColour(i, j, 0);
            }
        }
    }
}