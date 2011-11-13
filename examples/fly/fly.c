#include <float.h>
#include <time.h>
#include <stdio.h>
#include <math.h>
#include <conio.h>
#include <sys/nearptr.h>

#include <plush.h>

/* Physical size of land */
#define LAND_SIZE 65000
/* Number of divisions of the land. Higher number == more polygons */
#define LAND_DIV 32

/* These three make the executable small for DJGPP v2 programs */
void   __crt0_load_environment_file(char *_app_name) { }
char **__crt0_glob_function(char *_arg) { return 0; }
void __crt0_setup_arguments();

                  /* Mouse interface functions */
 // initializes mouse
void mouse_init();
 //  updates the variables below
void mouse_get();
 // deinitializes the mouse
void mouse_deinit();
 // These are the amount the mouse has moved since the last frame
signed short int mouse_x, mouse_y;
 // mouse_b = button status, mouse_avail is whether or not mouse is
 // available, mouse_buttons is number of buttons
int mouse_b, mouse_avail, mouse_buttons;
 // mouse sensitivity
float mouse_sens = 2048.0/32768.0;

                    /* Vga interface functions */
 // Waits for retrace
void inline vsync();
 // Sets a 256 color palette. Automagically converts to vga 6 bits/chan
void set_palette(char *);
 // Sets mode13 320x200x256
void set_mode13();
 // Sets text mode
void set_mode3();
 // Ultrafast memcpy designed for copying to video memory.
 // len16 = number of 16 byte blocks to copy
static void inline fpucopy(void *o, void *i, int len16);

                    /* Misc functions */
 // Sets up the materials
void setup_materials(pl_Mat **mat, unsigned char *pal);
 // Sets up the landscape and skies
pl_Obj *setup_landscape(pl_Mat *m, pl_Mat *sm, pl_Mat *sm2);

                       /* Main!!! */
