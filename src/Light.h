#pragma once

#include <math.h>
#include <cmath>
#include <glm/glm.hpp>
#include <ModelTriangle.h>
#include <RayTriangleIntersection.h>

#include <vector>

#include "PixelUtil.h"

using namespace glm;

struct Light
{
    vec3 position;
    float intensity;
    float radius;
    float r, g, b;

    Light(vec3 position, float intensity, float radius, float r, float g, float b)
    : position(position)
    , intensity(intensity)
    , radius(radius)
    , r(r)
    , g(g)
    , b(b)
    {}
};

/**
 * Calculate proximity light coefficient.
 *
 * @param rti Ray-triangle intersection.
 * @param lightSource The location and brightness of the light.
 * @return Brightness of point.
 */
float applyProximityLight(const RayTriangleIntersection& rti, const Light& light)
{   
    glm::vec3 diff = rti.intersectionPoint - light.position;
    float distSqr = dot(diff, diff);
    float strength = 150.0f;
    return strength / (4.0f * distSqr * M_PI);
}

/**
 * Calculate angle of incidence light coefficient.
 *
 * @param rti Ray-triangle intersection.
 * @param lightSource The location and brightness of the light.
 * @return Brightness of point.
 */
float applyAOILight(const RayTriangleIntersection& rti, const Light& light)
{
    vec3 normal = rti.normal;
    
    vec3 ray = light.position - rti.intersectionPoint;

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
float applyDiffuse(const RayTriangleIntersection& rti, const Light& light)
{
    return applyAOILight(rti, light) * applyProximityLight(rti, light);
}

vec3 reflectRay(const vec3 &ray, const vec3 &normal) 
{ 
    return ray - 2 * dot(ray, normal) * normal; 
} 

/**
 * Calculate specular light coefficient.
 *
 * @param rti Ray-triangle intersection.
 * @param lightSource The location and brightness of the light.
 * @return Brightness of point.
 */
float applySpecularLight(const RayTriangleIntersection& rti, const Light& light, const vec3& cameraRay)
{
    float n = 10.0f;

    vec3 normal = rti.normal;
    vec3 lightRay = rti.intersectionPoint - light.position;
    vec3 reflected = reflectRay(lightRay, normal);
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
vec3 applyBrightness(const Colour& colour, float rl, float gl, float bl)
{
    float r = colour.red * rl;
    float g = colour.green * gl;
    float b = colour.blue * bl;
    return vec3(r,g,b);
}

/**
 * Calculate and apply brightness to a point from all light sources.
 *
 * @param rti Ray-triangle intersection.
 * @param lights The locations and brightnesses of the light sources.
 * @return The illumnated point colour.
 */
vec3 getPointBrightess(const RayTriangleIntersection& rti, const vec3& cameraRay, const std::vector<Object>& objects, const std::vector<Light>& lights)
{
    float ambiance = 0.2f;
    float brightness = 0.0f;

    float rl = 0.0f;
    float gl = 0.0f;
    float bl = 0.0f;

    // Find brightness of the point
    for (Light light : lights)
    {
        brightness += applyDiffuse(rti, light);
        brightness += applySpecularLight(rti, light, cameraRay);

        rl += light.r;
        gl += light.g;
        bl += light.b;
    }

    rl = min(1.0f, rl);
    gl = min(1.0f, gl);
    bl = min(1.0f, bl);

    // Apply ambiant light
    brightness = applyAmbiantLight(brightness, ambiance);

    rl *= brightness;
    gl *= brightness;
    bl *= brightness;

    return vec3(rl, gl, bl);
}

vec3 getReflectedColour(const RayTriangleIntersection& rti, const vec3& cameraRay, const std::vector<Object>& objects)
{

}