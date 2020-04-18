#pragma once

#include <math.h>
#include <glm/glm.hpp>

#include <ModelTriangle.h>
#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <RayTriangleIntersection.h>

#include <vector>

#include "Object.h"
#include "Camera.h"
#include "Light.h"


using namespace glm;

std::ostream& operator<<(std::ostream& os, const glm::mat3x3& mat)
{
    os << "|" << mat[0][0] << " " << mat[0][1] << " " << mat[0][2] << "|" << std::endl;
    os << "|" << mat[1][0] << " " << mat[1][1] << " " << mat[1][2] << "|" << std::endl;
    os << "|" << mat[2][0] << " " << mat[2][1] << " " << mat[2][2] << "|" << std::endl;
    return os;
}

/**
 * Get closest ray-triangle intersection.
 *
 * @param cameraPosition Position of camera.
 * @param rayDirection Direction of ray from camera.
 * @param objects List of objects in the world that the ray may intersect.
 * @param result The resultant ray-triangle intersection (if found).
 * @return found Wether or not an intersection was found.
 */
bool getClosestIntersection(const vec3& cameraPosition, const vec3& rayDirection,
                            const std::vector<Object>& objects,
                            RayTriangleIntersection& result)
{
    bool found = false;
    float minPointDistance = std::numeric_limits<float>::infinity();
    for (int i = 0; i < (int) objects.size(); ++i)
    {
        Object object = objects[i];
        for (int j = 0; j < (int) object.triangles.size(); ++j)
        {
            ModelTriangle triangle = object.triangles[j];
            vec3 e0 = triangle.vertices[1] - triangle.vertices[0];
            vec3 e1 = triangle.vertices[2] - triangle.vertices[0];
            vec3 SPVector = cameraPosition - triangle.vertices[0];
            mat3 DEMatrix(-rayDirection, e0, e1);
            vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;

            float t = possibleSolution.x;
            float u = possibleSolution.y;
            float v = possibleSolution.z;

            if (u >= 0.0f && u <= 1.0f &&
                v >= 0.0f && v <= 1.0f &&
                u + v     <=      1.0f &&
                t < minPointDistance)
            {
                minPointDistance = t;
                result.intersectionPoint = triangle.vertices[0] + u * e0 + v * e1;
                result.distanceFromCamera = minPointDistance;
                result.intersectedTriangle = triangle;
                result.objectId = i;
                result.triangleId = j;

                if (triangle.hasVertexNormals)
                {
                    vec3 n0 = triangle.vertexNormals[0];
                    vec3 n1 = triangle.vertexNormals[1];
                    vec3 n2 = triangle.vertexNormals[2];
                    result.normal = u * n1 + v * n2 + (1 - u - v) * n0;
                }
                else
                {
                    result.normal = triangle.normal;
                }

                if (triangle.hasTexture)
                {
                    vec2 t0 = triangle.textureVertices[0];
                    vec2 t1 = triangle.textureVertices[1];
                    vec2 t2 = triangle.textureVertices[2];

                    vec3 v0 = triangle.vertices[0];
                    vec3 v1 = triangle.vertices[1];
                    vec3 v2 = triangle.vertices[2];

                    //vec2 texturePointInv = u * t1 / v1.z + v * t2 / v2.z + (1 - u - v) * t0 / v0.z;
                    //float pointZ = u * v1.z + v * v2.z + (1 - u - v) * v0.z;
                    
                    vec2 texturePoint = u * t1 + v * t2 + (1 - u - v) * t0;

                    uint32_t textureColour = object.texture.GetPixel(texturePoint.x * object.texture.getWidth(), texturePoint.y * object.texture.getHeight());
                    int r = getSubPixel(textureColour, 2);
                    int g = getSubPixel(textureColour, 1);
                    int b = getSubPixel(textureColour, 0);
                    Colour colour(r, g, b);

                    result.colour = colour;
                }
                else
                {
                    result.colour = triangle.colour;
                }

                //Find colour of texture point and use that as the colour.
                
                found = true;
            }
        }
    }

    return found;
}

