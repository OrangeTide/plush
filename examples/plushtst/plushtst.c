#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <conio.h>
#include <dos.h>
#include <sys/nearptr.h>

#include <plush.h>

// Vga mode13 funcs
int screenWidth=320, screenHeight=200;
void inline wait_vsync();
void set_palette(char *);
void set_mode13();
void set_mode3();

int clearfb = 1; // Are we clearing the framebuffer

void my_moveout(pl_CameraType *Camera, 
                unsigned long int offs, unsigned long int len) {
  memcpy((void *)
        __djgpp_conventional_base+0xA0000+offs,Camera->frameBuffer+offs,len);
}

void my_movein(pl_CameraType *Camera, 
               unsigned long int offs, unsigned long int len) {
  if (clearfb) {
    memset(Camera->frameBuffer+offs, 0, len);
    if (Camera->zBuffer)  
     memset((Camera->zBuffer + offs),0,(sizeof(pl_ZBufferType)*len));
  }
}

pl_MaterialType mat1, mat2;
void SetUpColors() {
  int x;
  char pal[768];
  mat1.PerspectiveCorrect = 0;
  mat1.NumGradients = 200;
  mat1.Red = 108;
  mat1.Green = 68;
  mat1.Blue = 108;
  mat1.RedSpec = 130;
  mat1.GreenSpec = 130;
  mat1.BlueSpec = 130;
  mat1.Shininess = 10;
  mat1.AmbientLight = 0;
  mat1.Transparent = 0;
  mat1.EnvScaling = 1.0;
  mat1.Priority = 1;
  mat2.NumGradients = 50;
  mat2.Shininess = 2;
  mat2.AmbientLight = 35;
  mat2.Red = 102;
  mat2.Green = 128;
  mat2.Blue = 48;
  mat2.RedSpec = 64;
  mat2.GreenSpec = 64;
  mat2.BlueSpec = 64;
  mat2.Priority = 0;
  plPMBegin(pal,1,255);
  plPMAddMaterial(&mat1);
  plPMAddMaterial(&mat2);
  plPMEnd();
  pal[0] = pal[1] = pal[2] = 0;
  for (x = 0; x < 768; x++) pal[x] >>= 2;
  set_palette(pal);
}

