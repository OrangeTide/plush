// Ex2.c: simple Plush example
// Rotates a flat shaded cube
// The cube is now a different colored cube then ex1.c
// ZBuffering has been added, as well as dynamic framebuffer allocation

#include <float.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

// Include the plush header file
#include <plush.h> 

// Include our example graphics interface module
#include "ex.h" 

// Our variables
pl_Light *TheLight;   // Our light
pl_Obj *TheCube;      // Our cube object
pl_Mat *CubeMat;      // The material for the cube
pl_Mat *AllMaterials[2]; // Used for creating palette
pl_Cam *TheCamera; // Our camera
char *TheFrameBuffer; // Our framebuffer to render to
pl_ZBuffer *TheZBuffer;   // Our zbuffer
char ThePalette[768];

int main() { // Main
  int i;
#if defined(DJGPP) || defined(__WATCOMC__)
   // Put the fpu in a low precision, no exception state
  _control87(MCW_EM|PC_24,MCW_EM|MCW_PC); 
#endif
  exSetGraphics(); // Set graphics
 
  TheFrameBuffer = (char *) malloc(320*200); // Alloc framebuffer
  if (!TheFrameBuffer) { 
    exSetText(); 
    printf("Out of memory!\n");
    exit(1);
  }
  // Alloc z-buffer
  TheZBuffer = (pl_ZBuffer *) malloc(320*200*sizeof(pl_ZBuffer));

  CubeMat = plMatCreate();    // Create the material for the cube
  CubeMat->NumGradients = 100; // Have it use 100 colors
  CubeMat->ShadeType = PL_SHADE_FLAT; // Make the cube flat shaded

  CubeMat->Ambient[0] = 32; // Set red ambient component
  CubeMat->Ambient[1] = 0;  // Set green ambient component
  CubeMat->Ambient[2] = 16; // Set blue ambient component

  CubeMat->Diffuse[0] = 200; // Set red diffuse component
  CubeMat->Diffuse[1] = 100; // Set green diffuse component
  CubeMat->Diffuse[2] = 150; // Set blue diffuse component

  plMatInit(CubeMat);          // Initialize the material

  AllMaterials[0] = CubeMat; // Make list of materials
  AllMaterials[1] = 0; // Null terminate list of materials
  plMatMakeOptPal(ThePalette,1,255,AllMaterials); // Create a nice palette

  ThePalette[0] = ThePalette[1] = ThePalette[2] = 0; // Color 0 is black

  plMatMapToPal(CubeMat,ThePalette,0,255); // Map the material to our palette

  // Convert std 8 bit/chan palette to vga's 6 bit/chan palette
  for (i = 0; i < 768; i ++) ThePalette[i] >>= 2;
  exSetPalette(ThePalette); // Set the palette
 
  TheCube = plMakeBox(100.0,100.0,100.0,CubeMat); // Create the cube

  TheCamera = plCamCreate(320, // Screen width
                          200, // Screen height
                          320*3.0/(200*4.0), // Aspect ratio
                          90.0, // Field of view
                          TheFrameBuffer, // Framebuffer
                          TheZBuffer // ZBuffer
                          ); // Create the camera
  TheCamera->Z = -300; // Back the camera up from the origin
  TheCamera->Sort = 0; // We don't need to sort since zbuffering takes care
                       // of it for us!

  TheLight = plLightSet(plLightCreate(), // Create a light to be set up
             PL_LIGHT_VECTOR, // vector light
             0.0,0.0,0.0, // rotation angles
             1.0, // intensity
             1.0); // falloff, not used for vector lights
             
  while (!exGetKey()) { // While the keyboard hasn't been touched
    TheCube->Xa += 1.0; // Rotate by 1 degree on each axis
    TheCube->Ya += 1.0;
    TheCube->Za += 1.0;

                                      // clear zbuffer for next frame
    memset(TheZBuffer,0,320*200*sizeof(pl_ZBuffer));
    memset(TheFrameBuffer,0,320*200); // clear framebuffer for next frame
    plRenderBegin(TheCamera);        // Start rendering with the camera
    plRenderLight(TheLight);         // Render our light
    plRenderObj(TheCube);            // Render our object
    plRenderEnd();                   // Finish rendering
    exWaitVSync();                   // Sync with retrace
    memcpy(exGraphMem,TheFrameBuffer,320*200); // dump to screen
  }
  exSetText(); // Restore text mode
  return 0;          // Quit
}
