/******************************************************************************
**                         Plush Version 1.0                                 **
**                                                                           **
**                       Translucent Rasterizers                             **
**                                                                           **
**             All code copyright (c) 1996-1997, Justin Frankel              **
******************************************************************************/

#include "plush.h"
#include "putface.h"

void plPF_TransF(pl_CameraType *cam, pl_FaceType *TriFace) {
  pl_VertexType *sp[3];
  unsigned char *gmem = cam->frameBuffer;
  pl_sInt32Type X1, X2, dX1, dX2, XL1, XL2;
  pl_sInt32Type Y1, Y2, Y0, dY;
  pl_uInt16Type *slb = cam->_ScanLineBuffer;
  unsigned char *lookuptable = TriFace->Material->_AddTable;
  unsigned char bc = TriFace->Shade*TriFace->Material->_tsfact;

  PUTFACE_SORT();

  X2 = X1 = sp[0]->scrx;
  Y0 = sp[0]->scry>>16;
  Y1 = sp[1]->scry>>16;
  Y2 = sp[2]->scry>>16;
  if (Y2 == Y0) Y2++;
  dY = Y1-Y0;
  dX1 = (sp[1]->scrx - X1) / (dY ? dY : 1);
  dX2 = (sp[2]->scrx - X1) / (Y2-Y0);
  if (!dY) {
    if (sp[1]->scrx > X1) X2 = sp[1]->scrx;
    else X1 = sp[1]->scrx;
  } else if (dX2 < dX1) {
    dX2 ^= dX1; dX1 ^= dX2; dX2 ^= dX1;
  }

  if (Y0 >= cam->ClipBottom) return;
  Y2 = plMin(cam->ClipBottom,Y2);

  gmem += (Y0 * cam->ScreenWidth);

  do {
    if (Y0 == Y1) {
      dY = (sp[2]->scry>>16) - (sp[1]->scry>>16);
      if (!dY) return;
      dX1 = (sp[2]->scrx-X1)/dY;
      dX2 = (sp[2]->scrx-X2)/dY;
    }
    if (Y0 >= cam->ClipTop) {
      XL1 = (X1+32768)>>16;
      XL2 = (X2+32768)>>16;
      XL1 = plMax(cam->ClipLeft, XL1);
      XL2 = plMin(cam->ClipRight, XL2);
      if ((XL2-XL1) > 0) {
        if (slb) {
          if (XL1 < *(slb + (Y0<<2))) *(slb + (Y0<<2)) = XL1;
          if (XL2 > *(slb + (Y0<<2) + 1)) *(slb + (Y0<<2) + 1) = XL2;
        }
        XL2 -= XL1;
        gmem += XL1;
        XL1 += XL2;
        do {
          *gmem = bc + lookuptable[*gmem];
          gmem++;
        } while (--XL2);
        gmem -= XL1;
      }
    }
    gmem += cam->ScreenWidth;
    X1 += dX1;
    X2 += dX2;
  } while (++Y0 < Y2);
}

void plPF_TransG(pl_CameraType *cam, pl_FaceType *TriFace) {
  pl_VertexType *sp[3];
  unsigned char *gmem = cam->frameBuffer;
  pl_sInt32Type X1, X2, dX1, dX2, XL1, XL2;
  pl_sInt32Type dC1, dCL, CL, C1, C2, dC2;
  pl_sInt32Type Y1, Y2, Y0, dY;
  pl_uInt16Type *slb = cam->_ScanLineBuffer;
  pl_FloatType nc = TriFace->Material->_tsfact*65535.0;
  unsigned char *lookuptable = TriFace->Material->_AddTable;

  PUTFACE_SORT();

  C1 = C2 = sp[0]->Shade*nc;
  X2 = X1 = sp[0]->scrx;
  Y0 = sp[0]->scry>>16;
  Y1 = sp[1]->scry>>16;
  Y2 = sp[2]->scry>>16;
  if (Y2 == Y0) Y2++;
  dY = Y1-Y0;
  dX1 = (sp[1]->scrx - X1) / (dY ? dY : 1);
  dX2 = (sp[2]->scrx - X1) / (Y2-Y0);
  dC1 = (sp[1]->Shade*nc - C1) / (dY ? dY : 1);
  dC2 = (sp[2]->Shade*nc - C1) / (Y2-Y0);
  if (!dY) {
    if (sp[1]->scrx > X1) {
      X2 = sp[1]->scrx;
      C2 = sp[1]->Shade*nc;
    } else {
      X1 = sp[1]->scrx;
      C1 = sp[1]->Shade*nc;
    }
  } else if (dX2 < dX1) {
    dX2 ^= dX1; dX1 ^= dX2; dX2 ^= dX1;
    dC2 ^= dC1; dC1 ^= dC2; dC2 ^= dC1;
  }

  if (Y0 >= cam->ClipBottom) return;
  Y2 = plMin(cam->ClipBottom,Y2);

  gmem += (Y0 * cam->ScreenWidth);
  if (((dX1-dX2)*dY)>>16) dCL = ((dC1-dC2)*dY)/(((dX1-dX2)*dY)>>16);
  else if ((X2-X1)>>16) dCL = (C2-C1)/(((X2-X1)>>16)+2);
  else dCL = 0;

  do {
    if (Y0 == Y1) {
      dY = (sp[2]->scry>>16) - (sp[1]->scry>>16);
      if (!dY) return;
      dC1 = (sp[2]->Shade*nc - C1) / dY;
      dX1 = (sp[2]->scrx-X1)/dY;
      dX2 = (sp[2]->scrx-X2)/dY;
    }
    if (Y0 >= cam->ClipTop) {
      CL = C1;
      XL1 = (X1+32768)>>16;
      XL2 = (X2+32768)>>16;
      if (XL1 < cam->ClipLeft) {
        CL += dCL*(cam->ClipLeft-XL1);
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
        XL1 += XL2;
        do {
          *gmem = (CL>>16) + lookuptable[*gmem];
          CL += dCL;
          gmem++;
        } while (--XL2);
        gmem -= XL1;
      }
    }
    gmem += cam->ScreenWidth;
    X1 += dX1;
    X2 += dX2;
    C1 += dC1;
  } while (++Y0 < Y2);
}
