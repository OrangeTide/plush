/******************************************************************************
**                         Plush Version 1.0                                 **
**                                                                           **
**              Affine Texture Mapping Z-Buffering Rasterizers               **
**                                                                           **
**             All code copyright (c) 1996-1997, Justin Frankel              **
******************************************************************************/

#include "plush.h"
#include "putface.h"

static void _plPF_TexFSZB(pl_CameraType *cam, pl_FaceType *TriFace, char env);
static void _plPF_TexGSZB(pl_CameraType *cam, pl_FaceType *TriFace, char env);

void plPF_TexFZB(pl_CameraType *cam, pl_FaceType *TriFace) {
  _plPF_TexFSZB(cam,TriFace,0);
}

void plPF_EnvFZB(pl_CameraType *cam, pl_FaceType *TriFace) {
  _plPF_TexFSZB(cam,TriFace,1);
}

void plPF_TexGZB(pl_CameraType *cam, pl_FaceType *TriFace) {
  _plPF_TexGSZB(cam,TriFace,0);
}

void plPF_EnvGZB(pl_CameraType *cam, pl_FaceType *TriFace) {
  _plPF_TexGSZB(cam,TriFace,1);
}

void plPF_TexEnvZB(pl_CameraType *cam, pl_FaceType *TriFace) {
  pl_VertexType *sp[3];
  unsigned char *gmem = cam->frameBuffer;
  pl_ZBufferType *zbuf = cam->zBuffer;
  pl_sInt32Type MappingU1, MappingU2, MappingU3;
  pl_sInt32Type MappingV1, MappingV2, MappingV3;
  pl_sInt32Type MappingU_AND, MappingV_AND;
  pl_sInt32Type eMappingU1, eMappingU2, eMappingU3;
  pl_sInt32Type eMappingV1, eMappingV2, eMappingV3;
  pl_sInt32Type eMappingU_AND, eMappingV_AND;
  unsigned char *texture, *environment;
  char twidth16minus;
  pl_uInt32Type twidthand;
  char ewidth16minus;
  pl_uInt32Type ewidthand;
  unsigned char *addtable;
  pl_TextureType *Texture, *Environment;

  pl_sInt32Type U1, V1, U2, V2, dU1, dU2, dV1, dV2, dUL, dVL, UL, VL;
  pl_sInt32Type eU1, eV1, eU2, eV2, edU1, edU2, 
                   edV1, edV2, edUL, edVL, eUL, eVL;
  pl_sInt32Type X1, X2, dX1, dX2, XL1, XL2;
  pl_ZBufferType Z1, ZL, dZ1, dZ2, dZL, Z2;
  pl_sInt32Type Y1, Y2, Y0, dY;

  pl_uInt16Type *slb = cam->_ScanLineBuffer;
  
  Environment = TriFace->Material->Environment;
  Texture = TriFace->Material->Texture;

  if (!Texture || !Environment) return;
  texture = Texture->Data;
  environment = Environment->Data;
  addtable = TriFace->Material->_AddTable;

  MappingV_AND = (1<<Texture->Height)-1;
  MappingU_AND = (1<<Texture->Width)-1;
  twidth16minus = 16 - Texture->Width;
  twidthand = (MappingV_AND << Texture->Width);
  eMappingV_AND = (1<<Environment->Height)-1;
  eMappingU_AND = (1<<Environment->Width)-1;
  ewidth16minus = 16 - Environment->Width;
  ewidthand = (eMappingV_AND << Environment->Width);

  PUTFACE_SORT_TEXENV();

  U1 = U2 = MappingU1;
  V1 = V2 = MappingV1;
  eU1 = eU2 = eMappingU1;
  eV1 = eV2 = eMappingV1;
  X2 = X1 = sp[0]->scrx;
  Z2 = Z1 = 1/sp[0]->xformedz;
  Y0 = sp[0]->scry>>16;
  Y1 = sp[1]->scry>>16;
  Y2 = sp[2]->scry>>16;

  if (Y2==Y0) Y2++;
  dY = Y1-Y0;
  dX1 = (sp[1]->scrx - X1) / (dY ? dY : 1);
  dU1 = (MappingU2 - U1) / (dY ? dY : 1);
  dV1 = (MappingV2 - V1) / (dY ? dY : 1);
  edU1 = (eMappingU2 - eU1) / (dY ? dY : 1);
  edV1 = (eMappingV2 - eV1) / (dY ? dY : 1);
  dZ1 = (1/sp[1]->xformedz - Z1) / (dY ? dY : 1);
  dX2 = (sp[2]->scrx - X1) / (Y2-Y0);
  dU2 = (MappingU3 - U1) / (Y2-Y0);
  dV2 = (MappingV3 - V1) / (Y2-Y0);
  edU2 = (eMappingU3 - eU1) / (Y2-Y0);
  edV2 = (eMappingV3 - eV1) / (Y2-Y0);
  dZ2 = (1/sp[2]->xformedz - Z1) / (Y2-Y0);

  if (!dY) {
    if (sp[1]->scrx > X1) {
      X2 = sp[1]->scrx;
      Z2 = 1/sp[1]->xformedz;
      U2 = MappingU2;
      V2 = MappingV2;
      eU2 = eMappingU2;
      eV2 = eMappingV2;
    } else {
      X1 = sp[1]->scrx;
      Z1 = 1/sp[1]->xformedz;
      U1 = MappingU2;
      V1 = MappingV2;
      eU1 = eMappingU2;
      eV1 = eMappingV2;
    }
  } else if (dX2 < dX1) {
    dX2 ^= dX1; dX1 ^= dX2; dX2 ^= dX1;
    dU2 ^= dU1; dU1 ^= dU2; dU2 ^= dU1;
    dV2 ^= dV1; dV1 ^= dV2; dV2 ^= dV1;
    edU2 ^= edU1; edU1 ^= edU2; edU2 ^= edU1;
    edV2 ^= edV1; edV1 ^= edV2; edV2 ^= edV1;
    dZL = dZ1; dZ1 = dZ2; dZ2 = dZL;
  }


  if (Y0 >= cam->ClipBottom) return;
  Y2 = plMin(cam->ClipBottom,Y2);

  gmem += (Y0 * cam->ScreenWidth);
  zbuf += (Y0 * cam->ScreenWidth);

  if (((dX1-dX2)*dY)>>16) {
    dUL = ((dU1-dU2)*dY)/(((dX1-dX2)*dY)>>16);
    dVL = ((dV1-dV2)*dY)/(((dX1-dX2)*dY)>>16);
    edUL = ((edU1-edU2)*dY)/(((dX1-dX2)*dY)>>16);
    edVL = ((edV1-edV2)*dY)/(((dX1-dX2)*dY)>>16);
    dZL = ((dZ1-dZ2)*dY)/(((dX1-dX2)*dY)>>16);
  } else if ((X2-X1)>>16) {
    edUL = (eU2-eU1)/((X2-X1)>>16);
    edVL = (eV2-eV1)/((X2-X1)>>16);
    dUL = (U2-U1)/((X2-X1)>>16);
    dVL = (V2-V1)/((X2-X1)>>16);
    dZL = (Z2-Z1)/((X2-X1)>>16);
  } else { dUL = dVL = edVL = edUL = 0; dZL = 0; }

  do {
    if (Y0 == Y1) {
      dY = (sp[2]->scry>>16)-(sp[1]->scry>>16);
      if (!dY) return;
      dX1 = (sp[2]->scrx-X1)/dY;
      dX2 = (sp[2]->scrx-X2)/dY;
      dV1 = (MappingV3 - V1) / dY;
      dU1 = (MappingU3 - U1) / dY;
      edV1 = (eMappingV3 - eV1) / dY;
      edU1 = (eMappingU3 - eU1) / dY;
      dZ1 = (1/sp[2]->xformedz-Z1)/dY;
    }
    if (Y0 >= cam->ClipTop) {
      XL1 = (X1+32768)>>16;
      XL2 = (X2+32768)>>16;
      ZL = Z1;
      UL = U1;
      VL = V1;
      eUL = eU1;
      eVL = eV1;
      if (XL1 < cam->ClipLeft) {  
        UL += dUL*(cam->ClipLeft-XL1);
        VL += dVL*(cam->ClipLeft-XL1);
        eUL += edUL*(cam->ClipLeft-XL1);
        eVL += edVL*(cam->ClipLeft-XL1);
        ZL += dZL*(cam->ClipLeft-XL1);
        XL1 = cam->ClipLeft;
      }
      XL2 = plMin(cam->ClipRight, XL2);
      if ((XL2-XL1) > 0) {
        if (slb) {
          if (XL1 < *(slb + (Y0<<2))) *(slb + (Y0<<2)) = XL1;
          if (XL2 > *(slb + (Y0<<2) + 1)) *(slb + (Y0<<2) + 1) = XL2;
        }
        XL2 -= XL1;
        gmem += XL1;
        zbuf += XL1;
        XL1 += XL2;
        do {
          if (*zbuf < ZL) {
            *zbuf = ZL;
            *gmem = addtable[environment[
                ((eUL>>16)&eMappingU_AND)+((eVL>>ewidth16minus)&ewidthand)]] +
                            texture[((UL>>16)&MappingU_AND) +
                                    ((VL>>twidth16minus)&twidthand)];
          }
          zbuf++;
          gmem++;
          ZL += dZL;
          UL += dUL;
          VL += dVL;
          eUL += edUL;
          eVL += edVL;
        } while (--XL2);
        gmem -= XL1;
        zbuf -= XL1;
      }
    }
    zbuf += cam->ScreenWidth;
    gmem += cam->ScreenWidth;
    Z1 += dZ1;
    X1 += dX1;
    X2 += dX2;
    U1 += dU1;
    V1 += dV1;
    eU1 += edU1;
    eV1 += edV1;
  } while (++Y0 < Y2);
}

