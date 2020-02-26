#pragma once

#include <inttypes.h>
#include <DrawingWindow.h>
#include "Interpolation.h"
#include "PixelUtil.h"

float* depthBuffer;
int dbWidth;
int dbHeight;

void clearDepthBuffer()
{
  for (int jj = 0; jj < dbHeight; ++jj)
  {
    for (int ii = 0; ii < dbWidth; ++ii)
    {
      depthBuffer[ii + dbWidth*jj] = 0;
    }
  }
}

void initDepthBuffer(int width, int height)
{
  dbWidth = width;
  dbHeight = height;
  depthBuffer = new float[dbWidth*dbHeight];
  clearDepthBuffer();
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
  if (from.depth < 0 || to.depth < 0) return;
  float xDiff = to.x - from.x;
  float yDiff = to.y - from.y;
  float depthDiff = to.depth - from.depth;

  float numberOfSteps = std::max(abs(xDiff), std::abs(yDiff));
  float xStepSize = xDiff/numberOfSteps;
  float yStepSize = yDiff/numberOfSteps;
  float depthStepSize = depthDiff/numberOfSteps;

  for (float i=0.0; i<numberOfSteps; i++) {
    float x = from.x + (xStepSize*i);
    float y = from.y + (yStepSize*i);
    float depth = from.depth + (depthStepSize*i);

    if (depth > depthBuffer[(int)(floor(x) + dbWidth * floor(y))])
    {
      depthBuffer[(int)(floor(x) + dbWidth * floor(y))] = depth;
      window.setPixelColour(floor(x), floor(y), colour);
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

void fitToWindow(CanvasPoint& point, const DrawingWindow& window)
{
  if (point.x > window.width) point.x = window.width - 2;
  if (point.y > window.height) point.y = window.height - 2;

  if (point.x < 0) point.x = 0;
  if (point.y < 0) point.y = 0;
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
  std::vector<float> zPointsRHS = interpolate(minPoint.depth, maxPoint.depth, yDiff);

  // Find the distance between the top of the triangle (min point) and the middle point of the triangle (mid point).
  // Interpolate x's down the top short face of the triangle.
  int yDiffTop = yMid - yStart;
  std::vector<float> xPointsLHS1 = interpolate(minPoint.x, midPoint.x, yDiffTop);
  std::vector<float> zPointsLHS1 = interpolate(minPoint.depth, midPoint.depth, yDiffTop);

  // Find the distance between the middle point of the triangle (mid point) and the bottom of the triangle (max point).
  // Interpolate x's down the bottom short face of the triangle.
  int yDiffBtm = yEnd - yMid;
  std::vector<float> xPointsLHS2 = interpolate(midPoint.x, maxPoint.x, yDiffBtm);
  std::vector<float> zPointsLHS2 = interpolate(midPoint.depth, maxPoint.depth, yDiffBtm);

  uint32_t colour = packRGB(triangle.colour.red, triangle.colour.green, triangle.colour.blue);

  // Fill top triangle.
  // Draw between x's in the top short face on the LHS and x's on the RHS.
  for (float y = yStart; y < yMid; ++y)
  {
    float x1 = xPointsLHS1[(int) (y - yStart)];
    float x2 = xPointsRHS[(int) (y - yStart)];
    float z1 = zPointsLHS1[(int) (y - yStart)];
    float z2 = zPointsRHS[(int) (y - yStart)];

    CanvasPoint p1 = {x1, y, z1};
    CanvasPoint p2 = {x2, y, z2};

    fitToWindow(p1, window);
    fitToWindow(p2, window);

    drawLine(p1, p2, colour, window);
  }

  //Fill bottom triangle.
  // Draw between x's in the bottom short face on the LHS and x's on the RHS.
  for (float y = yMid; y < yEnd; ++y)
  {
    float x1 = xPointsLHS2[(int) (y - yMid)];
    float x2 = xPointsRHS[(int) (y - yStart)];
    float z1 = zPointsLHS2[(int) (y - yMid)];
    float z2 = zPointsRHS[(int) (y - yStart)];

    CanvasPoint p1 = {x1, y, z1};
    CanvasPoint p2 = {x2, y, z2};

    fitToWindow(p1, window);
    fitToWindow(p2, window);

    drawLine(p1, p2, colour, window);
  }
}