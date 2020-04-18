#pragma once

#include <math.h>
#include <cmath>
#include <glm/glm.hpp>
#include <ModelTriangle.h>
#include <RayTriangleIntersection.h>

#include <stdlib.h>     /* srand, rand */

#include <vector>
#include <set>

#include "Object.h"
#include "Light.h"

using namespace glm;

/**
 * Determine if a point is illuminated by a light source.
 *
 * @param rti Ray-triangle intersection.
 * @param lightPosition The location and brightness of the light.
 * @param objects A list of all object that may be between the point and the lightPosition.
 * @return Wether or not a point is illuminated by the lightPosition.
 */
bool isIlluminated(const vec3& triangleIntersectionPoint, int triangleObjectId, const vec3& lightPosition, 
                   const std::vector<Object>& objects)
{
    vec3 shadowRay =  lightPosition - triangleIntersectionPoint;
    float rayLength = length(shadowRay);

    float minDistance = 0.001f;

    for (int i = 0; i < (int) objects.size(); ++i)
    {
        if (i == triangleObjectId) continue;
        Object object = objects[i];
        for (int j = 0; j < (int) object.triangles.size(); ++j)
        {
            ModelTriangle triangle = object.triangles[j];
            vec3 e0 = triangle.vertices[1] - triangle.vertices[0];
            vec3 e1 = triangle.vertices[2] - triangle.vertices[0];
            vec3 SPVector = triangleIntersectionPoint - triangle.vertices[0];
            mat3 DEMatrix(-normalize(shadowRay), e0, e1);
            vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;

            float t = possibleSolution.x;
            float u = possibleSolution.y;
            float v = possibleSolution.z;

            if (u >= 0.0f && u <= 1.0f &&
                v >= 0.0f && v <= 1.0f &&
                u + v     <=      1.0f &&  
                abs(t)    <  rayLength &&
                t         >  minDistance)
            {
               return false;
            }
        }
    }

    return true;
}


bool lightBlockedByTriangles(const vec3& triangleIntersectionPoint, const vec3& lightPosition,
                             const std::vector<ModelTriangle>& triangles)
{
    vec3 shadowRay =  lightPosition - triangleIntersectionPoint;
    float rayLength = length(shadowRay);
    float minDistance = 0.001f;

    for (ModelTriangle triangle : triangles)
    {
        vec3 e0 = triangle.vertices[1] - triangle.vertices[0];
        vec3 e1 = triangle.vertices[2] - triangle.vertices[0];
        vec3 SPVector = triangleIntersectionPoint - triangle.vertices[0];
        mat3 DEMatrix(-normalize(shadowRay), e0, e1);
        vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;

        float t = possibleSolution.x;
        float u = possibleSolution.y;
        float v = possibleSolution.z;

        if (u >= 0.0f && u <= 1.0f &&
            v >= 0.0f && v <= 1.0f &&
            u + v     <=      1.0f &&  
            abs(t)    <  rayLength &&
            t         >  minDistance)
        {
           return true;
        }
    }
    
    return false;
}

/**
 * Determine if a point is illuminated by a light source.
 *
 * @param rti Ray-triangle intersection.
 * @param objectId the Id of the object the ray triangle intersection belongs to
 * @param lightPosition The location and brightness of the light.
 * @param objects A list of all object that may be between the point and the lightPosition.
 * @return List of triangles blocking light for triangle intersection.
 */
std::vector<Object> getObjectsBlockingLight(const vec3& triangleIntersectionPoint, int triangleObjectId, const vec3& lightPosition,
                                                     const std::vector<Object>& objects)
{
    std::vector<Object> blockingObjects;

    vec3 shadowRay =  lightPosition - triangleIntersectionPoint;
    float rayLength = length(shadowRay);

    float minDistance = 0.001f;

    for (int id = 0; id < (int) objects.size(); ++id)
    {
        if (id == triangleObjectId) continue;
        Object object = objects[id];
        for (ModelTriangle triangle : object.triangles)
        {
            vec3 e0 = triangle.vertices[1] - triangle.vertices[0];
            vec3 e1 = triangle.vertices[2] - triangle.vertices[0];
            vec3 SPVector = triangleIntersectionPoint - triangle.vertices[0];
            mat3 DEMatrix(-normalize(shadowRay), e0, e1);
            vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;

            float t = possibleSolution.x;
            float u = possibleSolution.y;
            float v = possibleSolution.z;

            if (u >= 0.0f && u <= 1.0f &&
                v >= 0.0f && v <= 1.0f &&
                u + v     <=      1.0f &&  
                abs(t)    <  rayLength &&
                t         >  minDistance)
            {
               blockingObjects.push_back(object);
               break;
            }
        }
    }

    return blockingObjects;
}

