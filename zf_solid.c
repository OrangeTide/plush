/******************************************************************************
**                         Plush Version 1.0                                 **
**                                                                           **
**                  Solid Z-Buffering Rasterizers                            **
**                                                                           **
**             All code copyright (c) 1996-1997, Justin Frankel              **
******************************************************************************/

#include "plush.h"
#include "putface.h"

void plPF_SolidFZB(pl_CameraType *cam, pl_FaceType *TriFace) {
  pl_VertexType *sp[3];
  unsigned char *gmem = cam->frameBuffer;
  pl_ZBufferType *zbuf = cam->zBuffer;
  pl_sInt32Type X1, X2, dX1, dX2, XL1, XL2;
  pl_ZBufferType dZL, dZ1, dZ2, Z1, ZL, Z2;
  pl_sInt32Type Y1, Y2, Y0, dY;
  pl_uInt16Type *slb = cam->_ScanLineBuffer;
  unsigned char bc = TriFace->Material->_ColorBase + 
                     TriFace->Shade*(TriFace->Material->_ColorsUsed - 1);

  PUTFACE_SORT();

  X2 = X1 = sp[0]->scrx;
  Z2 = Z1 = 1.0 / sp[0]->xformedz;
  Y0 = sp[0]->scry>>16;
  Y1 = sp[1]->scry>>16;
  Y2 = sp[2]->scry>>16;

  if (Y2 == Y0) Y2++;
  dY = Y1-Y0;
  dX1 = (sp[1]->scrx - X1) / (dY ? dY : 1);
  dZ1 = ((1/sp[1]->xformedz) - Z1) / (dY ? dY : 1);
  dX2 = (sp[2]->scrx - X1) / (Y2 - Y0);
  dZ2 = ((1/sp[2]->xformedz) - Z1) / (Y2 - Y0);
  if (Y1 == Y0) {
    if (sp[1]->scrx > X1) {
      X2 = sp[1]->scrx;
      Z2 = 1/sp[1]->xformedz;
    } else {
      X1 = sp[1]->scrx;
      Z1 = 1/sp[1]->xformedz;
    }
  } else if (dX2 < dX1) {
    dX2 ^= dX1; dX1 ^= dX2; dX2 ^= dX1;
    dZL = dZ1; dZ1 = dZ2; dZ2 = dZL;
  }

  if (Y0 >= cam->ClipBottom) return;
  Y2 = plMin(cam->ClipBottom,Y2);

  gmem += (Y0 * cam->ScreenWidth);
  zbuf += (Y0 * cam->ScreenWidth);
  if (((dX1-dX2)*dY)>>16) dZL = ((dZ1-dZ2)*dY)/(((dX1-dX2)*dY)>>16);
  else if ((X2-X1)>>16) dZL = (Z2-Z1)/((X2-X1)>>16);
  else dZL = 0.0;

  do {
    if (Y0 == Y1) {
      dY = (sp[2]->scry>>16) - (sp[1]->scry>>16);
      if (!dY) return;
      dX1 = (sp[2]->scrx-X1)/dY;
      dX2 = (sp[2]->scrx-X2)/dY;
      dZ1 = (1/sp[2]->xformedz - Z1)/dY;
    }
    if (Y0 >= cam->ClipTop) {
      XL1 = (X1+32768)>>16;
      XL2 = (X2+32768)>>16;
      ZL = Z1;
      if (XL1 < cam->ClipLeft) {
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
        zbuf += XL1;
        gmem += XL1;
        XL1 += XL2;
        do {
          if (*zbuf < ZL) {
            *zbuf = ZL;
            *gmem = bc;
          }
          gmem++; 
          zbuf++;
          ZL += dZL;
        } while (--XL2);
        gmem -= XL1;
        zbuf -= XL1;
      }
    }
    gmem += cam->ScreenWidth;
    zbuf += cam->ScreenWidth;
    Z1 += dZ1;
    X1 += dX1;
    X2 += dX2;
  } while (++Y0 < Y2);
}

