#pragma once

#include <math.h>
#include <glm/glm.hpp>
#include <ModelTriangle.h>
#include <CanvasTriangle.h>
#include <DrawingWindow.h>

#include <RayTriangleIntersection.h>

#include <vector>

#include "PixelUtil.h"
#include "Object.h"
#include "Camera.h"


using namespace glm;

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
    for (Object object : objects)
    {
        for (ModelTriangle triangle : object.triangles)
        {
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
                found = true;
            }
        }
    }

    return found;
}

std::ostream& operator<<(std::ostream& os, const glm::mat3x3& mat)
{
    os << "|" << mat[0][0] << " " << mat[0][1] << " " << mat[0][2] << "|" << std::endl;
    os << "|" << mat[1][0] << " " << mat[1][1] << " " << mat[1][2] << "|" << std::endl;
    os << "|" << mat[2][0] << " " << mat[2][1] << " " << mat[2][2] << "|" << std::endl;
    return os;
}


float applyProximityLight(RayTriangleIntersection rti, vec4 lightSource)
{   
    glm::vec3 diff = rti.intersectionPoint - vec3(lightSource);
    float distSqr = dot(diff, diff);
    float strength = lightSource.w;
    return ((strength * 1.0f)/(4.0f * distSqr * M_PI));
}

float applyAOILight(RayTriangleIntersection rti, vec4 lightSource)
{
    //vec3 vs[3] = rti.intersectedTriangle.vertices;
    vec3 normal = glm::cross(rti.intersectedTriangle.vertices[1] - rti.intersectedTriangle.vertices[0], rti.intersectedTriangle.vertices[2] - rti.intersectedTriangle.vertices[0]);
    
    vec3 ray = vec3(lightSource) - rti.intersectionPoint;

    float aoiLight = dot(normalize(normal), normalize(ray));
    return aoiLight > 0.0f ? aoiLight : 0.0f;
}

uint32_t applyBrightness(Colour colour, float brightness)
{
    float r = colour.red * brightness;
    float g = colour.green * brightness;
    float b = colour.blue * brightness;
    return packRGB(r,g,b);
}


/**
 * Render objects in scene to window via ray-tracing.
 *
 * @param objects List of objects to render.
 * @param cameraToWorld Homogenious cameraToWorld matrix.
 * @param focalLength The focal length of the camera.
 * @param window The window to draw ray-traced objects on.
 */
void rayTraceObjects(const std::vector<Object>& objects, const std::vector<vec4>& lights, const mat4x4& cameraToWorld, float focalLength, DrawingWindow& window)
{
    mat3x3 cameraSpace = getCameraRotation(cameraToWorld);
    vec3 cameraPos = getCameraPosition(cameraToWorld);

    std::cout << cameraPos << std::endl;
    std::cout << cameraSpace << std::endl;
    
    vec3 rayWorldSpace;
    vec3 rayCameraSpace;
    RayTriangleIntersection rti;
    for (int j = 0; j < window.height; ++j)
    {
        for (int i = 0; i < window.width; ++i)
        {
            rayCameraSpace.x = i - window.width / 2.0f;
            rayCameraSpace.y = window.height / 2.0f - j;
            rayCameraSpace.z = -focalLength;

            rayWorldSpace = normalize(cameraSpace * rayCameraSpace);

            if (getClosestIntersection(cameraPos, rayWorldSpace, objects, rti))
            {
                float brightness = 0.2f;
                for (vec4 lightSource : lights){
                    brightness += applyProximityLight(rti, lightSource) * applyAOILight(rti, lightSource);

                    if (brightness > 1.0f) brightness = 1.0f;
                    if (brightness < 0.2f) brightness = 0.2f;

                    uint32_t colour = applyBrightness(rti.intersectedTriangle.colour, brightness);

                    window.setPixelColour(i, j, colour);
                }
                
            }
            else
            {
                window.setPixelColour(i, j, 0);
            }
        }
    }
}