void main() {
  char lastmessage[80] = "Fly 3.0"; // last message used for status line
  int draw_sky = 1;                 // do we draw the sky?
  int wait_vsync = 0;               // do we wait for vsync?
  int frames, t;                    // for framerate counter
  int i;

  pl_uChar *framebuffer;            // our doublebuffer
  pl_Mat *mat[3+1];                 // our materials, we have 1 extra for null
                                    // termination for plMatMakeOptPal2()
  pl_Cam *cam;                      // our camera
  pl_Obj *land;                     // the land object
  pl_Obj *sky, *sky2;               // the two skies
  int done = 0;

  char pal[768];                    // our palette

  srand(0);                         // initialize rng

  _control87(MCW_EM|PC_24,MCW_EM|MCW_PC);
     // Set the FPU in low precision, no exception mode (REQUIRED)

  printf("Plush 3D Fly v3.0.\n"
         "  %s\n"
         "  %s\n",plVersionString,plCopyrightString);
     // print out startup info

  mouse_init(); // initialize mouse

  printf("\n\nControls:\n"
         "  Mouse: rotate\n"
         "  Mouse buttons: left=move forward, right=move forward fast\n"
         "  s: toggle sky (default on)\n"
         "  -,+: adjust fov (default 90)\n"
         "  [,]: adjust mouse sensitivity\n"
         "  v: toggle vsync (default off)\n\n");

  printf("\nHit any key to begin...");
  getch();
  while (kbhit()) getch(); // make sure keyboard buffer is empty

  set_mode13();  // intialize graphics
  framebuffer = (pl_uChar *) malloc(320*200); // allocate framebuffer
      // create camera
  cam = plCamCreate(320, // width
                    200, // height
                    320.0/200.0*3.0/4.0, // aspect ratio
                    90.0, // fov
                    framebuffer, // framebuffer (our doublebuffer)
                    0);  // zbuffer (not used)
  cam->Y = 800; // move the camera up from the ground

  setup_materials(mat,pal); // intialize materials and palette

  land = setup_landscape(mat[0],mat[1],mat[2]); // create landscape
  sky = land->Children[0]; // unhierarchicalize the sky from the land
  land->Children[0] = 0;
  sky2 = land->Children[1];
  land->Children[1] = 0;

  frames = 0;     // set up for framerate counter
  t = uclock();
  while (!done) { // main loop
    // save time when the frame began, to be used later.
    float prevtime = uclock() / (float) UCLOCKS_PER_SEC;
    frames++; // update for framerate counter

    memset(framebuffer,1,64000); // clear our doublebuffer

    // lots of rendering special casing
    if (draw_sky) { // if we're drawing the sky
      if (cam->Y > 2000) { // if above the sky, only render the skies, with
                           // no far clip plane
        cam->ClipBack = 0.0;
        plRenderBegin(cam);
        plRenderObj(sky);
        plRenderObj(sky2);
      } else {           // otherwise, render the sky (but not the second sky),
                         // and the land, with a far clip plane
        cam->ClipBack = 10000.0;
        plRenderBegin(cam);
        plRenderObj(sky);
        plRenderObj(land);
      }
    } else { // not drawing sky, just render the land
      cam->ClipBack = 10000.0;
      plRenderBegin(cam);
      plRenderObj(land);
    }
    plRenderEnd(); // finish rendering

    // display framerate counter
    plTextPrintf(cam,cam->ClipLeft+5,cam->ClipTop,0.0,156,"%.2f fps",
        (frames/ (float) (uclock() - t)) * (float) UCLOCKS_PER_SEC);
    // display last message
    plTextPrintf(cam,cam->ClipLeft+5,cam->ClipBottom-16,0.0,156,lastmessage);


    if (wait_vsync) vsync(); // wait for vsync
      /* blit to screen. This is pretty darn fast on ip5's but on a 486 you
         would probably be faster doing a plain memcpy(), i.e:
         memcpy((void *) __djgpp_conventional_base+0xA0000,framebuffer,64000);
      */
    fpucopy((void *) __djgpp_conventional_base + 0xA0000,framebuffer,64000/16);

    // We calculate the amount of time in thousanths of seconds this frame took
    prevtime = ((uclock() / (float) UCLOCKS_PER_SEC) - prevtime)*1000.0;
    mouse_get(); // update the mouse
    if (mouse_b & 2) { // if right button hit, we go forward quickly
      cam->X -=
        prevtime*4*sin(cam->Pan*PL_PI/180.0)*cos(cam->Pitch*PL_PI/180.0);
      cam->Z +=
        prevtime*4*cos(cam->Pan*PL_PI/180.0)*cos(cam->Pitch*PL_PI/180.0);
      cam->Y +=
        prevtime*4*sin(cam->Pitch*PL_PI/180.0);
    } else if (mouse_b & 1) { // if left button hit, we go forward slowly
      cam->X -=
        prevtime*2*sin(cam->Pan*PL_PI/180.0)*cos(cam->Pitch*PL_PI/180.0);
      cam->Z +=
        prevtime*2*cos(cam->Pan*PL_PI/180.0)*cos(cam->Pitch*PL_PI/180.0);
      cam->Y +=
        prevtime*2*sin(cam->Pitch*PL_PI/180.0);
    }
    cam->Pitch += (mouse_y*mouse_sens); // update pitch and pan of ship
    cam->Pan -= (mouse_x*mouse_sens);

    if (cam->X > LAND_SIZE/2) cam->X = LAND_SIZE/2; // make sure we don't go
    if (cam->X < -LAND_SIZE/2) cam->X = -LAND_SIZE/2; // too far away
    if (cam->Z > LAND_SIZE/2) cam->Z = LAND_SIZE/2;
    if (cam->Z < -LAND_SIZE/2) cam->Z = -LAND_SIZE/2;
    if (cam->Y < 0) cam->Y = 0;
    if (cam->Y > 8999) cam->Y = 8999;

    while (kbhit()) switch(getch()) { // handle keystrokes
      case 27: done++; break;    // ESC == quit
        // + is for zooming in.
      case '=': case '+': cam->Fov -= 1.0; if (cam->Fov < 1.0) cam->Fov = 1.0;
        sprintf(lastmessage,"FOV: %2.f",cam->Fov);
      break;
        // - is for zooming out
      case '-': cam->Fov += 1.0; if (cam->Fov > 179.0) cam->Fov = 179.0;
        sprintf(lastmessage,"FOV: %2.f",cam->Fov);
      break;
        // [ decreases mouse sensitivity
      case '[': mouse_sens /= 1.1;
        sprintf(lastmessage,"MouseSens: %.3f",mouse_sens);
      break;
        // ] increases mouse sensitivity
      case ']': mouse_sens *= 1.1;
        sprintf(lastmessage,"MouseSens: %.3f",mouse_sens);
      break;
        // v toggles vsync
      case 'v': wait_vsync ^= 1;
        sprintf(lastmessage,"VSync %s",wait_vsync ? "on" : "off");
      break;
        // s toggles sky
      case 's': draw_sky ^= 1;
        sprintf(lastmessage,"Sky %s",draw_sky ? "on" : "off");
      break;
    }
  }
  // set text mode
  set_mode3();
  // clean up
  free(framebuffer);
  plObjDelete(land);
  plObjDelete(sky);
  plObjDelete(sky2);
  plMatDelete(mat[0]);
  plMatDelete(mat[1]);
  plMatDelete(mat[2]);
  plCamDelete(cam);

  printf("This has been a Plush demo app.\n"
         "Visit the Plush 3D homepage at: \n"
         "  http://nullsoft.home.ml.org/plush/\n\n");
}

