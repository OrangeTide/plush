/*
** Flurry: a Plush demo program.
** Copyright (c) 1997, Justin Frankel
** For use with djgpp v2.x
** For more information on Plush, see http://nullsoft.home.ml.org/plush/
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <plush.h>

char *framebuffer;
void inline wait_vsync();
void set_palette(char *);
void set_mode13();
void set_mode3();

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

  int vWidth = 320, vHeight = 200;
  pl_FloatType ar;
  char cpal[768];

  if (argc > 1) sscanf(argv[1],"%dx%d",&vWidth,&vHeight);

  printf("Flurry v1.0 -- Mode13 specific version\n"
         "Copyright (c) 1996, Justin Frankel\n");
  printf("Using:\n"
         "  %s\n",plVersionString);

  set_mode13();
  framebuffer = (char *) malloc(vWidth*vHeight);
  ar = (vWidth/(pl_FloatType) vHeight) * (3.0/4.0); // Calc aspect ratio

  cam = plNewCamera(vWidth,vHeight,ar,90.0,0,framebuffer,0);
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
  for (i = 0; i < 768; i ++)
    cpal[i] >>= 2;
  set_palette(cpal); // Set the new palette via mode 13 function

  // Make objects
  obj = plMakeBox(100,100,100,mat);
  makeBoxes(obj,100.0,mat+1,NUM_ITERS-1);

  // Setup light
  light = plNewLight();
  plSetLight(light,PL_LIGHT_VECTOR,0,0,0,1.0);

  while (!done) {
    rotateBoxes(obj,1.0);
    memset(framebuffer,0,vWidth*vHeight);
    plRenderBegin(cam);
    plRenderLight(light);
    plRenderObject(obj);
    plRenderEnd();
    wait_vsync();
    dosmemput(framebuffer,vWidth*vHeight,0xA0000);
    while (kbhit()) switch(getch()) {
      case 27: done = 1; break;
      case '-': cam->Fov += 1.0; if (cam->Fov > 170) cam->Fov = 170; break;
      case '=': cam->Fov -= 1.0; if (cam->Fov < 10) cam->Fov = 10; break;
    }
  }
  free(framebuffer);
  plFreeObject(obj);
  plFreeLight(light);
  plFreeCamera(cam);
  set_mode3();
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

// VGA mode 13 functions

void inline wait_vsync() {
  __asm__ __volatile__ ("movw $0x3DA, %%dx
    0: inb %%dx, %%al ; andb $8, %%al ; jnz 0b 
    0: inb %%dx, %%al ; andb $8, %%al ; jz 0b"
    :::"%edx", "%eax");
}

void set_palette(char *pal) {
  asm("
    movl $768, %%ecx
    movb $0, %%al
    movw $0x3C8, %%dx
    outb %%al, %%dx
    incw %%dx
    rep ; outsb
  "::"S" (pal):"%esi", "%eax", "%edx", "%ecx");
}

void set_mode13() {
  asm("int $0x10"::"a" (0x13):"%eax", "%ebx", "%ecx", "%edx");
}

void set_mode3() {
  asm("int $0x10"::"a" (3):"%eax", "%ebx", "%ecx", "%edx");
}
