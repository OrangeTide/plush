/******************************************************************************
**                         Plush Version 1.0                                 **
**                                                                           **
**                         3DS Object Reader                                 **
**                                                                           **
**             All code copyright (c) 1996-1997, Justin Frankel              **
******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "plush.h"

typedef struct {
    pl_uInt16Type id;
    void (*func)(FILE *f, pl_uInt32Type p);
} _pl_3DSChunkType;

static pl_ObjectType *obj;
static pl_ObjectType *bobj;
static pl_ObjectType *lobj;
static pl_sInt16Type currentobj;
static pl_MaterialType *_m;

static pl_FloatType _pl3DSReadFloat(FILE *f);
static pl_uInt32Type _pl3DSReadDWord(FILE *f);
static pl_uInt16Type _pl3DSReadWord(FILE *f);
static void _pl3DSChunkReader(FILE *f, pl_uInt32Type p);
static void _pl3DSRGBFReader(FILE *f, pl_uInt32Type p);
static void _pl3DSRGBBReader(FILE *f, pl_uInt32Type p);
static void _pl3DSASCIIZReader(FILE *f, pl_uInt32Type p, char *as);
static void _pl3DSObjBlockReader(FILE *f, pl_uInt32Type p);
static void _pl3DSTriMeshReader(FILE *f, pl_uInt32Type p);
static void _pl3DSVertListReader(FILE *f, pl_uInt32Type p);
static void _pl3DSFaceListReader(FILE *f, pl_uInt32Type p);
static void _pl3DSFaceMatReader(FILE *f, pl_uInt32Type p);
static void MapListReader(FILE *f, pl_uInt32Type p);
static pl_sInt16Type _pl3DSFindChunk(pl_uInt16Type id);

static _pl_3DSChunkType _pl3DSChunkNames[] = {
    {0x4D4D,NULL}, /* Main */
    {0x3D3D,NULL}, /* Object Mesh */
    {0x4000,_pl3DSObjBlockReader},
    {0x4100,_pl3DSTriMeshReader},
    {0x4110,_pl3DSVertListReader},
    {0x4120,_pl3DSFaceListReader},
    {0x4130,_pl3DSFaceMatReader},
    {0x4140,MapListReader},
    {0xAFFF,NULL}, /* Material */
    {0xA010,NULL}, /* Ambient */
    {0xA020,NULL}, /* Diff */
    {0xA030,NULL}, /* Specular */
    {0xA200,NULL}, /* Texture */
    {0x0010,_pl3DSRGBFReader},
    {0x0011,_pl3DSRGBBReader},
};

pl_ObjectType *plReadObject3DS(char *fn, pl_MaterialType *m) {
  FILE *f;
  pl_uInt32Type p;
  _m = m;
  obj = bobj = lobj = 0;
  currentobj = 0;
  f = fopen(fn, "rb");
  if (!f) return 0;
  fseek(f, 0, 2);
  p = ftell(f);
  rewind(f);
  _pl3DSChunkReader(f, p);
  fclose(f);
  return bobj;
}

static pl_FloatType _pl3DSReadFloat(FILE *f) {
  pl_IEEEFloat32Type c;
  fread(&c,sizeof(c),1,f);
  return ((pl_FloatType) c);
}

static pl_uInt32Type _pl3DSReadDWord(FILE *f) {
  pl_uInt32Type r;
  r = fgetc(f);
  r |= fgetc(f)<<8;
  r |= fgetc(f)<<16;
  r |= fgetc(f)<<24;
  return r;
}

static pl_uInt16Type _pl3DSReadWord(FILE *f) {
  pl_uInt16Type r;
  r = fgetc(f);
  r |= fgetc(f)<<8;
  return r;
}

static void _pl3DSRGBFReader(FILE *f, pl_uInt32Type p) {
  pl_FloatType c[3];
  c[0] = _pl3DSReadFloat(f);
  c[1] = _pl3DSReadFloat(f);
  c[2] = _pl3DSReadFloat(f);
}

static void _pl3DSRGBBReader(FILE *f, pl_uInt32Type p) {
  unsigned char c[3];
  if (fread(&c, sizeof(c), 1, f) != 1) return;
}

static void _pl3DSASCIIZReader(FILE *f, pl_uInt32Type p, char *as) {
  char c;
  if (!as) while ((c = fgetc(f)) != EOF && c != '\0');
  else { 
    while ((c = fgetc(f)) != EOF && c != '\0') *as++ = c;
    *as = 0;
  }
}

static void _pl3DSObjBlockReader(FILE *f, pl_uInt32Type p) {
  _pl3DSASCIIZReader(f,p,0);
  _pl3DSChunkReader(f, p);
}