void setup_materials(pl_Mat **mat, unsigned char *pal) {
  int i;
  // create our 3 materials, make the fourth null so that plMatMakeOptPal2()
  // knows where to stop
  mat[0] = plMatCreate();
  mat[1] = plMatCreate();
  mat[2] = plMatCreate();
  mat[3] = 0;

  pal[0] = pal[1] = pal[2] = 0; // make color 0 black.

  // set up material 0 (the ground)
  mat[0]->ShadeType = PL_SHADE_GOURAUD_DISTANCE;
  mat[0]->Shininess = 1;
  mat[0]->NumGradients = 2500;
  mat[0]->Ambient[0] = pal[0]*2 - 255; // these calculations are to get the
  mat[0]->Ambient[1] = pal[1]*2 - 255; // distance shading to work right
  mat[0]->Ambient[2] = pal[2]*2 - 255;
  mat[0]->Diffuse[0] = 127-pal[0];
  mat[0]->Diffuse[1] = 127-pal[1];
  mat[0]->Diffuse[2] = 127-pal[2];
  mat[0]->Specular[0] = 127-pal[0];
  mat[0]->Specular[1] = 127-pal[1];
  mat[0]->Specular[2] = 127-pal[2];
  mat[0]->FadeDist = 10000.0;
  mat[0]->Texture = plReadPCXTex("ground.pcx",1,1);
  mat[0]->TexScaling = 40.0*LAND_SIZE/50000;
  mat[0]->PerspectiveCorrect = 16;

  // set up material 1 (the sky)
  mat[1]->ShadeType = PL_SHADE_GOURAUD_DISTANCE;
  mat[1]->Shininess = 1;
  mat[1]->NumGradients = 1500;
  mat[1]->Ambient[0] = pal[0]*2 - 255;
  mat[1]->Ambient[1] = pal[1]*2 - 255;
  mat[1]->Ambient[2] = pal[2]*2 - 255;
  mat[1]->Diffuse[0] = 127-pal[0];
  mat[1]->Diffuse[1] = 127-pal[1];
  mat[1]->Diffuse[2] = 127-pal[2];
  mat[1]->Specular[0] = 127-pal[0];
  mat[1]->Specular[1] = 127-pal[1];
  mat[1]->Specular[2] = 127-pal[2];
  mat[1]->FadeDist = 10000.0;
  mat[1]->Texture = plReadPCXTex("sky.pcx",1,1);
  mat[1]->TexScaling = 45.0*LAND_SIZE/50000;
  mat[1]->PerspectiveCorrect = 32;

  // set up material 2 (the second sky)
  mat[2]->ShadeType = PL_SHADE_NONE;
  mat[2]->Shininess = 1;
  mat[2]->NumGradients = 1500;
  mat[2]->Texture = plReadPCXTex("sky2.pcx",1,1);
  mat[2]->TexScaling = 10.0; //200.0*LAND_SIZE/50000;
  mat[2]->PerspectiveCorrect = 2;

  // intialize the materials
  plMatInit(mat[0]);
  plMatInit(mat[1]);
  plMatInit(mat[2]);

  // make a nice palette
  plMatMakeOptPal2(pal,1,255,mat);

  // map the materials to this new palette
  plMatMapToPal(mat[0],pal,0,255);
  plMatMapToPal(mat[1],pal,0,255);
  plMatMapToPal(mat[2],pal,0,255);

  // set the new palette
  set_palette(pal);
}

