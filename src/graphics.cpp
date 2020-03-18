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
#include "Rasterise.h"
#include "RayTrace.h"
#include "AntiAliasing.h"

#include "KeyInput.h"

using namespace std;
using namespace glm;

#define SCALE 3

#define WIDTH 480
#define HEIGHT 360

#define AA_WIDTH WIDTH * SCALE + 2
#define AA_HEIGHT HEIGHT * SCALE + 2

#define BLACK 0


void draw();
void update();
void handleEvent(SDL_Event event);

DrawingWindow aaWindow;


DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);

float theta = 0.0f;

glm::vec3 cameraPos(0.0f, 2.4f, 3.8f);
glm::vec3 cameraAngle(0.0f, 0.0f, 0.0f);
glm::mat4x4 cameraToWorld = lookAt({0, 2, 5}, {0, 0, 0});


float canvasWidth = AA_WIDTH;
float canvasHeight = AA_HEIGHT;
float imageWidth = AA_WIDTH;
float imageHeight = AA_HEIGHT;

float focalLength = AA_WIDTH / 2;

std::vector<Object> objects = loadOBJ("models/cornell-box.obj", 1.0f);

std::vector<vec4> lights = {
    {-0.159877f, 4.71896f, -2.98309f, 150.0f}
  //, {0.0f, 2.4f, 3.8f, 100.0f}
}; 

int drawMode = 0;
int movementMode = 1;


int main(int argc, char* argv[])
{
  aaWindow.width = AA_WIDTH;
  aaWindow.height = AA_HEIGHT;

  SDL_Event event;
  initDepthBuffer(AA_WIDTH, AA_HEIGHT);

  //cameraToWorld = constructCameraSpace(cameraPos, cameraAngle);
  //rayTraceObjects(objects, lights, cameraToWorld, focalLength, window);

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
  aaWindow.clearPixels();
  clearDepthBuffer();

  switch (drawMode)
  {
    case 0:
      rasteriseObjectsWireframe(objects, cameraToWorld, focalLength, aaWindow);
      break;
    case 1:
      rasteriseObjects(objects, cameraToWorld, focalLength, aaWindow);
      break;
    case 2:
      rayTraceObjects(objects, lights, cameraToWorld, focalLength, aaWindow);
      break;
  }

  antiAlias(aaWindow, window);

}

void cameraControls()
{
  float vel = 0.01f;

  

  // Position
  if(keyDown(SDL_SCANCODE_LEFT))  cameraPos.x += -vel;
  if(keyDown(SDL_SCANCODE_RIGHT)) cameraPos.x +=  vel;

  if(keyDown(SDL_SCANCODE_UP))    cameraPos.z += -vel;
  if(keyDown(SDL_SCANCODE_DOWN))  cameraPos.z +=  vel;

  if(keyDown(SDL_SCANCODE_Z))     cameraPos.y +=  vel;
  if(keyDown(SDL_SCANCODE_X))     cameraPos.y += -vel;
  
  
  // Angle
  if(keyDown(SDL_SCANCODE_W)) cameraAngle.x +=  vel;
  if(keyDown(SDL_SCANCODE_S)) cameraAngle.x += -vel;

  if(keyDown(SDL_SCANCODE_D)) cameraAngle.y += -vel;
  if(keyDown(SDL_SCANCODE_A)) cameraAngle.y +=  vel;

  if(keyDown(SDL_SCANCODE_Q)) cameraAngle.z += -vel;
  if(keyDown(SDL_SCANCODE_E)) cameraAngle.z +=  vel;

  if(keyDown(SDL_SCANCODE_U)) focalLength +=  10;
  if(keyDown(SDL_SCANCODE_I)) focalLength += -10;

  

  cameraToWorld = constructCameraSpace(cameraPos, cameraAngle);

}

void update()
{
  updateKeyboard();

  switch (movementMode)
  {
    case 0: // Orbit Mode
      cameraToWorld = rotateAbout({0, 0, 0}, 3, 10, theta);
      theta += 0.03f;
      break;
    case 1: // Input Mode
      cameraControls();
      break;
  }

  if(keyPressed(SDL_SCANCODE_P)) drawMode = (drawMode + 1) % 3;
  if(keyPressed(SDL_SCANCODE_O)) drawMode = drawMode > 0 ? drawMode - 1 : 0;
  if(keyPressed(SDL_SCANCODE_C)) movementMode = (movementMode + 1) % 2;

}

void handleEvent(SDL_Event event)
{
  if(event.type == SDL_MOUSEBUTTONDOWN) cout << "MOUSE CLICKED" << endl;
}
