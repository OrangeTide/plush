/******************************************************************************
**                         Plush Version 1.0                                 **
**                                                                           **
**       Perspective Correct Texture Mapping Z-Buffering Rasterizers         **
**                                                                           **
**             All code copyright (c) 1996-1997, Justin Frankel              **
******************************************************************************/

#include "plush.h"
#include "putface.h"

#define PL_PBITS 12

void plPF_PTexFZB(pl_CameraType *cam, pl_FaceType *TriFace) {
  pl_VertexType *sp[3];
  unsigned char *gmem = cam->frameBuffer;
  pl_ZBufferType *zbuf = cam->zBuffer;
  pl_FloatType MappingU1, MappingU2, MappingU3;
  pl_FloatType MappingV1, MappingV2, MappingV3;
  pl_sInt32Type MappingU_AND, MappingV_AND;
  unsigned char *texture;
  char twidth16minus;
  pl_uInt32Type twidthand;
  unsigned char bc;
  pl_TextureType *Texture;
  pl_sInt32Type clr = TriFace->Shade*65536.0;

  unsigned char n, nm, nmb;
  pl_FloatType U1,V1,U2,V2,dU1,dU2,dV1,dV2,dUL,dVL,UL,VL,nmdUL,nmdVL;
  pl_sInt32Type iUL, iVL, idUL, idVL, iULnext, iVLnext;

  pl_sInt32Type X1, X2, dX1, dX2, XL1, XL2;
  pl_ZBufferType Z1, ZL, dZ1, dZ2, dZL, Z2;
  pl_FloatType idZL, iZL;
  pl_sInt32Type Y1, Y2, Y0, dY;

  pl_uInt16Type *slb = cam->_ScanLineBuffer;
  
  Texture = TriFace->Material->Texture;

  if (!Texture) return;
  texture = Texture->Data;
  bc = TriFace->Material->_AddTable[plMin(65535,clr)>>8];
  nm = TriFace->Material->PerspectiveCorrect;
  nmb = 0; while (nm) { nmb++; nm >>= 1; }
  nmb = plMin(5,nmb);
  nm = 1<<nmb;
  MappingV_AND = (1<<Texture->Height)-1;
  MappingU_AND = (1<<Texture->Width)-1;
  twidth16minus = 16 - Texture->Width;
  twidthand = (MappingV_AND << Texture->Width);

  {
    PUTFACE_SORT_TEX();
  }

  MappingU1 /= sp[0]->xformedz*(1<<PL_PBITS);
  MappingV1 /= sp[0]->xformedz*(1<<PL_PBITS);
  MappingU2 /= sp[1]->xformedz*(1<<PL_PBITS);
  MappingV2 /= sp[1]->xformedz*(1<<PL_PBITS);
  MappingU3 /= sp[2]->xformedz*(1<<PL_PBITS);
  MappingV3 /= sp[2]->xformedz*(1<<PL_PBITS);

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
  dU2 = (MappingU3 - U1) / (Y2-Y0);
  dV2 = (MappingV3 - V1) / (Y2-Y0);
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
    dUL = dU1; dU1 = dU2; dU2 = dUL;
    dVL = dV1; dV1 = dV2; dV2 = dVL;
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
  } else dUL = dVL = dZL = 0;

  nmdUL = dUL*nm;
  nmdVL = dVL*nm;

  do {
    if (Y0 == Y1) {
      dY = (sp[2]->scry>>16)-(sp[1]->scry>>16);
      if (!dY) return;
      dX1 = (sp[2]->scrx-X1)/dY;
      dX2 = (sp[2]->scrx-X2)/dY;
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
        iZL = ZL;
        idZL = dZL * nm;
        idUL = idVL = 0;
        iUL = iULnext = ((pl_sInt32Type) (UL/ZL)) << PL_PBITS;
        iVL = iVLnext = ((pl_sInt32Type) (VL/ZL)) << PL_PBITS;
        do {
          register pl_FloatType t;
          UL += nmdUL;
          VL += nmdVL;
          iUL = iULnext;
          iVL = iVLnext;
          iZL += idZL;
          t = 1.0/iZL;
          iULnext = ((pl_sInt32Type) (UL*t)) << PL_PBITS;
          iVLnext = ((pl_sInt32Type) (VL*t)) << PL_PBITS;
          idUL = (iULnext - iUL)>>nmb;
          idVL = (iVLnext - iVL)>>nmb;
          n = nm;
          XL2 -= n; 
          if (XL2 < 0) n += XL2;
          do {
            if (*zbuf < ZL) {
              *zbuf = ZL;
              *gmem = bc + texture[((iUL>>16)&MappingU_AND) +
                                   ((iVL>>twidth16minus)&twidthand)];
            }
            zbuf++;
            gmem++;
            ZL += dZL;
            iUL += idUL;
            iVL += idVL;
          } while (--n);
        } while (XL2 > 0);
        gmem -= XL1;
        zbuf -= XL1;
      }
    }
    zbuf += cam->ScreenWidth;
    gmem += cam->ScreenWidth;
    Z1 += dZ1;
    U1 += dU1;
    V1 += dV1;
    X1 += dX1;
    X2 += dX2;
  } while (++Y0 < Y2);
}

