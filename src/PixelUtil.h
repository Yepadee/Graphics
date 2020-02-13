#include <inttypes.h>
#include <glm/glm.hpp>

uint32_t packRGB(int r, int g, int b)
{
  return (255<<24) + (r<<16) + (g<<8) + b;
}

uint32_t packRGB(glm::vec3 rgb)
{
  return (255<<24) + (int(rgb.x)<<16) + (int(rgb.y)<<8) + int(rgb.z);
}

uint32_t packGreyscale(int n)
{
  return (255<<24) + (int(n)<<16) + (int(n)<<8) + int(n);
}