/******************************************************************************
**                         Plush Version 1.0                                 **
**                                                                           **
**                      Math and Matrix Functions                            **
**                                                                           **
**             All code copyright (c) 1996-1997, Justin Frankel              **
******************************************************************************/

#include <string.h>
#include <math.h>
#include "plush.h"

static void _plCalcFaceNormal(pl_FaceType *);

void plMatrixRotate(pl_FloatType *matrix, unsigned char m, pl_FloatType Deg) {
  unsigned char m1, m2;
  pl_FloatType c,s;
  Deg *= PL_PI / 180.0;
  memset(matrix,0,sizeof(pl_FloatType)<<4);
  matrix[((m-1)<<2)+m-1] = matrix[15] = 1.0;
  m1 = (m % 3)+1;
  m2 = (m1 % 3);
  m1--;
  c = cos(Deg); s = sin(Deg);
  matrix[(m1<<2)+m1]=c; matrix[(m1<<2)+m2]=s;
  matrix[(m2<<2)+m2]=c; matrix[(m2<<2)+m1]=-s;
}

void plMatrixTranslate(pl_FloatType *m, pl_FloatType x, pl_FloatType y, 
                       pl_FloatType z) {
  memset(m,0,sizeof(pl_FloatType)<<4);
  m[0] = m[4+1] = m[8+2] = m[12+3] = 1.0;
  m[0+3] = x; m[4+3] = y; m[8+3] = z;
}

void plMatrixMultiply(pl_FloatType *dest, pl_FloatType *src) {
  pl_FloatType temp[16];
  unsigned char i;
  memcpy(temp,dest,sizeof(pl_FloatType)<<4);
  for (i = 0; i < 16; i += 4) {
    *dest++ = src[i+0]*temp[(0<<2)+0]+src[i+1]*temp[(1<<2)+0]+
              src[i+2]*temp[(2<<2)+0]+src[i+3]*temp[(3<<2)+0];
    *dest++ = src[i+0]*temp[(0<<2)+1]+src[i+1]*temp[(1<<2)+1]+
              src[i+2]*temp[(2<<2)+1]+src[i+3]*temp[(3<<2)+1];
    *dest++ = src[i+0]*temp[(0<<2)+2]+src[i+1]*temp[(1<<2)+2]+
              src[i+2]*temp[(2<<2)+2]+src[i+3]*temp[(3<<2)+2];
    *dest++ = src[i+0]*temp[(0<<2)+3]+src[i+1]*temp[(1<<2)+3]+
              src[i+2]*temp[(2<<2)+3]+src[i+3]*temp[(3<<2)+3];
  }
}

void plMatrixApply(pl_FloatType *m, pl_FloatType x, pl_FloatType y, 
                   pl_FloatType z, pl_FloatType *outx, pl_FloatType *outy, 
                   pl_FloatType *outz) {
  *outx = x*m[0] + y*m[1] + z*m[2] + m[3];
  *outy	= x*m[4] + y*m[5] + z*m[6] + m[7];
  *outz = x*m[8] + y*m[9] + z*m[10] + m[11];
}

static void _plCalcFaceNormal(pl_FaceType *face) {
  pl_FloatType x1, x2, y1, y2, z1, z2;
  x1 = face->Vertices[0]->x-face->Vertices[1]->x;
  x2 = face->Vertices[0]->x-face->Vertices[2]->x;
  y1 = face->Vertices[0]->y-face->Vertices[1]->y;
  y2 = face->Vertices[0]->y-face->Vertices[2]->y;
  z1 = face->Vertices[0]->z-face->Vertices[1]->z;
  z2 = face->Vertices[0]->z-face->Vertices[2]->z;
  face->nx = y1*z2 - z1*y2;
  face->ny = z1*x2 - x1*z2;
  face->nz = x1*y2 - y1*x2;
  plNormalizeVector(&(face->nx), &(face->ny), &(face->nz));
}

void plCalcNormals(pl_ObjectType *obj) {
  pl_uInt32Type i;
  pl_VertexType *v = obj->Vertices;
  pl_FaceType *f = obj->Faces;
  i = obj->NumVertices;
  do {
    v->nx = 0.0; v->ny = 0.0; v->nz = 0.0;
    v++;
  } while (--i);
  i = obj->NumFaces;
  do {
    _plCalcFaceNormal(f);
    f->Vertices[0]->nx += f->nx;
    f->Vertices[0]->ny += f->ny;
    f->Vertices[0]->nz += f->nz;
    f->Vertices[1]->nx += f->nx;
    f->Vertices[1]->ny += f->ny;
    f->Vertices[1]->nz += f->nz;
    f->Vertices[2]->nx += f->nx;
    f->Vertices[2]->ny += f->ny;
    f->Vertices[2]->nz += f->nz;
    f++;
  } while (--i);
  v = obj->Vertices;
  i = obj->NumVertices;
  do {
    plNormalizeVector(&v->nx, &v->ny, &v->nz);
    v++;
  } while (--i);
  for (i = 0; i < PL_MAX_CHILDREN; i ++) 
    if (obj->Children[i]) plCalcNormals(obj->Children[i]);
}

void plNormalizeVector(pl_FloatType *x, pl_FloatType *y, pl_FloatType *z) {
  pl_FloatType length;
  length = (*x)*(*x)+(*y)*(*y)+(*z)*(*z);
  if (length > 0.0000000001) {
    length = sqrt(length);
    *x /= length;
    *y /= length;
    *z /= length;
  } else *x = *y = *z = 0;
}

pl_FloatType plDotProduct(pl_FloatType x1, pl_FloatType y1, pl_FloatType z1,
                          pl_FloatType x2, pl_FloatType y2, pl_FloatType z2) {
  return ((x1*x2)+(y1*y2)+(z1*z2));
}
