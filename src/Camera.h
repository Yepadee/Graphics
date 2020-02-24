#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ModelTriangle.h>
#include <CanvasTriangle.h>

using namespace glm;

/**
 * Construct homogenious world-to-camera matrix.
 *
 * @param pos A 3D position vector of camera.
 * @param angle A 3D angle vector of camera.
 * @return 4x4 homogenious matrix that maps vertices in world space to vertices on camera space.
 */
mat4x4 constructCameraSpace(const vec3& pos, const vec3& angle)
{
    // Alias angle components.
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

void translate(glm::mat4x4& cameraToWorld, const vec3& translation)
{
    cameraToWorld[3][0] += translation[0];
    cameraToWorld[3][1] += translation[1];
    cameraToWorld[3][2] += translation[2];
}

void rotateX(glm::mat4x4& cameraToWorld, float X)
{
    float values[16] = {
     1.0f, 0.0f  , 0.0f   , 0.0f,
     0.0f, cos(X), -sin(X), 0.0f,
     0.0f, sin(X), cos(X) , 0.0f,
     0.0f, 0.0f  , 0.0f   , 1.0f
    };

    cameraToWorld *= transpose(make_mat4(values));
}

void rotateY(glm::mat4x4& cameraToWorld, float Y)
{
    float values[16] = {
     cos(Y) , 0.0f, sin(Y), 0.0f,
     0.0f   , 1.0f, 0.0f  , 0.0f,
     -sin(Y), 0.0f, cos(Y), 0.0f,
     0.0f   , 0.0f, 0.0f  , 1.0f
    };

    cameraToWorld *= transpose(make_mat4(values));
}

void rotateZ(glm::mat4x4& cameraToWorld, float Z)
{
    float values[16] = {
     cos(Z), -sin(Z), 0.0f, 0.0f,
     sin(Z), cos(Z) , 0.0f, 0.0f,
     0.0f  , 0.0f   , 1.0f, 0.0f,
     0.0f  , 0.0f   , 0.0f, 1.0f
    };

    cameraToWorld *= transpose(make_mat4(values));
}

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
CanvasPoint project2D(const vec3& pointWorldSpace, const mat4x4& worldToCamera, float focalLength,
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
    pRaster.depth = -pointCamSpace.z;

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