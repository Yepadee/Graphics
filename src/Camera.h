#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ModelTriangle.h>
#include <CanvasTriangle.h>

using namespace glm;

/**
 * Construct homogenious world-to-camera matrix
 * using the camera's position and a point the
 * camera is looking at.
 *
 * @param from A 3D position vector of camera.
 * @param to A 3D position the camera is looking at.
 * @return 4x4 homogenious matrix that maps vertices in world space to vertices on camera space.
 */
mat4x4 lookAt(const vec3& from, const vec3& to)
{
    vec3 fwd = normalize(from - to);
    vec3 yUp(0,1,0);
    vec3 right = cross(normalize(yUp), fwd);
    vec3 up = cross(fwd, right);

    float values[16] = {
     right.x, up.x,  fwd.x, from.x,
     right.y, up.y,  fwd.y, from.y,
     right.z, up.z,  fwd.z, from.z,
     0.0f   , 0.0f,  0.0f , 1.0f
    };
    
    return transpose(make_mat4(values));
}

/**
 * Rotate's the camera about a point.
 *
 * @param centre The centre point of rotation.
 * @param radius The radius of the rotation.
 * @param angle The angle of rotation
 * @return 4x4 homogenious matrix that maps vertices in world space to vertices on camera space.
 */
mat4x4 rotateAbout(const vec3& centre, float radius, float angle)
{
    vec3 from(centre.x + radius * sin(angle), centre.y, centre.z + radius * cos(angle));
    return lookAt(from, centre);
}

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

void rotateX(mat4x4& cameraToWorld, float X)
{
    float values[16] = {
     1.0f, 0.0f  , 0.0f   , 0.0f,
     0.0f, cos(X), -sin(X), 0.0f,
     0.0f, sin(X), cos(X) , 0.0f,
     0.0f, 0.0f  , 0.0f   , 1.0f
    };

    cameraToWorld *= transpose(make_mat4(values));
}

void rotateY(mat4x4& cameraToWorld, float Y)
{
    float values[16] = {
     cos(Y) , 0.0f, sin(Y), 0.0f,
     0.0f   , 1.0f, 0.0f  , 0.0f,
     -sin(Y), 0.0f, cos(Y), 0.0f,
     0.0f   , 0.0f, 0.0f  , 1.0f
    };

    cameraToWorld *= transpose(make_mat4(values));
}

void rotateZ(mat4x4& cameraToWorld, float Z)
{
    float values[16] = {
     cos(Z), -sin(Z), 0.0f, 0.0f,
     sin(Z), cos(Z) , 0.0f, 0.0f,
     0.0f  , 0.0f   , 1.0f, 0.0f,
     0.0f  , 0.0f   , 0.0f, 1.0f
    };

    cameraToWorld *= transpose(make_mat4(values));
}

void translate(mat4x4& cameraToWorld, const vec3& translation)
{
    cameraToWorld[3][0] += translation[0];
    cameraToWorld[3][1] += translation[1];
    cameraToWorld[3][2] += translation[2];
}