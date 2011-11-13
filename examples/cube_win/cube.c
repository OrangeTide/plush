#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../plush.h"

static pl_MaterialType mat;
static pl_ObjectType *obj;
static pl_LightType *light;
static pl_CameraType *cam;
static pl_TextureType *texture;
static int active;

void cubeRenderFrame(char *fb) {
  obj->Xa += 1.0;
  obj->Ya += 1.0;
  obj->Za += 1.0;
  cam->frameBuffer = fb;
  memset(fb,0,cam->ScreenWidth*cam->ScreenHeight);
  plRenderBegin(cam);
  plRenderLight(light);
  plRenderObject(obj);
  plRenderEnd();
}

void cubeSetupColors(char *pal) {

  memset(&mat,0,sizeof(pl_MaterialType));
  mat.NumGradients = 200;
  mat.Transparent = 0;
  mat.AmbientLight = 0;
  mat.Shininess = 4;
  mat.ShadeType = PL_SHADE_FLAT;
  mat.Priority = 0;
  mat.Red = 10; 
  mat.Green = 10;
  mat.Blue = 10;
  mat.RedSpec = 150;
  mat.GreenSpec = 150;
  mat.BlueSpec = 150;
  mat.Texture = texture;
  mat.PerspectiveCorrect = 32;
  memset(pal,0,768);
  plPMBegin(pal,1,255);
  plPMAddMaterial(&mat);
  plPMEnd();
  pal[0] = pal[1] = pal[2] = 0;
  
}


int cubeInit(int w, int h, float fov) {
  if (!active) {
    obj = plMakeBox(100,100,100,&mat);
    light = plNewLight();
    plSetLight(light,PL_LIGHT_VECTOR,0,0,0,1.0);
    cam = plNewCamera(w,h,1.0,fov,0,0,0);
    cam->Sort = 0;
    cam->Zp = -350;
    texture = plReadTexturePCX("cube.pcx");
    if (!texture) return 1;
	active = 1;
  }
  cam->ScreenWidth = w;
  cam->ClipRight = w;
  cam->ScreenHeight = h;
  cam->ClipBottom = h;
  cam->Fov = fov;
  cam->CenterX = w>>1;
  cam->CenterY = h>>1;
  return 0;
}

void cubeDeInit() {
  if (active) {
    active = 0;
	plFreeObject(obj);
    plFreeLight(light);
    plFreeCamera(cam);
    plFreeTexture(texture);
  }
}