static void _pl3DSTriMeshReader(FILE *f, pl_uInt32Type p) {
  pl_uInt32Type i; 
  pl_FaceType *face;
  obj = plAllocObject(0,0);
  _pl3DSChunkReader(f, p);
  i = obj->NumFaces;
  face = obj->Faces;
  while (i--) {
    face->Vertices[0] = obj->Vertices + (pl_uInt32Type) face->Vertices[0];
    face->Vertices[1] = obj->Vertices + (pl_uInt32Type) face->Vertices[1];
    face->Vertices[2] = obj->Vertices + (pl_uInt32Type) face->Vertices[2];
    face->MappingU[0] = face->Vertices[0]->eMappingU;
    face->MappingV[0] = face->Vertices[0]->eMappingV;
    face->MappingU[1] = face->Vertices[1]->eMappingU;
    face->MappingV[1] = face->Vertices[1]->eMappingV;
    face->MappingU[2] = face->Vertices[2]->eMappingU;
    face->MappingV[2] = face->Vertices[2]->eMappingV;
    face++;
  }
  plCalcNormals(obj);
  if (currentobj == 0) {
    currentobj = 1;
    lobj = bobj = obj;
  } else {
    lobj->Children[0] = obj;
    lobj = obj;
  }
}

static void _pl3DSVertListReader(FILE *f, pl_uInt32Type p) {
  pl_uInt16Type nv;
  pl_VertexType *v;
  nv = _pl3DSReadWord(f);
  obj->NumVertices = nv;
  v = obj->Vertices = (pl_VertexType *) calloc(sizeof(pl_VertexType)*nv,1);
  while (nv--) {
    v->x = _pl3DSReadFloat(f);
    v->y = _pl3DSReadFloat(f);
    v->z = _pl3DSReadFloat(f);
    if (feof(f)) return;
    v++;
  }
}

static void _pl3DSFaceListReader(FILE *f, pl_uInt32Type p) {
  pl_uInt16Type nv;
  pl_uInt16Type c[3];
  pl_uInt16Type flags;
  pl_FaceType *face;

  nv = _pl3DSReadWord(f);
  obj->NumFaces = nv;
  face = obj->Faces = (pl_FaceType *) calloc(sizeof(pl_FaceType)*nv,1);
  while (nv--) {
    c[0] = _pl3DSReadWord(f);
    c[1] = _pl3DSReadWord(f);
    c[2] = _pl3DSReadWord(f);
    flags = _pl3DSReadWord(f);
    if (feof(f)) return;
    face->Vertices[0] = (pl_VertexType *) (c[0]&0x0000FFFF);
    face->Vertices[1] = (pl_VertexType *) (c[1]&0x0000FFFF);
    face->Vertices[2] = (pl_VertexType *) (c[2]&0x0000FFFF);
    face->Material = _m;
    face++;
  }
  _pl3DSChunkReader(f, p);
}

static void _pl3DSFaceMatReader(FILE *f, pl_uInt32Type p) {
  pl_uInt16Type n, nf;

  _pl3DSASCIIZReader(f, p,0);

  n = _pl3DSReadWord(f);
  while (n--) {
    nf = _pl3DSReadWord(f);
  }
}

static void MapListReader(FILE *f, pl_uInt32Type p) {
  pl_uInt16Type nv;
  pl_FloatType c[2];
  pl_VertexType *v;
  nv = _pl3DSReadWord(f);
  v = obj->Vertices;
  if (nv == obj->NumVertices) while (nv--) {
    c[0] = _pl3DSReadFloat(f);
    c[1] = _pl3DSReadFloat(f);
    if (feof(f)) return;
    v->eMappingU = (pl_sInt32Type) (c[0]*65536.0);
    v->eMappingV = (pl_sInt32Type) (c[1]*65536.0);
    v++;
  }
}

static pl_sInt16Type _pl3DSFindChunk(pl_uInt16Type id) {
  pl_sInt16Type i;
  for (i = 0; i < sizeof(_pl3DSChunkNames)/sizeof(_pl3DSChunkNames[0]); i++)
    if (id == _pl3DSChunkNames[i].id) return i;
  return -1;
}

static void _pl3DSChunkReader(FILE *f, pl_uInt32Type p) {
  pl_uInt32Type hlen;
  pl_uInt16Type hid;
  pl_sInt16Type n;
  pl_uInt32Type pc;

  while (ftell(f) < p) {
    pc = ftell(f);
    hid = _pl3DSReadWord(f); if (feof(f)) return;
    hlen = _pl3DSReadDWord(f); if (feof(f)) return;
    if (hlen == 0) return;
    n = _pl3DSFindChunk(hid);
    if (n < 0) fseek(f, pc + hlen, 0);
    else {
      pc += hlen;
      if (_pl3DSChunkNames[n].func != NULL) _pl3DSChunkNames[n].func(f, pc);
      else _pl3DSChunkReader(f, pc);
      fseek(f, pc, 0);
    }
    if (ferror(f)) break;
  }
}

