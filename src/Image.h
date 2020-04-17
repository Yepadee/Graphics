#pragma once

#include <fstream>
#include <sstream>

#include "PixelUtil.h"

class Image
{
private:
  int width, height;
  uint32_t* payload;

public:
  Image() {}

  Image(int width, int height, uint32_t* payload)
  : width(width)
  , height(height)
  , payload(payload)
  {}

  uint32_t GetPixel(int x, int y) const
  {
    int i = y * width + x;
    return payload[i];
  }

  int getWidth() const
  {
    return width;
  }

  int getHeight() const
  {
    return height;
  }
};

int readNumber(FILE *fptr)
{
  char buffer[8]; 
  if (!fscanf(fptr, "%s\n", buffer)) return EXIT_FAILURE;
  return atoi(buffer);
}

/**
 * Loads a PPM file.
 *
 * @param filename The location of the ppm file.
 * @return An image container, containing the payload and size of the image.
 */
Image loadPPM(const char* fileName)
{
  Image image;
  int imageSize;

  std::ifstream ifs(fileName, std::ifstream::in);

  std::string matFilepath;
  std::string buffer;

  // Read magic no.
  ifs >> buffer;

  //Check for comment
  ifs >> buffer;
  if (buffer[0] == '#') std::getline(ifs, buffer);

  // Read width
  ifs >> buffer;
  int width = std::stoi(buffer);

  // Read width
  ifs >> buffer;
  int height = std::stoi(buffer);

  ifs >> buffer;

  // Read width
  int maxval = std::stoi(buffer);

  // Copy payload
  imageSize = width * height;
  uint32_t* payload = (uint32_t*) malloc(imageSize * sizeof(uint32_t));

 
  char* pixelBuffer = new char [imageSize * 3];
  ifs.read(pixelBuffer, imageSize * 3);
  for (int i = 0; i < imageSize; ++i)
  { 
    unsigned char r = pixelBuffer[3 * i + 0];
    unsigned char g = pixelBuffer[3 * i + 1];
    unsigned char b = pixelBuffer[3 * i + 2];

    uint32_t colour = packRGBC(r,g,b); 
    payload[i] = colour;
  }

  ifs.close();

  return Image(width, height, payload);
}

void saveFrame(DrawingWindow& drawingWindow, int frameNo)
{
  
  int width = drawingWindow.width;
  int height = drawingWindow.height;

  std::string filename = "";
  filename += "renders/frame" + std::to_string(frameNo) + ".ppm";

  std::cout << "saving frame: " << filename << std::endl;

  std::ofstream file;
  file.open(filename);

  file << "P6" << std::endl;
  file << "# Created using James and Tommy\'s amazing frame renderer!" << std::endl;
  file << width << " " << height << std::endl;
  file << "255" << std::endl;

  /*
  file.write("P6\n");
  file.write(width + " " + height + "\n"); file.write("255\n");
  for (int i=0; i<pixels.length; y++) {
    file.write(getRed(pixel[i])); 
    file.write(getGreen(pixel[i])); 
    file.write(getBlue(pixel[i]));
  } 
  file.close();
  */

  for (int j = 0; j < height; ++j)
  {
    for (int i = 0; i < width; ++i)
    {
      uint32_t pixel = drawingWindow.getPixelColour(i, j);

      char r = (char) getSubPixel(pixel, 2);
      char g = (char) getSubPixel(pixel, 1);
      char b = (char) getSubPixel(pixel, 0);

      file.put(r);
      file.put(g);
      file.put(b);
    }

  }

  if (!file.good())
  {
    std::cout << "file evil!!!" << std::endl;
    exit(0);
  }

  file.close();

}