static void _plPF_TexFSZB(pl_CameraType *cam, pl_FaceType *TriFace, char env) {
  pl_VertexType *sp[3];
  unsigned char *gmem = cam->frameBuffer;
  pl_ZBufferType *zbuf = cam->zBuffer;
  pl_sInt32Type MappingU1, MappingU2, MappingU3;
  pl_sInt32Type MappingV1, MappingV2, MappingV3;
  pl_sInt32Type MappingU_AND, MappingV_AND;
  unsigned char *texture;
  char twidth16minus;
  pl_uInt32Type twidthand;
  pl_sInt32Type clr;
  pl_uInt16Type *slb = cam->_ScanLineBuffer;
  unsigned char bc;
  pl_TextureType *Texture;

  pl_ZBufferType Z1, ZL, dZ1, dZL, Z2, dZ2;
  pl_sInt32Type dU1, dV1, dU2, dV2, U1, V1, U2, V2;
  pl_sInt32Type dUL, dVL, UL, VL;
  pl_sInt32Type X1, X2, dX1, dX2, XL1, XL2;
  pl_sInt32Type Y1, Y2, Y0, dY;

  if (env) Texture = TriFace->Material->Environment;
  else Texture = TriFace->Material->Texture;

  if (!Texture) return;
  clr = TriFace->Shade*65536.0;
  bc = TriFace->Material->_AddTable[plMin(65535,clr)>>8];
  texture = Texture->Data;
  twidth16minus = 16 - Texture->Width;
  MappingV_AND = (1<<Texture->Height)-1;
  MappingU_AND = (1<<Texture->Width)-1;
  twidthand = (MappingV_AND << Texture->Width);
 
  if (env) {
    PUTFACE_SORT_ENV();
  } else {
    PUTFACE_SORT_TEX();
  }

  U1 = U2 = MappingU1;
  V1 = V2 = MappingV1;
  X2 = X1 = sp[0]->scrx;
  Z2 = Z1 = 1/sp[0]->xformedz;
  Y0 = sp[0]->scry>>16;
  Y1 = sp[1]->scry>>16;
  Y2 = sp[2]->scry>>16;

  if (Y2==Y0) Y2++;
  dY = Y1-Y0;
  dX1 = (sp[1]->scrx - X1) / (dY ? dY : 1);
  dU1 = (MappingU2 - U1) / (dY ? dY : 1);
  dV1 = (MappingV2 - V1) / (dY ? dY : 1);
  dZ1 = (1/sp[1]->xformedz - Z1) / (dY ? dY : 1);
  dX2 = (sp[2]->scrx - X1) / (Y2-Y0);
  dV2 = (MappingV3 - V1) / (Y2-Y0);
  dU2 = (MappingU3 - U1) / (Y2-Y0);
  dZ2 = (1/sp[2]->xformedz - Z1) / (Y2-Y0);

  if (!dY) {
    if (sp[1]->scrx > X1) {
      X2 = sp[1]->scrx;
      Z2 = 1/sp[1]->xformedz;
      U2 = MappingU2;
      V2 = MappingV2;
    } else {
      X1 = sp[1]->scrx;
      Z1 = 1/sp[1]->xformedz;
      U1 = MappingU2;
      V1 = MappingV2;
    }
  } else if (dX2 < dX1) {
    dX2 ^= dX1; dX1 ^= dX2; dX2 ^= dX1;
    dU2 ^= dU1; dU1 ^= dU2; dU2 ^= dU1;
    dV2 ^= dV1; dV1 ^= dV2; dV2 ^= dV1;
    dZL = dZ1; dZ1 = dZ2; dZ2 = dZL;
  }

  if (Y0 >= cam->ClipBottom) return;
  Y2 = plMin(cam->ClipBottom,Y2);

  gmem += (Y0 * cam->ScreenWidth);
  zbuf += (Y0 * cam->ScreenWidth);

  if (((dX1-dX2)*dY)>>16) {
    dUL = ((dU1-dU2)*dY)/(((dX1-dX2)*dY)>>16);
    dVL = ((dV1-dV2)*dY)/(((dX1-dX2)*dY)>>16);
    dZL = ((dZ1-dZ2)*dY)/(((dX1-dX2)*dY)>>16);
  } else if ((X2-X1)>>16) {
    dUL = (U2-U1)/((X2-X1)>>16);
    dVL = (V2-V1)/((X2-X1)>>16);
    dZL = (Z2-Z1)/((X2-X1)>>16);
  } else { dUL = dVL = 0; dZL = 0; }

  do {
    if (Y0 == Y1) {
      dY = (sp[2]->scry>>16) - (sp[1]->scry>>16);
      if (!dY) return;
      dX1 = (sp[2]->scrx-X1) / dY;
      dX2 = (sp[2]->scrx-X2) / dY;
      dV1 = (MappingV3 - V1) / dY;
      dU1 = (MappingU3 - U1) / dY;
      dZ1 = (1/sp[2]->xformedz-Z1)/dY;
    }
    if (Y0 >= cam->ClipTop) {
      XL1 = (X1+32768)>>16;
      XL2 = (X2+32768)>>16;
      ZL = Z1;
      UL = U1;
      VL = V1;
      if (XL1 < cam->ClipLeft) {  
        UL += dUL*(cam->ClipLeft-XL1);
        VL += dVL*(cam->ClipLeft-XL1);
        ZL += dZL*(cam->ClipLeft-XL1);
        XL1 = cam->ClipLeft;
      }
      XL2 = plMin(cam->ClipRight, XL2);
      if ((XL2-XL1) > 0) {
        if (slb) {
          if (XL1 < *(slb + (Y0<<2))) *(slb + (Y0<<2)) = XL1;
          if (XL2 > *(slb + (Y0<<2) + 1)) *(slb + (Y0<<2) + 1) = XL2;
        }
        XL2 -= XL1;
        gmem += XL1;
        zbuf += XL1;
        XL1 += XL2;
        do {
          if (*zbuf < ZL) {
            *zbuf = ZL;
            *gmem = bc + texture[((UL >> 16)&MappingU_AND) +
                                ((VL>>twidth16minus)&twidthand)];
          }
          zbuf++;
          gmem++;
          ZL += dZL;
          UL += dUL;
          VL += dVL;
        } while (--XL2);
        gmem -= XL1;
        zbuf -= XL1;
      }
    } 
    zbuf += cam->ScreenWidth;
    gmem += cam->ScreenWidth;
    X1 += dX1;
    X2 += dX2;    
    U1 += dU1; 
    V1 += dV1;
    Z1 += dZ1;
  } while (++Y0 < Y2);
}

