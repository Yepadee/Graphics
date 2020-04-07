#pragma once

#include <glm/glm.hpp>
#include "Colour.h"
#include <string>

#include <vector>

#define NORMAL_MAP_SIZE 32

class ModelTriangle
{
  public:
    glm::vec3 vertices[3];
    Colour colour;
    glm::vec3 normal;

    glm::vec3 vertexNormals[3];

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
