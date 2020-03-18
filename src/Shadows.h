#pragma once

#include <math.h>
#include <cmath>
#include <glm/glm.hpp>
#include <ModelTriangle.h>
#include <RayTriangleIntersection.h>

#include <vector>

#include "Object.h"

using namespace glm;


/**
 * Determine if a point is illuminated by a light source.
 *
 * @param rti Ray-triangle intersection.
 * @param lightSource The location and brightness of the light.
 * @param objects A list of all object that may be between the point and the lightsource.
 * @return Wether or not a point is illuminated by the lightsource.
 */
bool isIlluminated(const vec3& triangleIntersectionPoint, int objectId, int triangleId, const vec4& lightSource,
                   const std::vector<Object>& objects)
{
    vec3 shadowRay =  vec3(lightSource) - triangleIntersectionPoint;
    float rayLength = length(shadowRay);

    float minDistance = 0.01f;

    for (int i = 0; i < (int) objects.size(); ++i)
    {
        Object object = objects[i];
        for (int j = 0; j < (int) object.triangles.size(); ++j)
        {
            //if (i == objectId && j == triangleId) continue;
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

/**
 * Determine if a point is illuminated by a light source.
 *
 * @param rti Ray-triangle intersection.
 * @param lightSource The location and brightness of the light.
 * @param objects A list of all object that may be between the point and the lightsource.
 * @return Wether or not a point is illuminated by the lightsource.
 */
float getShadowIntensity(const RayTriangleIntersection& rti, const vec4& lightSource,
                         const std::vector<Object>& objects)
{   float shadowIntensity = 0.0f;

    float intensityCoeff = 1.0f;
    float surfaceShift = 0.5f;

    vec3 surfaceNormal = normalize(cross(rti.intersectedTriangle.vertices[1] - rti.intersectedTriangle.vertices[0], 
                               rti.intersectedTriangle.vertices[2] - rti.intersectedTriangle.vertices[0]));

    vec3 surfaceDisplacement = surfaceShift * surfaceNormal;

    vec3 lowSurfacePoint  = rti.intersectionPoint - surfaceDisplacement;
    vec3 midSurfacePoint  = rti.intersectionPoint;
    vec3 highSurfacePoint = rti.intersectionPoint + surfaceDisplacement;

    bool lowDark  = !isIlluminated(lowSurfacePoint , rti.objectId, rti.triangleId, lightSource, objects);
    bool midDark  = !isIlluminated(midSurfacePoint , rti.objectId, rti.triangleId, lightSource, objects);
    bool highDark = !isIlluminated(highSurfacePoint, rti.objectId, rti.triangleId, lightSource, objects);
    
    
    if (lowDark && midDark && highDark) shadowIntensity = 1.0f;
    else if (lowDark && !highDark)
    {
        
        float stepSize = 0.02f;
        float displacement = 0.0f;

        vec3 checkSurfacePoint = lowSurfacePoint;

        while (!isIlluminated(checkSurfacePoint, rti.objectId, rti.triangleId, lightSource, objects))
        {
            checkSurfacePoint = lowSurfacePoint + surfaceNormal * displacement;
            displacement += stepSize;
        }

        shadowIntensity = intensityCoeff * std::min(1.0f, abs(pow(displacement, 1.5f)));
        //if (shadowIntensity > 0.0f) std::cout << "shadowIntensity: " << shadowIntensity << std::endl;
        //shadowIntensity = 0.5f;
        
    }

    //if (midDark) shadowIntensity = 1.0f;
    
    return shadowIntensity;
}