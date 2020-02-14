#pragma once

#include <inttypes.h>
#include <DrawingWindow.h>

void drawCanvasPoint(int x, int y, uint32_t colour, int depth, float* depthBuffer, DrawingWindow& window)
{
  if (depth < depthBuffer[x + y * window.width])
  {
    depthBuffer[x + y * window.width] = depth;
    window.setPixelColour(std::round(x), std::round(y), colour);
  }
}