void drawScreenBuffer(DrawingWindow& window, std::vector<vec3> aaOffsets, uint32_t* screenBuffer)
{
    int numAAOffsets = aaOffsets.size();

    for (int j = 0; j < window.height; ++j)
    {
        for (int i = 0; i < window.width; ++i)
        {
            float sumRed = 0;
            float sumGreen = 0;
            float sumBlue = 0;
            float sumWeights = 0.0f;

            for (int k = 0; k < numAAOffsets; ++k)
            {
                vec3 offset = aaOffsets[k];

                uint32_t colour = screenBuffer[k + i * numAAOffsets + j * window.width * numAAOffsets];

                sumRed += offset.z * ((colour & (255 << 16)) >> 16);
                sumGreen += offset.z * ((colour & (255 << 8)) >> 8);
                sumBlue += offset.z * (colour & (255));
                sumWeights += offset.z;
            }

            uint32_t aaColour = packRGB(sumRed / sumWeights, sumGreen / sumWeights, sumBlue / sumWeights);
            window.setPixelColour(i, j, aaColour);
        }
    }
}

/**
 * Render objects in scene to window via ray-tracing.
 *
 * @param objects List of objects to render.
 * @param cameraToWorld Homogenious cameraToWorld matrix.
 * @param focalLength The focal length of the camera.
 * @param window The window to draw ray-traced objects on.
 */
void rayTraceObjects(const std::vector<Object>& objects, const std::vector<Light>& lights, const mat4x4& cameraToWorld, float focalLength, DrawingWindow& window, std::vector<vec3> aaOffsets)
{
    mat3x3 cameraSpace = getCameraRotation(cameraToWorld);
    vec3 cameraPos = getCameraPosition(cameraToWorld);

    std::cout << cameraPos << std::endl;
    std::cout << cameraSpace << std::endl;
    
    int numAAOffsets = aaOffsets.size();

    vec3 rayWorldSpace;
    vec3 rayCameraSpace;
    RayTriangleIntersection rti;

    vec3* colourBuffer = new vec3[window.width * window.height * numAAOffsets];
    vec3* brightnessBuffer = new vec3[window.width * window.height * numAAOffsets];
    uint32_t* screenBuffer = new uint32_t[window.width * window.height * numAAOffsets];
    float* occlusionBuffer = new float[window.width * window.height * numAAOffsets];
    float* depthBuffer = new float[window.width * window.height * numAAOffsets];
    float* lightSizeBuffer = new float[window.width * window.height * numAAOffsets];

    for (int j = 0; j < window.height; ++j)
    {
        for (int i = 0; i < window.width; ++i)
        {
            for (int k = 0; k < numAAOffsets; ++k)
            {
                vec3 offset = aaOffsets[k];
                rayCameraSpace.x = i - window.width / 2.0f + offset.x;
                rayCameraSpace.y = window.height / 2.0f - j + offset.y;
                rayCameraSpace.z = -focalLength;

                rayWorldSpace = normalize(cameraSpace * rayCameraSpace);

                if (getClosestIntersection(cameraPos, rayWorldSpace, objects, rti))
                {
                    depthBuffer[k + i * numAAOffsets + j * window.width * numAAOffsets] = rti.intersectionPoint.z;

                    getReducedOcclusionValue(rti.intersectionPoint, lights, objects,
                    occlusionBuffer[k + i * numAAOffsets + j * window.width * numAAOffsets],
                    lightSizeBuffer[k + i * numAAOffsets + j * window.width * numAAOffsets]);

                    colourBuffer[k + i * numAAOffsets + j * window.width * numAAOffsets] = vec3(rti.colour.red, rti.colour.green, rti.colour.blue);
                    brightnessBuffer[k + i * numAAOffsets + j * window.width * numAAOffsets] = getPointBrightess(rti, rayWorldSpace, objects, lights);

                    
                    
                    if (occlusionBuffer[k + i * numAAOffsets + j * window.width * numAAOffsets] < 1.0f)
                    {
                        brightnessBuffer[k + i * numAAOffsets + j * window.width * numAAOffsets] = vec3(0.2f);
                    }
                }
                else
                {
                    occlusionBuffer[k + i * numAAOffsets + j * window.width * numAAOffsets] = 1.0f;
                    colourBuffer[k + i * numAAOffsets + j * window.width * numAAOffsets] = vec3(0.0f);
                }

                screenBuffer[k + i * numAAOffsets + j * window.width * numAAOffsets] = packRGB(colourBuffer[k + i * numAAOffsets + j * window.width * numAAOffsets] * 
                                                                                               brightnessBuffer[k + i * numAAOffsets + j * window.width * numAAOffsets]);
            }
        }
    }

    

    drawScreenBuffer(window, aaOffsets, screenBuffer);
}


