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

#include "KeyInput.h"

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
//glm::mat4x4 cameraToWorld = constructCameraSpace(cameraPos, cameraAngle);
glm::mat4x4 cameraToWorld = lookAt({0, 2, 5}, {0, 0, 0});
float theta = 0.0f;


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

void draw()
{
  window.clearPixels();
  clearDepthBuffer();

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
  float vel = 0.02f;
  glm::vec3 translation = {0, 0, 0};
  // Function for performing animation (shifting artifacts or moving the camera)
  updateKeyboard();

  cameraToWorld = rotateAbout({0, 0, 0}, 10, theta);
  theta += 0.1f;

  if(keyDown(SDL_SCANCODE_LEFT)) translation.x -= vel;
  if(keyDown(SDL_SCANCODE_RIGHT)) translation.x += vel;

  if(keyDown(SDL_SCANCODE_UP)) translation.z += vel;
  if(keyDown(SDL_SCANCODE_DOWN)) translation.z -= vel;

  if(keyDown(SDL_SCANCODE_Z)) translation.y -= vel;
  if(keyDown(SDL_SCANCODE_X)) translation.y += vel;
  
  
  // Angle
  if(keyDown(SDL_SCANCODE_W)) rotateX(cameraToWorld, -vel);
  if(keyDown(SDL_SCANCODE_S)) rotateX(cameraToWorld, vel);

  if(keyDown(SDL_SCANCODE_D)) rotateY(cameraToWorld, vel);
  if(keyDown(SDL_SCANCODE_A)) rotateY(cameraToWorld, -vel);

  if(keyDown(SDL_SCANCODE_Q)) rotateZ(cameraToWorld, vel);
  if(keyDown(SDL_SCANCODE_E)) rotateZ(cameraToWorld, -vel);

  if(keyDown(SDL_SCANCODE_U)) focalLength += 10;
  if(keyDown(SDL_SCANCODE_I)) focalLength -= 10;

  translate(cameraToWorld, translation);


}

void handleEvent(SDL_Event event)
{


  if(event.type == SDL_KEYDOWN) {
    // Position
    std::cout << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z <<std::endl;
    std::cout << cameraAngle.x << ", " << cameraAngle.y << ", " << cameraAngle.z << std::endl << std::endl;
  }
  else if(event.type == SDL_MOUSEBUTTONDOWN) cout << "MOUSE CLICKED" << endl;
}
