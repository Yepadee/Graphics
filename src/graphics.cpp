#include <ModelTriangle.h>
#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <glm/glm.hpp>
#include <fstream>
#include <vector>

#include "Image.h"
#include "Object.h"
#include "Camera.h"

using namespace std;
using namespace glm;

#define WIDTH 720
#define HEIGHT 720

#define BLACK 0


void draw();
void update();
void handleEvent(SDL_Event event);

void drawLine(const CanvasPoint& from, const CanvasPoint& to, uint32_t colour);
void drawTriangle(CanvasTriangle& triangle, uint32_t colour);
void fillTriangle(CanvasTriangle& triangle);

std::vector<float> interpolate(float from, float to, float numValues);

DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);

std::vector<CanvasTriangle> triangles;
std::vector<CanvasTriangle> drawList;

Image image = loadPPM("textures/texture.ppm");

glm::vec3 cameraPos(0.2f, -2.5f, -2.8f);
glm::vec3 cameraAngle(0.0f, 0.0f, 0.0f);

float canvasWidth = WIDTH;
float canvasHeight = HEIGHT;
float imageWidth = WIDTH;
float imageHeight = HEIGHT;
float focalLength = WIDTH / 2;

std::vector<Object> objects = loadOBJ("models/cornell-box.obj", 1.0f);

int main(int argc, char* argv[])
{
  SDL_Event event;

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
    if (x > 0 && x < WIDTH - 1 && y < HEIGHT - 1 && y > 0)
    {
      window.setPixelColour(round(x), round(y), colour);
    }
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

void draw()
{
  window.clearPixels();

  glm::mat4x4 cameraToWorld = constructCameraSpace(cameraPos, cameraAngle);

  for (Object obj : objects)
  {
    for (ModelTriangle m : obj.triangles)
    {
      //drawList.push_back(projectTriangle(m, cameraToWorld, focalLength, canvasWidth, canvasHeight, imageWidth, imageHeight));
      CanvasTriangle t = projectTriangle(m, cameraToWorld, focalLength, canvasWidth, canvasHeight, imageWidth, imageHeight);
      //uint32_t rgb = packRGB(t.colour.red, t.colour.green, t.colour.blue);
      fillTriangle(t);
    }
  }

  for (CanvasTriangle t : drawList)
  {
    //fillTriangle(t);
    uint32 rgb = packRGB(t.colour.red, t.colour.green, t.colour.blue);
    drawTriangle(t, rgb);
  }
}

void update()
{
  // Function for performing animation (shifting artifacts or moving the camera)
}

void handleEvent(SDL_Event event)
{
  float vel = 0.1f;

  if(event.type == SDL_KEYDOWN) {
    // Position
    if(event.key.keysym.sym == SDLK_LEFT) cameraPos.x += vel;
    else if(event.key.keysym.sym == SDLK_RIGHT) cameraPos.x -= vel;

    else if(event.key.keysym.sym == SDLK_UP) cameraPos.z += vel;
    else if(event.key.keysym.sym == SDLK_DOWN) cameraPos.z -= vel;

    else if(event.key.keysym.sym == SDLK_LCTRL) cameraPos.y += vel;
    else if(event.key.keysym.sym == SDLK_LSHIFT) cameraPos.y -= vel;
    

    // Angle
    else if(event.key.keysym.sym == SDLK_w) cameraAngle.x += vel;
    else if(event.key.keysym.sym == SDLK_s) cameraAngle.x -= vel;

    else if(event.key.keysym.sym == SDLK_d) cameraAngle.y += vel;
    else if(event.key.keysym.sym == SDLK_a) cameraAngle.y -= vel;

    else if(event.key.keysym.sym == SDLK_q) cameraAngle.z += vel;
    else if(event.key.keysym.sym == SDLK_e) cameraAngle.z -= vel;

    else if(event.key.keysym.sym == SDLK_u) focalLength += 10;
    else if(event.key.keysym.sym == SDLK_i) focalLength -= 10;

    std::cout << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z <<std::endl;
    std::cout << cameraAngle.x << ", " << cameraAngle.y << ", " << cameraAngle.z << std::endl << std::endl;
  }
  else if(event.type == SDL_MOUSEBUTTONDOWN) cout << "MOUSE CLICKED" << endl;
}
