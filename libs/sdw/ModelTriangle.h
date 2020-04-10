#pragma once

#include <glm/glm.hpp>
#include "Colour.h"
#include <string>

#include <vector>

#define NORMAL_MAP_SIZE 32

class ModelTriangle
{
  public:
    bool hasVertexNormals;
    bool hasTexture;
    
    glm::vec3 vertices[3];
    Colour colour;
    glm::vec3 normal;
    glm::vec3 vertexNormals[3];
    glm::vec2 textureVertices[3];

    ModelTriangle()
    {
    }

    ModelTriangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, Colour trigColour)
    {
      vertices[0] = v0;
      vertices[1] = v1;
      vertices[2] = v2;
      colour = trigColour;
      normal = (glm::cross(v1 - v0, v2 - v0));
      hasVertexNormals = false;
      hasTexture = false;
    }

    void setVertexNormals(glm::vec3 n0, glm::vec3 n1, glm::vec3 n2)
    {
      vertexNormals[0] = n0;
      vertexNormals[1] = n1;
      vertexNormals[2] = n2;
      hasVertexNormals = true;
    }

    void setTexturePoints(glm::vec2 t0, glm::vec2 t1, glm::vec2 t2)
    {
      textureVertices[0] = t0;
      textureVertices[1] = t1;
      textureVertices[2] = t2;
      hasTexture = true;
    }
};

std::ostream& operator<<(std::ostream& os, const ModelTriangle& triangle)
{
    os << "(" << triangle.vertices[0].x << ", " << triangle.vertices[0].y << ", " << triangle.vertices[0].z << ")" << std::endl;
    os << "(" << triangle.vertices[1].x << ", " << triangle.vertices[1].y << ", " << triangle.vertices[1].z << ")" << std::endl;
    os << "(" << triangle.vertices[2].x << ", " << triangle.vertices[2].y << ", " << triangle.vertices[2].z << ")" << std::endl;
    os << std::endl;
    return os;
}

std::ostream& operator<<(std::ostream& os, const glm::vec3& vec)
{
    os << "{" << vec.x << ", " << vec.y << ", " << vec.z << "}";
    return os;
}

bool operator==(const ModelTriangle &lhs, const ModelTriangle &rhs)
{
  bool equal = true;
  for (int i = 0; i < 3; i++)
  {
    equal &= glm::all(glm::equal(lhs.vertices[i], rhs.vertices[i]));
  }

  return equal;
}

bool operator!=(const ModelTriangle &lhs, const ModelTriangle &rhs)
{
  bool equal = true;
  for (int i = 0; i < 3; i++)
  {
    equal &= glm::all(glm::notEqual(lhs.vertices[i], rhs.vertices[i]));
  }

  return equal;
}