/**
 * Determine if a point is illuminated by a light source.
 *
 * @param rti Ray-triangle intersection.
 * @param lightPosition The location and brightness of the light.
 * @param objects A list of all object that may be between the point and the lightPosition.
 * @return Wether or not a point is illuminated by the lightPosition.
 */
float getShadowIntensityVS(const RayTriangleIntersection& rti, const vec3& lightPosition, float lightRadius, const std::vector<Object>& objects)
{
    float shadowIntensity = 0.0f;

    float intensityCoeff = 1.0f;
    float surfaceShift = 0.5f * lightRadius;

    vec3 surfaceNormal = normalize(rti.normal);
    vec3 surfaceDisplacement = surfaceShift * surfaceNormal;

    vec3 lowSurfacePoint  = rti.intersectionPoint - surfaceDisplacement;
    vec3 midSurfacePoint  = rti.intersectionPoint;
    vec3 highSurfacePoint = rti.intersectionPoint + surfaceDisplacement;

    std::vector<Object> objectsBlockingLight = getObjectsBlockingLight(midSurfacePoint, rti.objectId, lightPosition, objects);

    bool midDark = objectsBlockingLight.size() > 0;
    bool lowDark  = !isIlluminated(lowSurfacePoint , rti.objectId, lightPosition, objectsBlockingLight);
    bool highDark = !isIlluminated(highSurfacePoint, rti.objectId, lightPosition, objectsBlockingLight);
    
    
    if (lowDark && midDark && highDark) shadowIntensity = 1.0f;
    else if (lowDark && !highDark)
    {
        float stepSize = 0.02f;
        float displacement = 0.0f;

        vec3 checkSurfacePoint = lowSurfacePoint;

        while (!isIlluminated(checkSurfacePoint, rti.objectId, lightPosition, objectsBlockingLight))
        {
            checkSurfacePoint = lowSurfacePoint + surfaceNormal * displacement;
            displacement += stepSize;
        }

        shadowIntensity = intensityCoeff * std::min(1.0f, (displacement - surfaceShift) / lightRadius);
    }

    return shadowIntensity;
}

void getLightBasis(const vec3& triangleIntersectionPoint, const vec3& lightPosition, vec3& lightX, vec3& lightY)
{
    vec3 ray = normalize(lightPosition - triangleIntersectionPoint);
    vec3 yUp(0.0f, 1.0f, 0.0f);
    lightX = cross(normalize(yUp), ray);
    lightY = cross(ray, lightX);
}

vec3 getRandomPointOnLight(const vec3& lightX, const vec3& lightY, float lightRadius)
{
    float rx = 2 * lightRadius * ((float) rand() / (float) RAND_MAX) - lightRadius;
    float ry = 2 * lightRadius * ((float) rand() / (float) RAND_MAX) - lightRadius;

    return rx * lightX + ry * lightY;
}

float getShadowIntensityMLP(const RayTriangleIntersection& rti, const vec3& lightPosition, float lightRadius, const std::vector<Object>& objects)
{
    float shadowIntensity = 0.0f;

    int numRays = (int) (25.0f * lightRadius * lightRadius);
    int numIlluminated = 0;

    vec3 lightX;
    vec3 lightY;
    getLightBasis(rti.intersectionPoint, lightPosition, lightX, lightY);

    for (int i = 0; i < numRays; ++i)
    {
        vec3 randomPointOnLight = lightPosition + getRandomPointOnLight(lightX, lightY, lightRadius);
        if (isIlluminated(rti.intersectionPoint, rti.objectId, randomPointOnLight, objects)) numIlluminated ++;
    }

    shadowIntensity = (float) (numRays - numIlluminated) / (float) numRays;      

    return shadowIntensity;
}


vec3 getShadowRay(const vec3& lightPosition, const vec3& triangleIntersectionPoint)
{
    return lightPosition - triangleIntersectionPoint;
}

