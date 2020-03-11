#pragma once

#include <math.h>
#include <cmath>
#include <glm/glm.hpp>
#include <ModelTriangle.h>
#include <RayTriangleIntersection.h>

#include <vector>

#include "PixelUtil.h"

using namespace glm;

/**
 * Calculate proximity light coefficient.
 *
 * @param rti Ray-triangle intersection.
 * @param lightSource The location and brightness of the light.
 * @return Brightness of point.
 */
float applyProximityLight(RayTriangleIntersection rti, vec4 lightSource)
{   
    glm::vec3 diff = rti.intersectionPoint - vec3(lightSource);
    float distSqr = dot(diff, diff);
    float strength = lightSource.w;
    return ((strength * 1.0f)/(4.0f * distSqr * M_PI));
}

/**
 * Calculate angle of incidence light coefficient.
 *
 * @param rti Ray-triangle intersection.
 * @param lightSource The location and brightness of the light.
 * @return Brightness of point.
 */
float applyAOILight(RayTriangleIntersection rti, vec4 lightSource)
{
    vec3 normal = cross(rti.intersectedTriangle.vertices[1] - rti.intersectedTriangle.vertices[0], 
                        rti.intersectedTriangle.vertices[2] - rti.intersectedTriangle.vertices[0]);
    
    vec3 ray = vec3(lightSource) - rti.intersectionPoint;

    float aoiLight = dot(normalize(normal), normalize(ray));
    return std::max(aoiLight, 0.0f);
}

/**
 * Calculate diffuse light coefficient.
 *
 * @param rti Ray-triangle intersection.
 * @param lightSource The location and brightness of the light.
 * @return Brightness of point.
 */
float applyDiffuse(RayTriangleIntersection rti, vec4 lightSource)
{
    return applyProximityLight(rti, lightSource) * applyAOILight(rti, lightSource);
}

vec3 reflect(const vec3 &I, const vec3 &N) 
{ 
    return I - 2 * dot(I, N) * N; 
} 

/**
 * Calculate specular light coefficient.
 *
 * @param rti Ray-triangle intersection.
 * @param lightSource The location and brightness of the light.
 * @return Brightness of point.
 */
float applySpecularLight(RayTriangleIntersection rti, vec4 lightSource, vec3 cameraRay)
{
    float n = 10.0f;

    vec3 normal = cross(rti.intersectedTriangle.vertices[1] - rti.intersectedTriangle.vertices[0], 
                        rti.intersectedTriangle.vertices[2] - rti.intersectedTriangle.vertices[0]);

    vec3 lightRay = rti.intersectionPoint - vec3(lightSource);

    vec3 reflected = reflect(lightRay, normal);

    float magnitude = dot(normalize(reflected), -normalize(cameraRay));

    return std::pow(std::max(0.0f, magnitude), n);
}

/**
 * Apply ambiant light to calculated coefficient.
 *
 * @param brightness The calculated brightness of a point.
 * @param ambiance The minimum brightness of a point.
 * @return Brightness of point.
 */
float applyAmbiantLight(float brightness, float ambiance)
{
    return std::min(1.0f, std::max(ambiance, brightness));
}

/**
 * Apply brightness to a point.
 *
 * @param colour The colour of the point.
 * @param brightness The brightness of the point.
 * @return Colour of points under light.
 */
uint32_t applyBrightness(Colour colour, float brightness)
{
    float r = colour.red * brightness;
    float g = colour.green * brightness;
    float b = colour.blue * brightness;
    return packRGB(r,g,b);
}

/**
 * Calculate and apply brightness to a point from all light sources.
 *
 * @param rti Ray-triangle intersection.
 * @param lights The locations and brightnesses of the light sources.
 * @return The illumnated point colour.
 */
uint32_t illuminatePoint(RayTriangleIntersection rti, vec3 cameraRay, std::vector<vec4> lights)
{
    float ambiance = 0.2f;
    float brightness = 0.0f;
    for (vec4 lightSource : lights){
        brightness += applyDiffuse(rti, lightSource);
        brightness += applySpecularLight(rti, lightSource, cameraRay);
    }
    brightness = applyAmbiantLight(brightness, ambiance);
    uint32_t colour = applyBrightness(rti.intersectedTriangle.colour, brightness);

    return colour;
}