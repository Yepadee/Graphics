#pragma once

#include <ModelTriangle.h>
#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <glm/glm.hpp>
#include "Image.h"
#include "Interpolation.h"

/**
 * Draw an image to a window.
 *
 * @param img A loaded image.
 * @param window The window the image is to be drawn onto.
 */
void drawImage(const Image& img, DrawingWindow& window)
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

/**
 * Draws a line on a window between two canvas points.
 *
 * @param from The first point of the line.
 * @param to The second point of the line.
 * @param colour A bitpacked RGB colour of the line.
 * @param window The window the image is to be drawn onto.
 */
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

/**
 * Draws a triangle onto the window.
 *
 * @param triangle CanvasTriangle to be drawn.
 * @param colour A bitpacked RGB colour of the line.
 * @param window The window the image is to be drawn onto.
 */
void drawTriangle(const CanvasTriangle& triangle, uint32_t colour, DrawingWindow& window)
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

/**
 * Fills a triangle onto the window.
 *
 * @param triangle CanvasTriangle to be filled.
 * @param window The window the image is to be drawn onto.
 */
void fillTriangle(CanvasTriangle& triangle, DrawingWindow& window)
{
  // Sort the vertices of the triangle from smallest to largest.
  sortVertices(triangle);
  CanvasPoint minPoint = triangle.vertices[0];
  CanvasPoint midPoint = triangle.vertices[1];
  CanvasPoint maxPoint = triangle.vertices[2];

  // Define the y points to interpolate between so we can draw between them. 
  float yStart = minPoint.y;
  float yMid = midPoint.y;
  float yEnd = maxPoint.y;

  // Find the total height of the triangle.
  // Then interpolate down the Long face of the triangle.
  // We do this to find the x values of the long side of
  // the triangle for each of the y values between the
  // top and bottom of the triangle.
  int yDiff = yEnd - yStart;
  std::vector<float> xPointsRHS = interpolate(minPoint.x, maxPoint.x, yDiff);

  // Find the distance between the top of the triangle (min point) and the middle point of the triangle (mid point).
  // Interpolate x's down the top short face of the triangle.
  int yDiffTop = yMid - yStart;
  std::vector<float> xPointsLHS1 = interpolate(minPoint.x, midPoint.x, yDiffTop);

  // Find the distance between the middle point of the triangle (mid point) and the bottom of the triangle (max point).
  // Interpolate x's down the bottom short face of the triangle.
  int yDiffBtm = yEnd - yMid;
  std::vector<float> xPointsLHS2 = interpolate(midPoint.x, maxPoint.x, yDiffBtm);

  uint32_t colour = packRGB(triangle.colour.red, triangle.colour.green, triangle.colour.blue);

  // Fill top triangle.
  // Draw between x's in the top short face on the LHS and x's on the RHS.
  for (float y = yStart; y < yMid; ++y)
  {
    float x1 = xPointsLHS1[(int) (y - yStart)];
    float x2 = xPointsRHS[(int) (y - yStart)];
    drawLine({x1, y}, {x2, y}, colour, window);
  }

  //Fill bottom triangle.
  // Draw between x's in the bottom short face on the LHS and x's on the RHS.
  for (float y = yMid; y < yEnd; ++y)
  {
    float x1 = xPointsLHS2[(int) (y - yMid)];
    float x2 = xPointsRHS[(int) (y - yStart)];
    drawLine({x1, y}, {x2, y}, colour, window);
  }
}

/**
 * Fills a triangle onto the window.
 *
 * @param triangle CanvasTriangle to be filled.
 * @pararm image The image used to texture the triangle.
 * @param window The window the image is to be drawn onto.
 */
void fillTriangleTexture(CanvasTriangle& triangle, const Image& image, DrawingWindow& window)
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