void plPF_SolidGZB(pl_CameraType *cam, pl_FaceType *TriFace) {
  unsigned char *gmem = cam->frameBuffer;
  pl_ZBufferType *zbuf = cam->zBuffer;
  pl_ZBufferType dZL, dZ1, dZ2, Z1, Z2, ZL;
  pl_VertexType *sp[3];
  pl_sInt32Type dX1, dX2, X1, X2, XL1, XL2;
  pl_sInt32Type C1, C2, dC1, dC2, dCL, CL;
  pl_sInt32Type Y1, Y2, Y0, dY;
  pl_uInt16Type *slb = cam->_ScanLineBuffer;

  pl_sInt32Type bc = TriFace->Material->_ColorBase<<16;
  pl_FloatType nc = (TriFace->Material->_ColorsUsed-1)*65536.0;

  PUTFACE_SORT();
  
  C2 = C1 = sp[0]->Shade*nc + bc;
  X2 = X1 = sp[0]->scrx;
  Z2 = Z1 = 1/sp[0]->xformedz;

  Y0 = sp[0]->scry>>16;
  Y1 = sp[1]->scry>>16;
  Y2 = sp[2]->scry>>16;

  if (Y2 == Y0) Y2++;
  dY = Y1 - Y0;
  dX1 = (sp[1]->scrx - X1) / (dY ? dY : 1);
  dX2 = (sp[2]->scrx - X1) / (Y2-Y0);
  dC1 = (sp[1]->Shade*nc + bc - C1) / (dY ? dY : 1);
  dC2 = (sp[2]->Shade*nc + bc - C1) / (Y2-Y0);
  dZ1 = ((1/sp[1]->xformedz) - Z1) / (dY ? dY : 1);
  dZ2 = ((1/sp[2]->xformedz) - Z2) / (Y2-Y0);

  if (!dY) {
    if (sp[1]->scrx > X1) {
      X2 = sp[1]->scrx;
      C2 = sp[1]->Shade*nc + bc;
      Z2 = 1/sp[1]->xformedz;
    } else {
      X1 = sp[1]->scrx;
      C1 = sp[1]->Shade*nc + bc;
      Z1 = 1/sp[1]->xformedz;
    }
  } else if (dX2 < dX1) {
    dX2 ^= dX1; dX1 ^= dX2; dX2 ^= dX1;
    dC2 ^= dC1; dC1 ^= dC2; dC2 ^= dC1;
    dZL = dZ1; dZ1 = dZ2; dZ2 = dZL;
  }

  if (Y0 >= cam->ClipBottom) return;
  Y2 = plMin(cam->ClipBottom,Y2);

  gmem += (Y0 * cam->ScreenWidth);
  zbuf += (Y0 * cam->ScreenWidth);

  if (((dX1-dX2)*dY)>>16) {
    dCL = ((dC1-dC2)*dY)/(((dX1-dX2)*dY)>>16);
    dZL = ((dZ1-dZ2)*dY)/(((dX1-dX2)*dY)>>16);
  } else if ((X2-X1)>>16) {
    dCL = (C2-C1)/(((X2-X1)>>16)+2);
    dZL = (Z2-Z1)/((X2-X1)>>16);
  } else { dCL = 0; dZL = 0; }

  do {
    if (Y0 == Y1) {
      dY = (sp[2]->scry>>16) - (sp[1]->scry>>16);
      if (!dY) return;
      dZ1 = (1/sp[2]->xformedz-Z1)/dY;
      dC1 = (sp[2]->Shade*nc + bc - C1) / dY;
      dX1 = (sp[2]->scrx - X1) / dY;
      dX2 = (sp[2]->scrx - X2) / dY;
    }
    if (Y0 >= cam->ClipTop) {
      CL = C1;
      XL1 = (X1+32768)>>16;
      XL2 = (X2+32768)>>16;
      ZL = Z1;
      if (XL1 < cam->ClipLeft) {
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
        gmem+=XL1; 
        zbuf += XL1;
        XL1 += XL2;
        do {
          if (*zbuf < ZL) {
            *zbuf = ZL;
            *gmem = CL>>16;
          }
          gmem++;
          zbuf++;
          ZL += dZL;
          CL += dCL;
        } while (--XL2);
        gmem -= XL1;
        zbuf -= XL1;
      }
    }
    gmem += cam->ScreenWidth;
    zbuf += cam->ScreenWidth;
    X1 += dX1;
    X2 += dX2; 
    C1 += dC1;
    Z1 += dZ1;
  } while (++Y0 < Y2);
}
