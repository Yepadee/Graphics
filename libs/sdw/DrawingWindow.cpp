#include "DrawingWindow.h"

// Simple constructor method
DrawingWindow::DrawingWindow()
{
}

// Complex constructor method
DrawingWindow::DrawingWindow(int w, int h, bool fullscreen)
{
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) !=0) {
    printMessageAndQuit("Could not initialise SDL: ", SDL_GetError());
  }

  width = w;
  height = h;
  pixelBuffer = new uint32_t[width*height];
  clearPixels();
}

// Deconstructor method
void DrawingWindow::destroy()
{
  delete[] pixelBuffer;
}

uint32_t* DrawingWindow::getPixelBuffer()
{
  return pixelBuffer;
}

void DrawingWindow::setPixelColour(int x, int y, uint32_t colour)
{
  if((x<0) || (x>=width) || (y<0) || (y>=height)) {
    std::cout << x << "," <<  y << " not on visible screen area" << std::endl;
  }
  else pixelBuffer[(y*width)+x] = colour;
}

uint32_t DrawingWindow::getPixelColour(int x, int y) const
{
  if((x<0) || (x>=width) || (y<0) || (y>=height)) {
    std::cout << x << "," <<  y << " not on visible screen area" << std::endl;
    return -1;
  }
  else return pixelBuffer[(y*width)+x];
}

void DrawingWindow::clearPixels()
{
  memset(pixelBuffer, 0, width * height * sizeof(uint32_t));
}
