#include <ModelTriangle.h>
#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <glm/glm.hpp>
#include <fstream>
#include <vector>

using namespace std;
using namespace glm;

#define WIDTH 500
#define HEIGHT 500

#define BLACK 0

struct Image
{
  int width, height;
  uint32_t* payload;

  Image() {}

  Image(int width, int height, uint32_t* payload)
  : width(width)
  , height(height)
  , payload(payload)
  {}

  uint32_t GetPixel(int x, int y)
  {
    int i = y * width + x;
    return payload[i];
  }
};


void draw();
void update();
void handleEvent(SDL_Event event);

Image loadPPM(const char* filename);

inline uint32_t packRGB(int r, int g, int b);

void drawLine(const CanvasPoint& from, const CanvasPoint& to, uint32_t colour);
void drawTriangle(CanvasTriangle& triangle, uint32_t colour);
void fillTriangle(CanvasTriangle& triangle);


std::vector<float> interpolate(float from, float to, float numValues);

DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);

std::vector<CanvasTriangle> triangles;

Image image = loadPPM("texture.ppm");

int main(int argc, char* argv[])
{
  SDL_Event event;

  loadPPM("texture.ppm");


  while(true)
  {
    // We MUST poll for events - otherwise the window will freeze !
    if(window.pollForInputEvents(&event)) handleEvent(event);
    update();
    draw();
    // Need to render the frame at the end, or nothing actually gets shown on the screen !
    window.renderFrame();
  }
}

int readNumber(FILE *fptr)
{
  char buffer[8];
  fscanf(fptr, "%s\n", buffer);
  return atoi(buffer);
}

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
  image.width = readNumber(fptr);

  // Whitespace
  retval = fscanf(fptr, " ");

  // Read width
  image.height = readNumber(fptr);

  // Whitespace
  retval = fscanf(fptr, " ");

  // Read width
  int maxval = readNumber(fptr);

  if (retval) exit(EXIT_FAILURE);
  if (!maxval) exit(EXIT_FAILURE);

  // Copy payload
  imageSize = image.width * image.height;
  image.payload = (uint32_t*) malloc(imageSize * sizeof(uint32_t));

  for (int i = 0; i < imageSize; ++i)
  {
    int r = (int) getc(fptr);
    int g = (int) getc(fptr);
    int b = (int) getc(fptr);

    uint32_t colour = packRGB(r,g,b); 
    image.payload[i] = colour;
  }

  fclose(fptr);

  return image;
}

void drawImage(Image& img)
{
  for (int y = 0; y < img.height; ++y)
  {
    for (int x = 0; x < img.width; ++x)
    {
      uint32_t colour = img.GetPixel(x, y);
      window.setPixelColour(x, y, colour);
    }
  }
}


void drawLine(const CanvasPoint& from, const CanvasPoint& to, uint32_t colour)
{
  float xDiff = to.x - from.x;
  float yDiff = to.y - from.y;
  float numberOfSteps = std::max(abs(xDiff), std::abs(yDiff));
  float xStepSize = xDiff/numberOfSteps;
  float yStepSize = yDiff/numberOfSteps;
  for (float i=0.0; i<numberOfSteps; i++) {
    float x = from.x + (xStepSize*i);
    float y = from.y + (yStepSize*i);
    window.setPixelColour(round(x), round(y), colour);
  }
}

void drawTriangle(CanvasTriangle& triangle, uint32_t colour)
{
  int j = 2;
  for (int i = 0; i < 3; ++i)
  {
    drawLine(triangle.vertices[i], triangle.vertices[j], colour);
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

void fillTriangle(CanvasTriangle& triangle)
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
    drawLine({x1, y}, {x2, y}, colour);
  }

  //Fill bottom triangle.
  for (float y = yMid; y < yEnd; ++y)
  {
    float x1 = xPointsLHS2[(int) (y - yMid)];
    float x2 = xPointsRHS[(int) (y - yStart)];
    drawLine({x1, y}, {x2, y}, colour);
  }
}

