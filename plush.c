/******************************************************************************
**                           Plush Version 1.0                               **
**                                                                           **
**                  Misc Light, Camera, and Face Code                        **
**                                                                           **
**             All code copyright (c) 1996-1997, Justin Frankel              **
******************************************************************************/

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "plush.h"

char plVersionString[] = "Plush 3D Version 1.0.00";
char plCopyrightString[] = "Copyright (C) 1996-1997, Justin Frankel";

/*
** Renders "face" to "cam". Chooses the appropriate rasterizer by using 
** "cam" and "face"->Material.
*/
void plPutFace(pl_CameraType *cam, pl_FaceType *face) {
  unsigned char *frame;
  pl_ZBufferType *zb;
  pl_MaterialType *m = face->Material;
  frame = cam->frameBuffer;
  zb = cam->zBuffer;
  if (zb) switch (m->_ft) {
    case PL_FILL_TRANSPARENT: switch(m->_st) {
      case PL_SHADE_NONE:
      case PL_SHADE_FLAT: plPF_TransFZB(cam, face); break;
      case PL_SHADE_GOURAUD: plPF_TransGZB(cam, face); break;
    } break;
    case PL_FILL_SOLID: switch(m->_st) {
      case PL_SHADE_NONE:
      case PL_SHADE_FLAT: plPF_SolidFZB(cam, face); break;
      case PL_SHADE_GOURAUD: plPF_SolidGZB(cam, face); break;
    } break;
    case PL_FILL_ENVIRONMENT: switch (m->_st) {
      case PL_SHADE_NONE:
      case PL_SHADE_FLAT: plPF_EnvFZB(cam, face); break;
      case PL_SHADE_GOURAUD: plPF_EnvGZB(cam, face); break;
    } break;
    case PL_FILL_TEXTURE: if (m->PerspectiveCorrect) switch (m->_st) {
      case PL_SHADE_NONE:
      case PL_SHADE_FLAT: plPF_PTexFZB(cam, face); break;
      case PL_SHADE_GOURAUD: plPF_PTexGZB(cam, face); break;
    } else switch (m->_st) {
      case PL_SHADE_NONE:
      case PL_SHADE_FLAT: plPF_TexFZB(cam, face); break;
      case PL_SHADE_GOURAUD: plPF_TexGZB(cam, face); break;
    } break;
    case PL_FILL_TEXTURE|PL_FILL_ENVIRONMENT: 
      plPF_TexEnvZB(cam, face);
    break;
  } else switch (m->_ft) {
    case PL_FILL_TRANSPARENT: switch(m->_st) {
      case PL_SHADE_NONE:
      case PL_SHADE_FLAT: plPF_TransF(cam, face); break;
      case PL_SHADE_GOURAUD: plPF_TransG(cam, face); break;
    } break;
    case PL_FILL_SOLID: switch (m->_st) {
      case PL_SHADE_NONE:
      case PL_SHADE_FLAT: plPF_SolidF(cam, face); break;
      case PL_SHADE_GOURAUD: plPF_SolidG(cam, face); break;
    } break;
    case PL_FILL_ENVIRONMENT: switch (m->_st) {
      case PL_SHADE_NONE:
      case PL_SHADE_FLAT: plPF_EnvF(cam, face); break;
      case PL_SHADE_GOURAUD: plPF_EnvG(cam, face); break;
    } break;
    case PL_FILL_TEXTURE: if (m->PerspectiveCorrect) switch (m->_st) {
      case PL_SHADE_NONE:
      case PL_SHADE_FLAT: plPF_PTexF(cam, face); break;
      case PL_SHADE_GOURAUD:plPF_PTexG(cam, face); break;
    } else switch (m->_st) {
      case PL_SHADE_NONE:
      case PL_SHADE_FLAT: plPF_TexF(cam, face); break;
      case PL_SHADE_GOURAUD: plPF_TexG(cam, face); break;
    } break;
    case PL_FILL_TEXTURE|PL_FILL_ENVIRONMENT: 
      plPF_TexEnv(cam, face);
    break;
  }
}

pl_LightType *plSetLight(pl_LightType *light, unsigned char mode,
                         pl_FloatType x, pl_FloatType y, pl_FloatType z,
                         pl_FloatType intensity) {
  pl_FloatType m[16], m2[16];
  light->Type = mode;
  light->Intensity = intensity;
  switch (mode) {
    case PL_LIGHT_VECTOR:
      plMatrixRotate(m,1,x);
      plMatrixRotate(m2,2,y);
      plMatrixMultiply(m,m2);
      plMatrixRotate(m2,3,z);
      plMatrixMultiply(m,m2);
      plMatrixApply(m,0.0,0.0,-1.0,&light->Xp, &light->Yp, &light->Zp);
    break;
    case PL_LIGHT_POINT:
      light->Xp = x;
      light->Yp = y; 
      light->Zp = z;
    break;
  }
  return light;
}

pl_LightType *plNewLight() {
  pl_LightType *l;
  l = malloc(sizeof(pl_LightType));
  if (!l) return 0;
  memset(l,0,sizeof(pl_LightType));
  return (l);
}

void plFreeLight(pl_LightType *l) {
  free(l);
}

void plFreeTexture(pl_TextureType *t) {
  free(t->Data);
  free(t->PaletteData);
  free(t);
}

void plFreeCamera(pl_CameraType *c) {
  plFreeScanLineBuffers(c);
  free(c);
}

void plSetCameraTarget(pl_CameraType *c, pl_FloatType x, pl_FloatType y,
                       pl_FloatType z) {
  pl_FloatType dx, dy, dz;
  dx = x - c->Xp;
  dy = y - c->Yp;
  dz = z - c->Zp;
  if (dz > 0.0001) {
    c->Ya = -atan(dx/dz)*180/PL_PI;
    dz /= cos(c->Ya*PL_PI/180.0);
    c->Xa = atan(dy/dz)*180/PL_PI;
  } else if (dz < -0.0001) { 
    c->Ya = 180-atan(dx/dz)*180/PL_PI;
    dz /= cos((c->Ya-180)*PL_PI/180.0);
    c->Xa = -atan(dy/dz)*180/PL_PI;
  } else {
    c->Ya = 0.0;
    c->Xa = -90.0;
  }
}

pl_CameraType *plNewCamera(pl_sInt32Type sw, pl_sInt32Type sh, 
                           pl_FloatType ar, pl_FloatType fov,
                           unsigned char ds, unsigned char *fb,
                           pl_ZBufferType *zb) {
  pl_CameraType *c;
  c = malloc(sizeof(pl_CameraType));
  if (!c) return 0;
  memset(c,0,sizeof(pl_CameraType));
  c->Fov = fov;
  c->AspectRatio = ar;
  c->ClipRight = c->ScreenWidth = sw;
  c->ClipBottom = c->ScreenHeight = sh;
  c->CenterX = sw>>1;
  c->CenterY = sh>>1;
  c->ClipFront = 0.01;
  c->ClipBack = 8e30;
  c->frameBuffer = fb;
  c->zBuffer = zb;
  c->Sort = c->EnvEnabled = c->GouraudEnabled = 1;
  if (zb) c->Sort = 0;
  if (ds) plInitScanLineBuffers(c);
  return (c);
}
