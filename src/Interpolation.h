#pragma once

#include <vector>
#include <glm/glm.hpp>

using namespace glm;


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