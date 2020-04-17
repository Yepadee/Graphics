#pragma once
#include <inttypes.h>
#include <glm/glm.hpp>
#include <Colour.h>

#include <vector>

uint32_t packRGB(int r, int g, int b)
{
  return (255<<24) + (r<<16) + (g<<8) + b;
}

uint32_t packRGBC(unsigned char r, unsigned char g, unsigned char b)
{
  return (255<<24) + (r<<16) + (g<<8) + b;
}

uint32_t packRGB(glm::vec3 rgb)
{
  return (255<<24) + (int(rgb.x)<<16) + (int(rgb.y)<<8) + int(rgb.z);
}

uint32_t packRGB(Colour colour)
{
  return (255<<24) + (int(colour.red)<<16) + (int(colour.green)<<8) + int(colour.blue);
}

uint32_t packGreyscale(int n)
{
  return (255<<24) + (int(n)<<16) + (int(n)<<8) + int(n);
}

uint32_t getSubPixel(uint32_t pixel, int i)
{
  return (pixel & (255 << i * 8)) >> (i * 8);
}

uint32_t averagePixels(std::vector<uint32_t> pixels, std::vector<int> weights)
{
  int sumRed = 0;
  int sumGreen = 0;
  int sumBlue = 0;
  int sumWeight = 0;

  int numPixels = (int) pixels.size();

  for (int i = 0; i < numPixels; ++i)
  {
    uint32_t pixel = pixels[i];
    int weight = weights[i];

    sumWeight += weight;

    sumRed += weight * getSubPixel(pixel, 2);
    sumGreen += weight * getSubPixel(pixel, 1);
    sumBlue += weight * getSubPixel(pixel, 0);
  }

  

  return packRGB(sumRed / sumWeight, sumGreen / sumWeight, sumBlue / sumWeight);
}