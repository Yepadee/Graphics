#pragma once

#include <vector>
#include <glm/glm.hpp>

using namespace glm;

/**
 * Interpolates between two floating point numbers
 *
 * @param from The starting point.
 * @param to The ending point.
 * @param numValues The number of values to obtain beteen the starting and ending points.
 * @return A vector containing the interpolated values
 */
std::vector<float> interpolate(float from, float to, float numValues)
{
  std::vector<float> values;

  float incr = (to - from) / (numValues - 1.0f);

  float acc = from;
  for (float i = 0; i <= numValues; ++i)
  {
    values.push_back(acc);
    acc += incr;
  }

  return values;
}

/**
 * Interpolates between two 3D vectors.
 *
 * @param from The starting point.
 * @param to The ending point.
 * @param numValues The number of values to obtain beteen the starting and ending points.
 * @return A vector containing the interpolated values
 */
std::vector<vec3> interpolate(vec3 from, vec3 to, float numValues)
{
  std::vector<vec3> values;
  
  vec3 incr = (to - from) / (vec3(numValues - 1.0f));

  vec3 acc = from;
  for (float i = 0; i <= numValues; ++i)
  {
    values.push_back(acc);
    acc += incr;
  }

  return values;
}