#pragma once

#include <math.h>
#include <cmath>
#include <glm/glm.hpp>
#include <ModelTriangle.h>
#include <RayTriangleIntersection.h>

#include <vector>

#include "PixelUtil.h"
#include "Shadows.h"

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
float applyProximityLight(RayTriangleIntersection rti, Light light)
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
float applyAOILight(RayTriangleIntersection rti, Light light)
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
float applyDiffuse(RayTriangleIntersection rti, Light light)
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
float applySpecularLight(RayTriangleIntersection rti, Light light, vec3 cameraRay)
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
uint32_t applyBrightness(Colour colour, float rl, float gl, float bl)
{
    float r = colour.red * rl;
    float g = colour.green * gl;
    float b = colour.blue * bl;
    return packRGB(r,g,b);
}

/**
 * Calculate and apply brightness to a point from all light sources.
 *
 * @param rti Ray-triangle intersection.
 * @param lights The locations and brightnesses of the light sources.
 * @return The illumnated point colour.
 */
uint32_t illuminatePoint(RayTriangleIntersection rti, vec3 cameraRay, std::vector<Object> objects, std::vector<Light> lights)
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


    // Find darkness of the point
    float darkness = 1.0f;
    for (Light light: lights)
    {
        float lightShadowIntensity = getShadowIntensityVS(rti, light.position, light.radius, objects);
        darkness = std::min(darkness, lightShadowIntensity);
    }

    // Apply darkness
    brightness *= (1.0f - darkness);

    // Apply ambiant light
    brightness = applyAmbiantLight(brightness, ambiance);


    rl *= brightness;
    gl *= brightness;
    bl *= brightness;

    uint32_t colour = applyBrightness(rti.intersectedTriangle.colour, rl, gl, bl);

    return colour;
}