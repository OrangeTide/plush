/*
** DUCKDEMO: a Plush demo program.
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

pl_MaterialType Material1, Material2;   // Materials for the duck
pl_CameraType *Camera;                // Camera
pl_ObjectType *Object;              // The duck
pl_LightType *Light;              // Light source

void SetUpColors();

void main(int argc, char **argv) {
  pl_ZBufferType *zbuffer;             // Our zbuffer
  int vWidth = 320, vHeight = 200;     // Default video size for allegro

  printf("%s\n%s\n\n",plVersionString,plCopyrightString); // I like to do this

  Object = plReadObject3DS("duckdemo.3ds",&Material1);
  if (!Object) {
    perror("Can't load duckdemo.3ds");
    exit(1);
  }
     // First child is an eye, child of child is eye
  plSetObjectMaterial(Object->Children[0],&Material2,0); 
  plSetObjectMaterial(Object->Children[0]->Children[0],&Material2,0);
     // Child of eye is other eye, make it child of duck
  Object->Children[1] = Object->Children[0]->Children[0];
  Object->Children[0]->Children[0] = 0;

  plScaleObject(Object,0.1); // Scale object down... 

  Object->BackfaceCull = 0;    // We want to be able to see through the duck
  Object->BackfaceIllumination = 1;

  Light = plNewLight();        // Create a lightsource
  plSetLight(Light,PL_LIGHT_VECTOR,0,0,0,1.0); // Vector light, 1.0 intensity

  allegro_init();
  if (argc > 1) sscanf(argv[1],"%dx%d",&vWidth,&vHeight);
  if (set_gfx_mode(GFX_AUTODETECT,vWidth,vHeight,0,0)) {
    allegro_exit(); 
    printf("Mode not supported\n");
    exit(1);
  }
  DIB = create_bitmap(vWidth,vHeight);
  if ((argc > 1 && !stricmp(argv[1],"-nozb")) || 
      (argc > 2 && !stricmp(argv[2],"-nozb"))) zbuffer = 0;
  else 
    zbuffer = (pl_ZBufferType *) malloc(sizeof(pl_ZBufferType)*vWidth*vHeight);

  Camera = plNewCamera(vWidth,vHeight, // Create camera
                       vWidth*3.0/(vHeight*4.0), // Aspect ratio (usually 1.0)
                       80.0, 0, DIB->dat, zbuffer);
  Camera->Zp = -500;   // move the camera back a bit
  if (zbuffer) Camera->Sort = 0; // Sorting not necessary w/ zbuffer
  else Camera->Sort = 1;

  SetUpColors(); // Init palette

  while (!kbhit()) {
    Object->Xa += 2.0;  // Rotate object
    Object->Ya += 2.0;
    Object->Za += 2.0;

    clear(DIB); // clear framebuffer & zbuffer
    if (Camera->zBuffer) memset(Camera->zBuffer,0,sizeof(pl_ZBufferType)*
                                Camera->ScreenWidth*Camera->ScreenHeight);

    plRenderBegin(Camera); // Render to camera
    plRenderLight(Light);  // Render light
    plRenderObject(Object); // Render duck
    plRenderEnd(); // Finish rendering
    vsync();         // Allegro wait for vsync
    blit(DIB,screen,0,0,0,0,Camera->ScreenWidth,Camera->ScreenHeight);
      // Blit framebuffer
  }
  plFreeObject(Object); // Free duck
  plFreeLight(Light);   // Free light
  plFreeCamera(Camera); // Free camera

  free(zbuffer);
  destroy_bitmap(DIB);

  set_gfx_mode(GFX_TEXT,0,0,0,0);
  allegro_exit();
  printf("Try \"duckdemo 640x480\" or \"duckdemo 320x200 -nozb\" etc\n");
}

void SetUpColors() {
  int x;
  PALETTE pal;  // Allegro palette
  char cpal[768]; // Our rgb triplet palette
  memset(cpal,0,768);

  Material1.Priority = 0;  // setup material 1
  Material1.NumGradients = 200;
  Material1.Red = 203;
  Material1.Green = 212;
  Material1.Blue = 0;
  Material1.RedSpec = 128;
  Material1.GreenSpec = 56;
  Material1.BlueSpec = 0;
  Material1.Shininess = 15;
  Material1.AmbientLight = 0;
  Material1.Transparent = 0;
  Material1.Environment = NULL;
  Material1.Texture = NULL; // Could do plReadTexturePCX() here for texture...
  Material1.ShadeType = PL_SHADE_GOURAUD;

  Material2.Priority = 1;  // setup material 2
  Material2.NumGradients = 100;
  Material2.Red = 0;
  Material2.Green = 0;
  Material2.Blue = 0;
  Material2.RedSpec = 160;
  Material2.GreenSpec = 130;
  Material2.BlueSpec = 0;
  Material2.Shininess = 5;
  Material2.AmbientLight = 0;
  Material2.Transparent = 0;
  Material2.Environment = NULL;
  Material2.Texture = NULL;
  Material2.ShadeType = PL_SHADE_GOURAUD;

  plPMBegin(cpal,1,254);    // Generate the palette
  plPMAddMaterial(&Material1);
  plPMAddMaterial(&Material2);
  plPMEnd();

  for (x = 0; x < 256; x++) { // Convert std palette to allegro palette
    pal[x].r = cpal[x*3+0] >> 2;
    pal[x].g = cpal[x*3+1] >> 2;
    pal[x].b = cpal[x*3+2] >> 2;
  }
  set_palette(pal); // Set the palette
}
