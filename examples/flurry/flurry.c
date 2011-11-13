/*
** Flurry2: a Plush demo program. VGA-MODE13 specific version
** Copyright (c) 1997, Justin Frankel
** For use with djgpp v2.x and Allegro
** For more information on Plush, see http://nullsoft.home.ml.org/plush/
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <allegro.h>

#include <plush.h>

BITMAP *DIB;   // Our DIB to draw to

#define NUM_ITERS 4
#define BOX_DIST 2.75
#define BOX_ROTFACTOR 2.6

void makeBoxes(pl_ObjectType *obj, pl_FloatType s,
               pl_MaterialType *m, int i);       // Makes hierarchy of cubes
void rotateBoxes(pl_ObjectType *obj, pl_FloatType r); // Rotates hierarchy

void main(int argc, char **argv) {
  int i, done = 0;

  pl_MaterialType mat[NUM_ITERS]; // Materials
  pl_ObjectType *obj;             // Head object
  pl_LightType *light;
  pl_CameraType *cam;

  int vWidth = 320, vHeight = 200; // For allegro
  pl_FloatType ar;          // Aspect ratio
  char cpal[768];
  PALETTE pal; // Allegro palette

  if (argc > 1) sscanf(argv[1],"%dx%d",&vWidth,&vHeight);

  printf("Flurry v1.0\n"
         "Copyright (c) 1996, Justin Frankel\n");
  printf("Using:\n"
         "  %s\n",plVersionString);

  allegro_init();
  if (set_gfx_mode(GFX_AUTODETECT,vWidth,vHeight,0,0)) {
    allegro_exit();
    printf("Mode not supported\n");
    exit(1);
  }
  DIB = create_bitmap(vWidth,vHeight);
  ar = (vWidth/(pl_FloatType) vHeight) * (3.0/4.0); // Calc aspect ratio

  cam = plNewCamera(vWidth,vHeight,ar,90.0,0,DIB->dat,0);
  cam->Sort = 1;
  cam->Zp = -350;

  // Initialize materials
  memset(&mat,0,sizeof(pl_MaterialType)*NUM_ITERS);
  for (i = 0; i < NUM_ITERS; i ++) {
    mat[i].NumGradients = 200;
    mat[i].Transparent = 2;
    mat[i].AmbientLight = 20;
    mat[i].Shininess = 3;
    mat[i].ShadeType = PL_SHADE_GOURAUD;
    mat[i].Priority = i;
  }
  mat[0].Red = 190; mat[0].Green = 190;  mat[0].Blue = 0;
  mat[0].RedSpec = 240; mat[0].GreenSpec = 240; mat[0].BlueSpec = 0;
  mat[1].Red = 0; mat[1].Green = 0;  mat[1].Blue = 100;
  mat[1].RedSpec = 0; mat[1].GreenSpec = 0; mat[1].BlueSpec = 100;
  mat[2].Red = 0; mat[2].Green = 130;  mat[2].Blue = 0;
  mat[2].RedSpec = 0; mat[2].GreenSpec = 130; mat[2].BlueSpec = 0;
  mat[3].Red = 100; mat[3].Green = 0;  mat[3].Blue = 0;
  mat[3].RedSpec = 100; mat[3].GreenSpec = 0; mat[3].BlueSpec = 0;

  memset(cpal,0,768);
  plPMBegin(cpal,1,255);
  for (i = 0; i < NUM_ITERS; i ++) plPMAddMaterial(mat + i);
  plPMEnd();
  cpal[0] = cpal[1] = cpal[2] = 0;
  for (i = 0; i < 256; i ++) {
    pal[i].r = cpal[i*3] >> 2;
    pal[i].g = cpal[i*3+1] >> 2;
    pal[i].b = cpal[i*3+2] >> 2;
  }
  set_palette(pal); // Set the new palette via allegro

  // Make objects
  obj = plMakeBox(100,100,100,mat);
  makeBoxes(obj,100.0,mat+1,NUM_ITERS-1);

  // Setup light
  light = plNewLight();
  plSetLight(light,PL_LIGHT_VECTOR,0,0,0,1.0);

  while (!done) {
    rotateBoxes(obj,1.0);
    clear(DIB); // clear framebuffer & zbuffer
    plRenderBegin(cam);
    plRenderLight(light);
    plRenderObject(obj);
    plRenderEnd();
    vsync();         // Allegro wait for vsync
    blit(DIB,screen,0,0,0,0,cam->ScreenWidth,cam->ScreenHeight);
    while (kbhit()) switch(getch()) {
      case 27: done = 1; break;
      case '-': cam->Fov += 1.0; if (cam->Fov > 170) cam->Fov = 170; break;
      case '=': cam->Fov -= 1.0; if (cam->Fov < 10) cam->Fov = 10; break;
    }
  }
  destroy_bitmap(DIB);
  plFreeObject(obj);
  plFreeLight(light);
  plFreeCamera(cam);
  set_gfx_mode(GFX_TEXT,0,0,0,0);
  allegro_exit();
  printf("Try \"flurry 640x480\"\n");
}

void makeBoxes(pl_ObjectType *obj, pl_FloatType s, pl_MaterialType *m, int i) {
  int x;
  if (!i) return;
  obj->Children[0] = plMakeBox(s/2,s/2,s/2,m);
  obj->Children[0]->Xp = s*BOX_DIST;
  obj->Children[1] = plMakeBox(s/2,s/2,s/2,m);
  obj->Children[1]->Xp = -s*BOX_DIST;
  obj->Children[2] = plMakeBox(s/2,s/2,s/2,m);
  obj->Children[2]->Yp = s*BOX_DIST;
  obj->Children[3] = plMakeBox(s/2,s/2,s/2,m);
  obj->Children[3]->Yp = -s*BOX_DIST;
  obj->Children[4] = plMakeBox(s/2,s/2,s/2,m);
  obj->Children[4]->Zp = s*BOX_DIST;
  obj->Children[5] = plMakeBox(s/2,s/2,s/2,m);
  obj->Children[5]->Zp = -s*BOX_DIST;
  for (x = 0; x < 6; x ++)
    makeBoxes(obj->Children[x],s/2,m+1,i-1);
}

void rotateBoxes(pl_ObjectType *obj, pl_FloatType r) {
  int i;
  if (!obj) return;
  obj->Ya += r;
  obj->Xa += r;
  for (i = 0 ; i < 6; i ++)
    rotateBoxes(obj->Children[i],r*BOX_ROTFACTOR);
  rotateBoxes(obj->Children[5],r*BOX_ROTFACTOR);
}