static void _plPF_TexGSZB(pl_CameraType *cam, pl_FaceType *TriFace, char env) {
  pl_VertexType *sp[3];
  unsigned char *gmem = cam->frameBuffer;
  pl_ZBufferType *zbuf = cam->zBuffer;
  pl_sInt32Type MappingU1, MappingU2, MappingU3;
  pl_sInt32Type MappingV1, MappingV2, MappingV3;
  pl_sInt32Type MappingU_AND, MappingV_AND;
  unsigned char *texture;
  char twidth16minus;
  pl_uInt32Type twidthand;
  unsigned char *addtable;
  pl_TextureType *Texture;

  pl_sInt32Type U1, V1, U2, V2, dU1, dU2, dV1, dV2, dUL, dVL, UL, VL;
  pl_sInt32Type X1, X2, dX1, dX2, XL1, XL2;
  pl_sInt32Type C1, C2, dC1, dC2, OC2, OC3, CL, dCL;
  pl_ZBufferType Z1, ZL, dZ1, dZ2, dZL, Z2;
  pl_sInt32Type Y1, Y2, Y0, dY;

  pl_uInt16Type *slb = cam->_ScanLineBuffer;
  
  if (env) Texture = TriFace->Material->Environment;
  else Texture = TriFace->Material->Texture;

  if (!Texture) return;
  texture = Texture->Data;
  addtable = TriFace->Material->_AddTable;
  twidth16minus = 16 - Texture->Width;
  MappingV_AND = (1<<Texture->Height)-1;
  MappingU_AND = (1<<Texture->Width)-1;
  twidthand = (MappingV_AND << Texture->Width);

  if (env) {
    PUTFACE_SORT_ENV();
  } else {
    PUTFACE_SORT_TEX();
  }

  C1 = C2 = sp[0]->Shade*65535.0;
  OC2 = sp[1]->Shade*65535.0;
  OC3 = sp[2]->Shade*65535.0;
  U1 = U2 = MappingU1;
  V1 = V2 = MappingV1;
  X2 = X1 = sp[0]->scrx;
  Z2 = Z1 = 1/sp[0]->xformedz;
  Y0 = sp[0]->scry>>16;
  Y1 = sp[1]->scry>>16;
  Y2 = sp[2]->scry>>16;

  if (Y2==Y0) Y2++;
  dY = Y1-Y0;
  dX1 = (sp[1]->scrx - X1) / (dY ? dY : 1);
  dU1 = (MappingU2 - U1) / (dY ? dY : 1);
  dV1 = (MappingV2 - V1) / (dY ? dY : 1);
  dC1 = (OC2 - C1) / (dY ? dY : 1);
  dZ1 = (1/sp[1]->xformedz - Z1) / (dY ? dY : 1);
  dX2 = (sp[2]->scrx - X1) / (Y2-Y0);
  dU2 = (MappingU3 - U1) / (Y2-Y0);
  dV2 = (MappingV3 - V1) / (Y2-Y0);
  dZ2 = (1/sp[2]->xformedz - Z1) / (Y2-Y0);
  dC2 = (OC3 - C1) / (Y2-Y0);

  if (!dY) {
    if (sp[1]->scrx > X1) {
      X2 = sp[1]->scrx;
      Z2 = 1/sp[1]->xformedz;
      U2 = MappingU2;
      V2 = MappingV2;
      C2 = OC2;
    } else {
      X1 = sp[1]->scrx;
      Z1 = 1/sp[1]->xformedz;
      U1 = MappingU2;
      V1 = MappingV2;
      C1 = OC2;
    }
  } else if (dX2 < dX1) {
    dX2 ^= dX1; dX1 ^= dX2; dX2 ^= dX1;
    dU2 ^= dU1; dU1 ^= dU2; dU2 ^= dU1;
    dV2 ^= dV1; dV1 ^= dV2; dV2 ^= dV1;
    dC2 ^= dC1; dC1 ^= dC2; dC2 ^= dC1;
    dZL = dZ1; dZ1 = dZ2; dZ2 = dZL;
  }


  if (Y0 >= cam->ClipBottom) return;
  Y2 = plMin(cam->ClipBottom,Y2);

  gmem += (Y0 * cam->ScreenWidth);
  zbuf += (Y0 * cam->ScreenWidth);

  if (((dX1-dX2)*dY)>>16) {
    dUL = ((dU1-dU2)*dY)/(((dX1-dX2)*dY)>>16);
    dVL = ((dV1-dV2)*dY)/(((dX1-dX2)*dY)>>16);
    dZL = ((dZ1-dZ2)*dY)/(((dX1-dX2)*dY)>>16);
    dCL = ((dC1-dC2)*dY)/(((dX1-dX2)*dY)>>16);
  } else if ((X2-X1)>>16) {
    dUL = (U2-U1)/((X2-X1)>>16);
    dVL = (V2-V1)/((X2-X1)>>16);
    dZL = (Z2-Z1)/((X2-X1)>>16);
    dCL = (C2-C1)/(((X2-X1)>>16)+2);
  } else { dUL = dCL = dVL = 0; dZL = 0; }

  do {
    if (Y0 == Y1) {
      dY = (sp[2]->scry>>16)-(sp[1]->scry>>16);
      if (!dY) return;
      dX1 = (sp[2]->scrx-X1)/dY;
      dX2 = (sp[2]->scrx-X2)/dY;
      dV1 = (MappingV3 - V1) / dY;
      dU1 = (MappingU3 - U1) / dY;
      dC1 = (OC3-C1)/dY;
      dZ1 = (1/sp[2]->xformedz-Z1)/dY;
    }
    if (Y0 >= cam->ClipTop) {
      XL1 = (X1+32768)>>16;
      XL2 = (X2+32768)>>16;
      CL = C1;
      ZL = Z1;
      UL = U1;
      VL = V1;
      if (XL1 < cam->ClipLeft) {  
        UL += dUL*(cam->ClipLeft-XL1);
        VL += dVL*(cam->ClipLeft-XL1);
        CL += dCL*(cam->ClipLeft-XL1);
        ZL += dZL*(cam->ClipLeft-XL1);
        XL1 = cam->ClipLeft;
      }
      XL2 = plMin(cam->ClipRight, XL2);
      if ((XL2-XL1) > 0) {
        if (slb) {
          if (XL1 < *(slb + (Y0<<2))) *(slb + (Y0<<2)) = XL1;
          if (XL2 > *(slb + (Y0<<2) + 1)) *(slb + (Y0<<2) + 1) = XL2;
        }
        XL2 -= XL1;
        gmem += XL1;
        zbuf += XL1;
        XL1 += XL2;
        do {
          if (*zbuf < ZL) {
            *zbuf = ZL;
            *gmem = addtable[CL>>8] +
                            texture[((UL>>16)&MappingU_AND) +
                                    ((VL>>twidth16minus)&twidthand)];
          }
          zbuf++;
          gmem++;
          ZL += dZL;
          CL += dCL;
          UL += dUL;
          VL += dVL;
        } while (--XL2);
        gmem -= XL1;
        zbuf -= XL1;
      }
    }
    zbuf += cam->ScreenWidth;
    gmem += cam->ScreenWidth;
    Z1 += dZ1;
    X1 += dX1;
    X2 += dX2;
    C1 += dC1;
    U1 += dU1;
    V1 += dV1;
  } while (++Y0 < Y2);
}
