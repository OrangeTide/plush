/******************************************************************************
**                         Plush Version 1.0                                 **
**                                                                           **
**                      Object Utility Functions                             **
**                                                                           **
**             All code copyright (c) 1996-1997, Justin Frankel              **
******************************************************************************/

#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include "plush.h"

pl_ObjectType *plScaleObject(pl_ObjectType *o, pl_FloatType s) {
  register pl_uInt32Type i = o->NumVertices;
  register pl_VertexType *v = o->Vertices;
  while (i--) {
    v->x *= s;
    v->y *= s;
    v->z *= s;
    v++;
  }
  for (i = 0; i < PL_MAX_CHILDREN; i ++) 
    if (o->Children[i]) plScaleObject(o->Children[i],s);
  return o;
}

pl_ObjectType *plStretchObject(pl_ObjectType *o, pl_FloatType x,
                                     pl_FloatType y, pl_FloatType z,
                                     char rcn) {
  register pl_uInt32Type i = o->NumVertices;
  register pl_VertexType *v = o->Vertices;
  while (i--) {
    v->x *= x;
    v->y *= y;
    v->z *= z;
    v++;
  }
  if (rcn) plCalcNormals(o);
  for (i = 0; i < PL_MAX_CHILDREN; i ++) 
    if (o->Children[i]) plStretchObject(o->Children[i],x,y,z,rcn);
  return o;
}

pl_ObjectType *plTranslateObject(pl_ObjectType *o, pl_FloatType x, 
                                       pl_FloatType y, pl_FloatType z) {
  register pl_uInt32Type i = o->NumVertices;
  register pl_VertexType *v = o->Vertices;
  while (i--) {
    v->x += x;
    v->y += y;
    v->z += z;
    v++;
  }
  for (i = 0; i < PL_MAX_CHILDREN; i ++) 
    if (o->Children[i]) plTranslateObject(o->Children[i],x,y,z);
  return o;
}

pl_ObjectType *plFlipObjectNormals(pl_ObjectType *o) {
  pl_uInt32Type i = o->NumVertices;
  pl_VertexType *v = o->Vertices;
  pl_FaceType *f = o->Faces;
  while (i--) {
    v->nx = - v->nx;
    v->ny = - v->ny;
    v->nz = - v->nz;
    v++;
  } 
  i = o->NumFaces;
  while (i--) {
    f->nx = - f->nx;
    f->ny = - f->ny;
    f->nz = - f->nz;
    v = f->Vertices[1];
    f->Vertices[1] = f->Vertices[2];
    f->Vertices[2] = v;
    f++;
  }
  for (i = 0; i < PL_MAX_CHILDREN; i ++) 
    if (o->Children[i]) plFlipObjectNormals(o->Children[i]);
  return o;
}

void plFreeObject(pl_ObjectType *o) {
  int i;
  for (i = 0; i < PL_MAX_CHILDREN; i ++) 
    if (o->Children[i]) plFreeObject(o->Children[i]);
  free(o->Vertices);
  free(o->Faces);
  free(o);
}

pl_ObjectType *plAllocObject(pl_uInt32Type nv, pl_uInt32Type nf) {
  register pl_ObjectType *o;
  if (!(o = (pl_ObjectType *) malloc(sizeof(pl_ObjectType)))) return 0;
  memset(o,0,sizeof(pl_ObjectType));
  o->BackfaceCull = 1;
  o->NumVertices = nv;
  o->NumFaces = nf;
  if (nv && !(o->Vertices=(pl_VertexType *) malloc(sizeof(pl_VertexType)*nv))) {
    free(o);
    return 0;
  }
  if (nf && !(o->Faces = (pl_FaceType *) malloc(sizeof(pl_FaceType)*nf))) {
    free(o->Vertices);
    free(o); 
    return 0;
  }
  memset(o->Vertices,0,sizeof(pl_VertexType)*nv);
  memset(o->Faces,0,sizeof(pl_FaceType)*nf);
  return o;
}

pl_ObjectType *plCloneObject(pl_ObjectType *o) {
  register pl_FaceType *iff, *of;
  register pl_uInt32Type i;
  pl_ObjectType *out;
  if (!(out = plAllocObject(o->NumVertices,o->NumFaces))) return 0;
  for (i = 0; i < PL_MAX_CHILDREN; i ++) 
    if (o->Children[i]) out->Children[i] = plCloneObject(o->Children[i]);
  out->Xa = o->Xa; out->Ya = o->Ya; out->Za = o->Za;
  out->Xp = o->Xp; out->Yp = o->Yp; out->Zp = o->Zp;
  memcpy(out->Children,o->Children,sizeof(pl_ObjectType *)*16);
  out->BackfaceCull = o->BackfaceCull;
  out->BackfaceIllumination = o->BackfaceIllumination;
  memcpy(out->Vertices, o->Vertices, sizeof(pl_VertexType) * o->NumVertices);
  iff = o->Faces;
  of = out->Faces;
  i = out->NumFaces;
  while (i--) {
    of->Vertices[0] = (pl_VertexType *) 
      out->Vertices + (iff->Vertices[0] - o->Vertices);
    of->Vertices[1] = (pl_VertexType *) 
      out->Vertices + (iff->Vertices[1] - o->Vertices);
    of->Vertices[2] = (pl_VertexType *) 
      out->Vertices + (iff->Vertices[2] - o->Vertices);
    of->MappingU[0] = iff->MappingU[0];
    of->MappingV[0] = iff->MappingV[0];
    of->MappingU[1] = iff->MappingU[1];
    of->MappingV[1] = iff->MappingV[1];
    of->MappingU[2] = iff->MappingU[2];
    of->MappingV[2] = iff->MappingV[2];
    of->nx = iff->nx;
    of->ny = iff->ny;
    of->nz = iff->nz;
    of->Material = iff->Material;
    of++;
    iff++;
  }
  return out;
}

pl_ObjectType *plScaleObjectTexture(pl_ObjectType *o, 
                                    pl_FloatType u, pl_FloatType v) {
  pl_uInt32Type i = o->NumFaces;
  pl_FaceType *f = o->Faces;
  while (i--) {
    f->MappingU[0] = f->MappingU[0] * u;
    f->MappingV[0] = f->MappingV[0] * v;
    f->MappingU[1] = f->MappingU[1] * u;
    f->MappingV[1] = f->MappingV[1] * v;
    f->MappingU[2] = f->MappingU[2] * u;
    f->MappingV[2] = f->MappingV[2] * v;
    f++;
  }
  for (i = 0; i < PL_MAX_CHILDREN; i ++) 
    if (o->Children[i]) plScaleObjectTexture(o->Children[i],u,v);
  return o;
}

void plSetObjectMaterial(pl_ObjectType *o, pl_MaterialType *m, char th) {
  pl_sInt32Type i = o->NumFaces;
  pl_FaceType *f = o->Faces;
  while (i--) (f++)->Material = m; 
  if (th) for (i = 0; i < PL_MAX_CHILDREN; i++) 
    if (o->Children[i]) plSetObjectMaterial(o->Children[i],m,th);
}