float getLightDirection(const vec3& surfaceNormal, const vec3& shadowRay)
{
    return dot(normalize(shadowRay), surfaceNormal);
}

float getOcclusionValue(const vec3& triangleIntersectionPoint, const vec3& shadowRay, float lightRadius, const std::vector<Object>& objects)
{
    float rayLength = length(shadowRay);

    float minDistance = 0.001f;

    float minOcclusionDistance = rayLength;

    for (int i = 0; i < (int) objects.size(); ++i)
    {
        Object object = objects[i];
        for (int j = 0; j < (int) object.triangles.size(); ++j)
        {
            ModelTriangle triangle = object.triangles[j];
            vec3 e0 = triangle.vertices[1] - triangle.vertices[0];
            vec3 e1 = triangle.vertices[2] - triangle.vertices[0];
            vec3 SPVector = triangleIntersectionPoint - triangle.vertices[0];
            mat3 DEMatrix(-normalize(shadowRay), e0, e1);
            vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;

            float t = possibleSolution.x;
            float u = possibleSolution.y;
            float v = possibleSolution.z;

            if (u >= 0.0f && u <= 1.0f &&
                v >= 0.0f && v <= 1.0f &&
                u + v     <=      1.0f &&  
                abs(t)    <  rayLength &&
                t         >  minDistance)
            {
               if (t < minOcclusionDistance) minOcclusionDistance = t;
            }
        }
    }

    return minOcclusionDistance / rayLength;
}

void getShadowData(const vec3& triangleIntersectionPoint, const vec3& surfaceNormal, const std::vector<Light>& lights, const std::vector<Object>& objects,
                   float& occlusionValueResult, float& occlusionRadiusResult, float& lightDirectionResult)
{
    float occlusionValue = 1.0f;
    float occlusionLightRadius = 0.0f;
    float lightDirection = 1.0f;
    for (Light light : lights)
    {
        vec3 shadowRay = getShadowRay(light.position, triangleIntersectionPoint);
        float checkOcclusionValue = getOcclusionValue(triangleIntersectionPoint, shadowRay, light.radius, objects);
        float checklightDirection = getLightDirection(surfaceNormal, shadowRay);
        if (checkOcclusionValue < occlusionValue)
        {
            occlusionValue = checkOcclusionValue;
            occlusionLightRadius = light.radius;
            lightDirection = checklightDirection;
        }
    }

    occlusionValueResult = occlusionValue;
    occlusionRadiusResult = 10.0f * occlusionLightRadius;
    lightDirectionResult = lightDirection;
}

float getShadowStrength(float* occlusionBuffer, float* depthBuffer, float lightSize, float lightAngle, int canvasX, int canvasY, int canvasNX, int canvasNY)
{
    float shadowStrength = 0.0f;

    int canvasPos = canvasX + canvasY * canvasNX;
    float occlusionValue = occlusionBuffer[canvasPos];

    if (occlusionValue < 1.0f)
    {
        if (lightAngle < 0.0f)
        {
            shadowStrength = 1.0f;
        }
        else
        {
            float pixelDepth = depthBuffer[canvasPos];
            int sampleSize = std::abs((1.0f - occlusionValue) * lightSize / pixelDepth);

            int startX = std::max(0, canvasX - sampleSize / 2);
            int endX = std::min(canvasX, canvasX + sampleSize / 2);

            int startY = std::max(0, canvasY - sampleSize / 2);
            int endY = std::min(canvasY, canvasY + sampleSize / 2);

            float max_dist = sampleSize * sampleSize / 4;
            float min_dist_sq = std::numeric_limits<float>::infinity();

            for (int j = startY; j < endY; ++j)
            {
                for (int i = startX; i < endX; ++i)
                {
                    int checkPos = i + j * canvasNX;
                    float checkOcclusionValue = occlusionBuffer[checkPos];
                    if (checkOcclusionValue >= 1.0f)
                    {       
                        float dist_sq = (j - canvasY) * (j - canvasY) + (i - canvasX) * (i - canvasX);
                        if (dist_sq < min_dist_sq) min_dist_sq = dist_sq;
                    }
                }
            }
            shadowStrength = sqrt(min_dist_sq) / sqrt(max_dist);
        }
    }

    return shadowStrength;
}