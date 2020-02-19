#include <ModelTriangle.h>
#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <glm/glm.hpp>
#include <fstream>
#include <vector>

#include "Drawing3D.h"
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

std::vector<float> interpolate(float from, float to, float numValues);

DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);

std::vector<CanvasTriangle> triangles;
std::vector<CanvasTriangle> drawList;

Image image = loadPPM("textures/texture.ppm");

glm::vec3 cameraPos(0.0f, 0.0f, 8.0f);
glm::vec3 cameraAngle(0.0f, 0.0f, 0.0f);
glm::mat4x4 cameraToWorld = constructCameraSpace(cameraPos, cameraAngle);

float canvasWidth = WIDTH;
float canvasHeight = HEIGHT;
float imageWidth = WIDTH;
float imageHeight = HEIGHT;
float focalLength = WIDTH / 2;

std::vector<Object> objects = loadOBJ("models/cornell-box.obj", 1.0f);

int main(int argc, char* argv[])
{
  SDL_Event event;
  initDepthBuffer(WIDTH, HEIGHT);
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
  clearDepthBuffer();
  //glm::mat4x4 cameraToWorld = constructCameraSpace(cameraPos, cameraAngle);


  glm::mat4x4 worldToCamera = glm::inverse(cameraToWorld);

  for (Object obj : objects)
  {
    for (ModelTriangle m : obj.triangles)
    {
      CanvasTriangle t = projectTriangle(m, worldToCamera, focalLength, canvasWidth, canvasHeight, imageWidth, imageHeight);
      fillTriangle(t, window);
    }
  }

  for (CanvasTriangle t : drawList)
  {
    //fillTriangle(t);
    uint32 rgb = packRGB(t.colour.red, t.colour.green, t.colour.blue);
    drawTriangle(t, rgb, window);
  }
}

void update()
{
  // Function for performing animation (shifting artifacts or moving the camera)
}

void handleEvent(SDL_Event event)
{
  float vel = 0.1f;
  glm::vec3 translation = {0, 0, 0};

  if(event.type == SDL_KEYDOWN) {
    // Position
    if(event.key.keysym.sym == SDLK_LEFT) translation.x -= vel;
    if(event.key.keysym.sym == SDLK_RIGHT) translation.x += vel;

    if(event.key.keysym.sym == SDLK_UP) translation.z += vel;
    if(event.key.keysym.sym == SDLK_DOWN) translation.z -= vel;

    if(event.key.keysym.sym == SDLK_LCTRL) translation.y -= vel;
    if(event.key.keysym.sym == SDLK_LSHIFT) translation.y += vel;
    
    translate(cameraToWorld, translation);

    // Angle
    if(event.key.keysym.sym == SDLK_w) rotateX(cameraToWorld, -vel);
    if(event.key.keysym.sym == SDLK_s) rotateX(cameraToWorld, vel);

    if(event.key.keysym.sym == SDLK_d) rotateY(cameraToWorld, vel);
    if(event.key.keysym.sym == SDLK_a) rotateY(cameraToWorld, -vel);

    if(event.key.keysym.sym == SDLK_q) rotateZ(cameraToWorld, vel);
    if(event.key.keysym.sym == SDLK_e) rotateZ(cameraToWorld, -vel);

    if(event.key.keysym.sym == SDLK_u) focalLength += 10;
    if(event.key.keysym.sym == SDLK_i) focalLength -= 10;

    std::cout << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z <<std::endl;
    std::cout << cameraAngle.x << ", " << cameraAngle.y << ", " << cameraAngle.z << std::endl << std::endl;
  }
  else if(event.type == SDL_MOUSEBUTTONDOWN) cout << "MOUSE CLICKED" << endl;
}