void fillTriangleTexture(CanvasTriangle& triangle, Image& image)
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

inline uint32_t packRGB(int r, int g, int b)
{
  return (255<<24) + (r<<16) + (g<<8) + b;
}

inline uint32_t packRGB(vec3 rgb)
{
  return (255<<24) + (int(rgb.x)<<16) + (int(rgb.y)<<8) + int(rgb.z);
}

inline uint32_t packGreyscale(int n)
{
  return (255<<24) + (int(n)<<16) + (int(n)<<8) + int(n);
}

void drawRandomTriangle()
{
  int r = rand() % 255;
  int g = rand() % 255;
  int b = rand() % 255;

  CanvasPoint p1(rand() % window.width, rand() % window.height);
  CanvasPoint p2(rand() % window.width, rand() % window.height);
  CanvasPoint p3(rand() % window.width, rand() % window.height);

  CanvasTriangle t(p1,p2,p3, Colour(r,g,b));

  triangles.push_back(t);
}

void colourInterp()
{
  vec3 TL( 255, 0, 0 );
  vec3 TR( 0, 0, 255 );

  vec3 BL( 255, 255, 0 );
  vec3 BR( 0, 255, 0 );

  std::vector<vec3> LHS = interpolate(TL, BL, window.height);
  std::vector<vec3> RHS = interpolate(TR, BR, window.height);

  for(int y=0; y<window.height ;y++) {
    vec3 from = LHS[y];
    vec3 to = RHS[y];
    std::vector<vec3> values = interpolate(from, to, window.width);

    for(int x=0; x<window.width ;x++) {
      vec3 value = values[x];
      uint32_t colour = packRGB(value);
      window.setPixelColour(x, y, colour);
    }
  }
}

void greyScale()
{
  std::vector<float> values = interpolate(255, 0, window.width);

  for(int y=0; y<window.height ;y++) {
    for(int x=0; x<window.width ;x++) {
      int value = (int) values[x];
      uint32_t colour = packGreyscale(value);
      window.setPixelColour(x, y, colour);
    }
  }
}

void redNoise()
{
  for(int y=0; y<window.height ;y++) {
    for(int x=0; x<window.width ;x++) {
      float red = rand() % 255;
      float green = 0.0;
      float blue = 0.0;
      uint32_t colour = (255<<24) + (int(red)<<16) + (int(green)<<8) + int(blue);
      window.setPixelColour(x, y, colour);
    }
  }
}

void draw()
{
  window.clearPixels();
  colourInterp();

  CanvasPoint p1(160, 10);
  CanvasPoint p2(300, 230);
  CanvasPoint p3(10, 150);

  p1.texturePoint = {195, 5};
  p2.texturePoint = {395, 380};
  p3.texturePoint = {65, 330};

  CanvasTriangle triangle(p1,p2,p3);

  
  fillTriangleTexture(triangle, image);
  drawTriangle(triangle, 0);
  
  
  for (CanvasTriangle t : triangles)
  {
    fillTriangle(t);
    drawTriangle(t, 0);
  }
}

void update()
{
  // Function for performing animation (shifting artifacts or moving the camera)
}

void handleEvent(SDL_Event event)
{
  if(event.type == SDL_KEYDOWN) {
    if(event.key.keysym.sym == SDLK_LEFT) cout << "LEFT" << endl;
    else if(event.key.keysym.sym == SDLK_RIGHT) cout << "RIGHT" << endl;
    else if(event.key.keysym.sym == SDLK_UP) cout << "UP" << endl;
    else if(event.key.keysym.sym == SDLK_DOWN) cout << "DOWN" << endl;
    else if(event.key.keysym.sym == SDLK_u) drawRandomTriangle();
    else if(event.key.keysym.sym == SDLK_i) triangles.pop_back();
  }
  else if(event.type == SDL_MOUSEBUTTONDOWN) cout << "MOUSE CLICKED" << endl;
}