pl_Obj *setup_landscape(pl_Mat *m, pl_Mat *sm, pl_Mat *sm2) {
  int i;
  // make our root object the land
  pl_Obj *o = plMakePlane(LAND_SIZE,LAND_SIZE,LAND_DIV-1,m);
  // give it a nice random bumpy effect
  for (i = 0; i < o->NumVertices; i ++)
    o->Vertices[i].y += (float) (rand()%1400)-700;
  // gotta recalculate normals for backface culling to work right
  plObjCalcNormals(o);

  // Make our first child the first sky
  o->Children[0] = plMakePlane(LAND_SIZE,LAND_SIZE,1,sm);
  o->Children[0]->Yp = 2000;
  o->Children[0]->BackfaceCull = 0;

  // and the second the second sky
  o->Children[1] = plMakeSphere(LAND_SIZE,10,10,sm2);
  o->Children[1]->Yp = 2000;
  plObjFlipNormals(o->Children[1]);

  return (o);
}

// VGA mode 13 functions

  // uses the fpu to copy memory quickly into non-cached memory (i.e. vram)
static void inline fpucopy(void *o, void *i, int len16) {
  __asm__ __volatile__("
    .align 4, 0x90
    0:
    fildq (%%esi)
    fildq 8(%%esi)
    fxch
    fistpq (%%edi)
    fistpq 8(%%edi)
    addl $16, %%edi
    addl $16, %%esi
    decl %%ecx
    jnz 0b
  "::"c" (len16), "S" (i), "D" (o)
   :"%ecx","%edi","%esi");
}

void inline vsync() {
  __asm__ __volatile__ ("movw $0x3DA, %%dx
    0: inb %%dx, %%al ; andb $8, %%al ; jnz 0b
    0: inb %%dx, %%al ; andb $8, %%al ; jz 0b"
    :::"%edx", "%eax");
}

void set_palette(char *pal) {
  __asm__ __volatile__("
    movl $768, %%ecx
    subl %%eax, %%eax
    movw $0x3C8, %%dx
    outb %%al, %%dx
    incw %%dx
    0:
      movb (%%esi), %%al
      #addl $2, %%eax
      shrl $2, %%eax
      outb %%al, %%dx
      incl %%esi
      subl %%eax, %%eax
      decl %%ecx
    jnz 0b
  "::"S" (pal):"%esi", "%eax", "%edx", "%ecx");
}

void set_mode13() {
  __djgpp_nearptr_enable();
  __asm__ __volatile__("int $0x10"::"a" (0x13):"%eax", "%ebx", "%ecx", "%edx");
}

void set_mode3() {
  __asm__ __volatile__("int $0x10"::"a" (3):"%eax", "%ebx", "%ecx", "%edx");
  __djgpp_nearptr_disable();
}

void mouse_init() {
  __asm__ __volatile__("
    subl %%eax, %%eax
    int $0x33
  ":"=a" (mouse_avail), "=b" (mouse_buttons)::"%eax","%ebx","%ecx","%edx");
  if (!mouse_avail) {
    printf("Mouse not found ... you won't be able to move\n");
  }
  if (mouse_buttons > 3) mouse_buttons = 3;
  printf("%d button mouse found\n",mouse_buttons);

}

void mouse_get() {
  static signed short int old_mouse_x, old_mouse_y;
  signed short int mx, my;
  if (mouse_avail) {
    __asm__ __volatile__("
      movl $11, %%eax
      int $0x33
    ":"=c" (mx), "=d" (my)::"%eax","%ebx","%ecx","%edx");
    mouse_x = (old_mouse_x + mx) * 0.5;
    mouse_y = (old_mouse_y + my) * 0.5;
    old_mouse_x = mx;
    old_mouse_y = my;
    __asm__ __volatile__("
      movl $3, %%eax
      int $0x33
    ":"=b" (mouse_b)::"%eax","%ebx","%ecx","%edx");
    mouse_b &= 7;
  } else mouse_x = mouse_y = mouse_b = 0;
}

void mouse_deinit() {
  // we don't really need to do anything
}
