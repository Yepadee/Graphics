#pragma once

#include <fstream>
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
  FILE *fptr;
  int retval;

  Image image;
  int imageSize;

  if ((fptr = fopen(fileName,"r")) == NULL){
      printf("Error! opening file");
      exit(1);
  }

  // Read magic no.
  retval = fscanf(fptr, "%*[^\n]\n");

  //printf("str: %s\n", buf);

  // Read Space
  retval = fscanf(fptr, " ");

  // Read comment
  char c = (char) getc(fptr);
  if (c == '#') retval = fscanf(fptr, "%*[^\n]\n");

  // Read width
  int width = readNumber(fptr);

  // Whitespace
  retval = fscanf(fptr, " ");

  // Read width
  int height = readNumber(fptr);

  // Whitespace
  retval = fscanf(fptr, " ");

  // Read width
  int maxval = readNumber(fptr);

  if (retval) exit(EXIT_FAILURE);
  if (!maxval) exit(EXIT_FAILURE);

  // Copy payload
  imageSize = width * height;
  uint32_t* payload = (uint32_t*) malloc(imageSize * sizeof(uint32_t));

  for (int i = 0; i < imageSize; ++i)
  {
    int r = (int) getc(fptr);
    int g = (int) getc(fptr);
    int b = (int) getc(fptr);

    uint32_t colour = packRGB(r,g,b); 
    payload[i] = colour;
  }

  fclose(fptr);

  return Image(width, height, payload);
}