void main(int argc, char **argv) {
  pl_CameraType *Camera;
  pl_ObjectType *myobj1; // Arrowhead like thing
  pl_ObjectType *myobj2; // Sphere
  pl_LightType light1, light2;
  pl_ZBufferType *zbuffer; // Our zbuffer
  pl_TextureType *tex2, *tex1; // Our two textures
  unsigned char *framebuffer; // Our framebuffer
  int done = 0; // ESC hit?
  int frames = 0; // Frames counted
  int fps;   // Fps
  int ticks; // Ticks counted

  plSetLight(&light1,PL_LIGHT_VECTOR,0,0,0,0.77); // Set up lights
  plSetLight(&light2,PL_LIGHT_VECTOR,85,0,0,0.69);

  if ((myobj1 = plReadObject3DS("object1.3ds",&mat1)) == 0) exit(1);
  plScaleObject(myobj1,20.0);
  plTranslateObject(myobj1,-10.0,60.0,0.0);

  if ((myobj2 = plReadObject3DS("object2.3ds",&mat2)) == 0) exit(1);
  plScaleObject(myobj2,20.0);
  plTranslateObject(myobj2,40.0,-40.0,40.0);

  mat1.Texture = tex1 = plReadTexturePCX("texture1.pcx");
  tex2 = plReadTexturePCX("texture2.pcx");

  mat1.ShadeType = PL_SHADE_GOURAUD;
  mat2.ShadeType = PL_SHADE_FLAT;

  printf("%s\n%s\n\n",plVersionString, plCopyrightString);
  printf("Object 1:: Points: %d, Faces: %d\n",
         (int) myobj1->NumVertices,(int) myobj1->NumFaces);
  printf("Object 2:: Points: %d, Faces: %d\n", 
         (int) myobj2->NumVertices,(int) myobj2->NumFaces);
  printf("Keys: \n"
         "  t : toggles texturemapping of arrowhead & sphere (default on)\n"
         "  e : toggles envmapping of arrowhead (default off)\n"
         "  z : toggles zbuffering (default on)\n"
         "  1 : cycles shading mode of arrowhead (default gouraud)\n"
         "  2 : cycles shading mode of sphere (default none)\n"
         "  c : toggles framebuffer erasing (default on)\n"
         "  -,+: zoom in,out\n"
         "  l : toggle translucent sphere (default on)\n"
         "\n");
         
  printf("Hit any key to begin");
  fflush(stdout);
  getch();
  set_mode13();
  framebuffer = malloc(screenWidth*screenHeight);
  if (!framebuffer) {
    set_mode3();
    printf("Error: not enough memory for framebuffer\n"); 
    exit(1);
  }
  zbuffer = (pl_ZBufferType *) 
    malloc(sizeof(pl_ZBufferType)*screenWidth*screenHeight);
  Camera = plNewCamera(screenWidth,screenHeight,
                       (screenWidth*3.0)/(screenHeight*4.0),  
                       40.0,1,framebuffer,zbuffer);
  if (zbuffer) 
    memset(zbuffer,0,(sizeof(pl_ZBufferType)*
           Camera->ScreenWidth*Camera->ScreenHeight));
  memset(framebuffer,0,(Camera->ScreenWidth*Camera->ScreenHeight));
  mat2.Transparent = 1;
  mat2.Environment = NULL;
  mat1.Environment = NULL;
  SetUpColors();
  ticks = *((long *) __djgpp_conventional_base + 0x46C);
  myobj1->Xp = 0;
  myobj1->Yp = 0;
  myobj1->Zp = 1024;
  myobj1->Xa = 0;
  myobj1->Ya = 0;
  myobj1->Za = 0;
  myobj2->Xp = 0;
  myobj2->Yp = 0;
  myobj2->Zp = 1024;
  myobj2->Xa = 0;
  myobj2->Ya = 0;
  myobj2->Za = 0;
  myobj1->BackfaceCull = 1;
  myobj1->BackfaceIllumination = 0;
  myobj2->BackfaceCull = 1;
  myobj2->BackfaceIllumination = 0;
  while (done != 1) {
    frames++;
    myobj1->Xa -= 3.0;
    myobj1->Ya += 1.0;
    myobj1->Za -= 0.5;
    myobj2->Xa+= 1.0;
    myobj2->Ya+= 1.0;
    myobj2->Za+= 1.0;
   
    if (Camera->zBuffer) Camera->Sort = 0;
    else Camera->Sort = 1;
    plRenderBegin(Camera);
    plRenderLight(&light1);
    plRenderLight(&light2);
    plRenderObject(myobj1);
    plRenderObject(myobj2);
    plRenderEnd();
    wait_vsync();
    plWriteBuffer(Camera,my_moveout,my_movein);
    if (kbhit()) switch(getch()) {
      case 27: done = 1; break;
      case 'c': clearfb = !clearfb; 
        if (clearfb) { 
          plInitScanLineBuffers(Camera);
          memset(Camera->frameBuffer,0,
                Camera->ScreenWidth*Camera->ScreenHeight);
          memset((void *)__djgpp_conventional_base+0xA0000,0,
                    Camera->ScreenWidth*Camera->ScreenHeight);
        }
      break;
      case '-': Camera->Fov ++; break;
      case '+': case '=': Camera->Fov--; break;
      case 'l': mat2.Transparent ^= 1; SetUpColors(); break;
      case 'z': if (Camera->zBuffer) Camera->zBuffer = NULL; 
                else { Camera->zBuffer = zbuffer;
                   if (zbuffer) 
                     memset(zbuffer,0,(sizeof(pl_ZBufferType)*
                     Camera->ScreenWidth*Camera->ScreenHeight));
                } 
      break;
      case 'e': if (mat1.Environment) mat1.Environment = NULL; 
                else mat1.Environment = tex2;
                SetUpColors();
      break;
      case 't': if (mat1.Texture) mat1.Texture = NULL; 
                else mat1.Texture = tex1;
                SetUpColors(); 
            break;
      case '1': mat1.ShadeType++; if (mat1.ShadeType > PL_SHADE_GOURAUD) mat1.ShadeType = PL_SHADE_NONE; SetUpColors(); break;
      case '2': mat2.ShadeType++; if (mat2.ShadeType > PL_SHADE_GOURAUD) mat2.ShadeType = PL_SHADE_NONE; SetUpColors(); break;
    }
  }
  ticks -= *((long *) __djgpp_conventional_base + 0x46C);
  ticks = -ticks;
  if (ticks) fps = (182 * frames) / ticks;
  else fps = 0;
  set_mode3();
  printf("There were: %d.%d seconds and %d frames\n",(ticks * 10) / 182, 
                   ((ticks * 100) / 182) % 10, frames);
  printf("%d.%d fps\n", fps/10, fps % 10);
  plFreeObject(myobj1);
  plFreeObject(myobj2);
  free(framebuffer);
  if (zbuffer) free(zbuffer);
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
  __djgpp_nearptr_enable();
  asm("int $0x10"::"a" (0x13):"%eax", "%ebx", "%ecx", "%edx");
}

void set_mode3() {
  __djgpp_nearptr_disable();
  asm("int $0x10"::"a" (3):"%eax", "%ebx", "%ecx", "%edx");
}
