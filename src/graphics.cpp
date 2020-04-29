#include <ModelTriangle.h>
#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <glm/glm.hpp>
#include <fstream>
#include <vector>

#include <unistd.h>

#include "Drawing3D.h"
#include "Image.h"
#include "Object.h"
#include "Camera.h"
#include "Rasterise.h"
#include "RayTrace.h"

#include "KeyInput.h"

using namespace std;
using namespace glm;

#define WIDTH 640
#define HEIGHT 480

#define BLACK 0


void draw();
void update();
void handleEvent(SDL_Event event);

DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);




glm::vec3 cameraPos(0.0f, 2.4f, 3.8f);
glm::vec3 cameraAngle(0.0f, 0.0f, 0.0f);
glm::mat4x4 cameraToWorld = lookAt({0, 2, 5}, {0, 0, 0});


float canvasWidth = WIDTH;
float canvasHeight = HEIGHT;
float imageWidth = WIDTH;
float imageHeight = HEIGHT;

float focalLength = WIDTH / 2;

int frameNo = 0;

std::vector<Object> objects = loadOBJ("models/cornell-box.obj", 1.0f, {0.0f, 0.0f, 0.0f});
Object sphere = loadOBJ("models/sphere.obj", 0.04f, {-1.5f, 1.6f, -1.8f})[0];
Object hsLogo = loadOBJ("models/logo.obj", 0.01f, {-3.1f, -0.3f, -5.5f})[0];


std::vector<Light> lights = {
  Light({-0.159877f, 4.71896f, -2.98309f}, 1.0f, 8.0f, 255.0f/255.0f, 244.0f/255.0f, 242.0f/255.0f)
  //, Light({0.0f, 2.4f, 1.8f}, 1.0f, 8.0f, 1.0f, 1.0f, 1.0f)
};

//Must be square for shadow calculations
std::vector<std::vector<glm::vec3>> offsets = {
  {
    {-0.5f, -0.5f, 1.0f},
    {0.0f, -0.5f, 0.0f},
    {0.5f, -0.5f, 1.0f}
  },

  {
    {-0.5f, 0.0f, 0.0f},
    {0.0f, 0.0f, 2.0f},
    {0.5f, 0.0f, 0.0f}
  },

  {
    {-0.5f, 0.5f, 1.0f},
    {0.0f, 0.5f, 0.0f},
    {-0.5f, 0.5f, 1.0f}
  }
};

int offsetsNX = 3;
int offsetsNY = 3;

int drawMode = 0;
int movementMode = 1;

float ballVelocity = 0.0f;
float cameraAngularVel = 0.0037f;
float cameraAngularAcc = -0.000008f;

float theta = 0.0f;
float cameraRadius = 3.0f;
float cameraHeight = 0.5f;
bool passedMidPoint = false;
int numOrbits = 0;
float timeLookingAtBall = 0.0f;

vec3 lastCameraPos;

vec3 orbitCentre(0.0f, 2.75f, 0.0f);


int main(int argc, char* argv[])
{
  SDL_Event event;
  initDepthBuffer(WIDTH, HEIGHT);

  objects.push_back(sphere);
  objects.push_back(hsLogo);

  initBuffers(WIDTH, HEIGHT, offsetsNX, offsetsNY);

  cameraToWorld = constructCameraSpace(cameraPos, cameraAngle);
  //rayTraceObjects(objects, lights, cameraToWorld, focalLength, window, offsets, offsetsNX, offsetsNY);

  bool running = false;
  while(!running)
  {
    // We MUST poll for events - otherwise the window will freeze !
    if(window.pollForInputEvents(&event)) handleEvent(event);
    running = event.type == SDL_MOUSEBUTTONDOWN;
    // Need to render the frame at the end, or nothing actually gets shown on the screen !
    window.renderFrame();
  }

  while(running)
  {
    // We MUST poll for events - otherwise the window will freeze !
    if(window.pollForInputEvents(&event)) handleEvent(event);
    update();
    draw();
    // Need to render the frame at the end, or nothing actually gets shown on the screen !
    window.renderFrame();
  }

  freeBuffers();
}

void draw()
{
  window.clearPixels();
  clearDepthBuffer();

  switch (drawMode)
  {
    case 0:
      rasteriseObjectsWireframe(objects, cameraToWorld, focalLength, window);
      break;
    case 1:
      rasteriseObjects(objects, cameraToWorld, focalLength, window);
      break;
    case 2:
      rayTraceObjects(objects, lights, cameraToWorld, focalLength, window, offsets, offsetsNX, offsetsNY);
      break;
  }
  sleep(0.066666666666f);


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

  // for (Light& light: lights)
  // {
  //   light.r = ((int) (255.0f * light.r + 50.0f) % 255) / 255.0f;
  //   light.g = 1.0f - light.r;
  //   light.b = 1.0f;
  // }

  rotateObjectZ(objects[9], 0.01f,  vec3(-0.1f, 2.7f, 0.0f));

  ballVelocity -= 0.0001f;
  if (objects[8].triangles[0].vertices[0].y <= 1.4f) ballVelocity *= -1.0f;
  if (ballVelocity > 2.4f) ballVelocity = 2.4f;
  translateObject(objects[8],  vec3(0.0f, ballVelocity, 0.0f));
  
  std::cout << getCameraPosition(cameraToWorld) << std::endl;

  switch (movementMode)
  {
    case 0: // Orbit Mode

      
      if (numOrbits < 2)
      {
        theta += cameraAngularVel;
        cameraAngularVel += cameraAngularAcc;

        if (abs(theta) < 0.010f && !passedMidPoint)
        {
          cameraAngularAcc *= -1.0f;
          passedMidPoint = true;
          cameraAngularVel = 0.0037f * -sign(theta);  
        }

        if (abs(cameraAngularVel) < 0.001f && passedMidPoint)
        {
          cameraAngularVel = 0.001f * sign(cameraAngularVel);
          passedMidPoint = false;
          numOrbits ++;
        }
        cameraToWorld = rotateAbout(orbitCentre, cameraHeight, cameraRadius, theta);
      }
      else
      {
        
        if (theta < 1.5f && timeLookingAtBall <= 0.0f)
        {
          theta += 0.001f;
          cameraToWorld = rotateAbout(orbitCentre, cameraHeight, cameraRadius, theta);
        } 
        if (cameraRadius > 1.0f)
        {
          orbitCentre += vec3(0.001f, -0.003f, -0.003f);
          cameraHeight -= 0.001f;
          cameraRadius -= 0.001f;
          //translate(cameraToWorld, vec3(0.0f, -0.002f, -0.003f));
        }
        else
        {
          timeLookingAtBall += 1.0f;
          lastCameraPos = getCameraPosition(cameraToWorld);
        }

        if (timeLookingAtBall > 20.0f && theta > -1.5f)
        {
          exit(0);
        }
        
        
      }
      
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
