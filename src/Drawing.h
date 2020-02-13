#pragma once

#include <ModelTriangle.h>
#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <glm/glm.hpp>
#include "Image.h"
#include "Interpolation.h"


void drawImage(Image& img, DrawingWindow& window)
{
  for (int y = 0; y < img.getHeight(); ++y)
  {
    for (int x = 0; x < img.getWidth(); ++x)
    {
      uint32_t colour = img.GetPixel(x, y);
      window.setPixelColour(x, y, colour);
    }
  }
}


void drawLine(const CanvasPoint& from, const CanvasPoint& to, uint32_t colour, DrawingWindow& window)
{
  float xDiff = to.x - from.x;
  float yDiff = to.y - from.y;
  float numberOfSteps = std::max(abs(xDiff), std::abs(yDiff));
  float xStepSize = xDiff/numberOfSteps;
  float yStepSize = yDiff/numberOfSteps;
  for (float i=0.0; i<numberOfSteps; i++) {
    float x = from.x + (xStepSize*i);
    float y = from.y + (yStepSize*i);
    if (x > 0 && x < window.width - 1 && y < window.height - 1 && y > 0)
    {
      window.setPixelColour(round(x), round(y), colour);
    }
  }
}

void drawTriangle(CanvasTriangle& triangle, uint32_t colour, DrawingWindow& window)
{
  int j = 2;
  for (int i = 0; i < 3; ++i)
  {
    drawLine(triangle.vertices[i], triangle.vertices[j], colour, window);
    j = i;
  }
}

void sortVertices(CanvasTriangle& triangle)
{
    if (triangle.vertices[1].y < triangle.vertices[0].y) 
       std::swap(triangle.vertices[0], triangle.vertices[1]); 
  
    // Insert arr[2] 
    if (triangle.vertices[2].y < triangle.vertices[1].y) 
    { 
       std::swap(triangle.vertices[1], triangle.vertices[2]); 
       if (triangle.vertices[1].y < triangle.vertices[0].y) 
          std::swap(triangle.vertices[1], triangle.vertices[0]); 
    }
}

void fillTriangle(CanvasTriangle& triangle, DrawingWindow& window)
{
  sortVertices(triangle);
  CanvasPoint minPoint = triangle.vertices[0];
  CanvasPoint midPoint = triangle.vertices[1];
  CanvasPoint maxPoint = triangle.vertices[2];

  float yStart = minPoint.y;
  float yMid = midPoint.y;
  float yEnd = maxPoint.y;

  int yDiff = yEnd - yStart;
  std::vector<float> xPointsRHS = interpolate(minPoint.x, maxPoint.x, yDiff);

  int yDiffTop = yMid - yStart;
  std::vector<float> xPointsLHS1 = interpolate(minPoint.x, midPoint.x, yDiffTop);

  int yDiffBtm = yEnd - yMid;
  std::vector<float> xPointsLHS2 = interpolate(midPoint.x, maxPoint.x, yDiffBtm);

  uint32_t colour = packRGB(triangle.colour.red, triangle.colour.green, triangle.colour.blue);

  //Fill top triangle.
  for (float y = yStart; y < yMid; ++y)
  {
    float x1 = xPointsLHS1[(int) (y - yStart)];
    float x2 = xPointsRHS[(int) (y - yStart)];
    drawLine({x1, y}, {x2, y}, colour, window);
  }

  //Fill bottom triangle.
  for (float y = yMid; y < yEnd; ++y)
  {
    float x1 = xPointsLHS2[(int) (y - yMid)];
    float x2 = xPointsRHS[(int) (y - yStart)];
    drawLine({x1, y}, {x2, y}, colour, window);
  }
}

void fillTriangleTexture(CanvasTriangle& triangle, Image& image, DrawingWindow& window)
{
  sortVertices(triangle);
  CanvasPoint minPoint = triangle.vertices[0];
  CanvasPoint midPoint = triangle.vertices[1];
  CanvasPoint maxPoint = triangle.vertices[2];

  float yStart = minPoint.y;
  float yMid = midPoint.y;
  float yEnd = maxPoint.y;

  int yDiff = yEnd - yStart;
  std::vector<float> xPointsRHS = interpolate(minPoint.x, maxPoint.x, yDiff);
  std::vector<float> xPointsRHST = interpolate(minPoint.texturePoint.x, maxPoint.texturePoint.x, yDiff);
  
  std::vector<float> yPointsT = interpolate(minPoint.texturePoint.y, maxPoint.texturePoint.y, yDiff);


  int yDiffTop = yMid - yStart;
  std::vector<float> xPointsLHS1 = interpolate(minPoint.x, midPoint.x, yDiffTop);
  std::vector<float> xPointsLHS1T = interpolate(minPoint.texturePoint.x, midPoint.texturePoint.x, yDiffTop);


  int yDiffBtm = yEnd - yMid;
  std::vector<float> xPointsLHS2 = interpolate(midPoint.x, maxPoint.x, yDiffBtm);
  std::vector<float> xPointsLHS2T = interpolate(midPoint.texturePoint.x, maxPoint.texturePoint.x, yDiffBtm);

  //Fill top triangle.
  for (float y = yStart; y < yMid; ++y)
  {
    int iy = (int) (y - yStart);
    float x1 = xPointsLHS1[iy];
    float x2 = xPointsRHS[iy];

    float x1T = xPointsLHS1T[iy];
    float x2T = xPointsRHST[iy];

    float xDiff = x2 - x1;

    std::vector<float> pixelRow = interpolate(x1T, x2T, xDiff);

    for (float x = x1; x <= x2; ++x)
    {
      int ix = (int) (x - x1);

      float xt = pixelRow[ix];
      float yt = yPointsT[iy];

      uint32_t pixel = image.GetPixel(xt, yt);
      window.setPixelColour(x, y, pixel);
    }
  }

  //Fill bottom triangle.
  for (float y = yMid; y < yEnd; ++y)
  {
    int iy = (int) (y - yStart);
    int iyL = (int) (y - yMid);

    float x1 = xPointsLHS2[iyL];
    float x2 = xPointsRHS[iy];

    float x1T = xPointsLHS2T[iyL];
    float x2T = xPointsRHST[iy];

    float xDiff = x2 - x1;

    std::vector<float> pixelRow = interpolate(x1T, x2T, xDiff);

    for (float x = x1; x < x2; ++x)
    {
      int ix = (int) (x - x1);

      float xt = pixelRow[ix];
      float yt = yPointsT[iy];

      uint32_t pixel = image.GetPixel(xt, yt);
      window.setPixelColour(x, y, pixel);
    }
  }
}