void plPF_PTexGZB(pl_CameraType *cam, pl_FaceType *TriFace) {
  pl_VertexType *sp[3];
  unsigned char *gmem = cam->frameBuffer;
  pl_ZBufferType *zbuf = cam->zBuffer;
  pl_FloatType MappingU1, MappingU2, MappingU3;
  pl_FloatType MappingV1, MappingV2, MappingV3;
  pl_sInt32Type MappingU_AND, MappingV_AND;
  unsigned char *texture;
  char twidth16minus;
  pl_uInt32Type twidthand;
  unsigned char *addtable;
  pl_TextureType *Texture;

  unsigned char n, nm, nmb;
  pl_FloatType U1,V1,U2,V2,dU1,dU2,dV1,dV2,dUL,dVL,UL,VL,nmdUL,nmdVL;
  pl_sInt32Type iUL, iVL, idUL, idVL, iULnext, iVLnext;

  pl_sInt32Type X1, X2, dX1, dX2, XL1, XL2;
  pl_sInt32Type C1, C2, dC1, dC2, OC2, OC3, CL, dCL;
  pl_ZBufferType Z1, ZL, dZ1, dZ2, dZL, Z2;
  pl_FloatType idZL, iZL;
  pl_sInt32Type Y1, Y2, Y0, dY;

  pl_uInt16Type *slb = cam->_ScanLineBuffer;
  
  Texture = TriFace->Material->Texture;

  if (!Texture) return;
  texture = Texture->Data;
  addtable = TriFace->Material->_AddTable;
  nm = TriFace->Material->PerspectiveCorrect;
  nmb = 0; while (nm) { nmb++; nm >>= 1; }
  nmb = plMin(5,nmb);
  nm = 1<<nmb;
  MappingV_AND = (1<<Texture->Height)-1;
  MappingU_AND = (1<<Texture->Width)-1;
  twidth16minus = 16 - Texture->Width;
  twidthand = (MappingV_AND << Texture->Width);

  {
    PUTFACE_SORT_TEX();
  }

  MappingU1 /= sp[0]->xformedz*(1<<PL_PBITS);
  MappingV1 /= sp[0]->xformedz*(1<<PL_PBITS);
  MappingU2 /= sp[1]->xformedz*(1<<PL_PBITS);
  MappingV2 /= sp[1]->xformedz*(1<<PL_PBITS);
  MappingU3 /= sp[2]->xformedz*(1<<PL_PBITS);
  MappingV3 /= sp[2]->xformedz*(1<<PL_PBITS);

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
    dUL = dU1; dU1 = dU2; dU2 = dUL;
    dVL = dV1; dV1 = dV2; dV2 = dVL;
    dZL = dZ1; dZ1 = dZ2; dZ2 = dZL;
    dC2 ^= dC1; dC1 ^= dC2; dC2 ^= dC1;
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
  } else { dUL = dVL = dZL = 0; dCL = 0; }

  nmdUL = dUL*nm;
  nmdVL = dVL*nm;

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
        iZL = ZL;
        idZL = dZL * nm;
        idUL = idVL = 0;
        iUL = iULnext = ((pl_sInt32Type) (UL/ZL)) << PL_PBITS;
        iVL = iVLnext = ((pl_sInt32Type) (VL/ZL)) << PL_PBITS;
        do {
          register pl_FloatType t;
          UL += nmdUL;
          VL += nmdVL;
          iUL = iULnext;
          iVL = iVLnext;
          iZL += idZL;
          t = 1.0/iZL;
          iULnext = ((pl_sInt32Type) (UL*t)) << PL_PBITS;
          iVLnext = ((pl_sInt32Type) (VL*t)) << PL_PBITS;
          idUL = (iULnext - iUL)>>nmb;
          idVL = (iVLnext - iVL)>>nmb;
          n = nm;
          XL2 -= n;  
          if (XL2 < 0) n += XL2;
          do {
            if (*zbuf < ZL) {
              *zbuf = ZL;
              *gmem = addtable[CL>>8] +
                      texture[((iUL>>16)&MappingU_AND) +
                              ((iVL>>twidth16minus)&twidthand)];
            }
            zbuf++;
            gmem++;
            ZL += dZL;
            CL += dCL;
            iUL += idUL;
            iVL += idVL;
          } while (--n);
        } while (XL2 > 0);
        gmem -= XL1;
        zbuf -= XL1;
      }
    }
    zbuf += cam->ScreenWidth;
    gmem += cam->ScreenWidth;
    Z1 += dZ1;
    U1 += dU1;
    V1 += dV1;
    X1 += dX1;
    X2 += dX2;
    C1 += dC1;
  } while (++Y0 < Y2);
}
