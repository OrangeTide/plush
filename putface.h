/******************************************************************************
**                         Plush Version 1.0                                 **
**                                                                           **
**                Triangle Vertex Sorting Code for ?f_*.c                    **
**                                                                           **
**             All code copyright (c) 1996-1997, Justin Frankel              **
******************************************************************************/

#define PUTFACE_SORT_ENV() \
  pl_VertexType *v; \
  sp[0] = TriFace->Vertices[0]; \
  sp[1] = TriFace->Vertices[1]; \
  sp[2] = TriFace->Vertices[2]; \
  if (sp[0]->scry > sp[1]->scry) { \
     v = sp[0]; sp[0] = sp[1]; sp[1] = v; \
  } \
  if (sp[0]->scry > sp[2]->scry) { \
     v = sp[0]; sp[0] = sp[2]; sp[2] = v; \
  } \
  if (sp[1]->scry > sp[2]->scry) { \
     v = sp[1]; sp[1] = sp[2]; sp[2] = v; \
  } \
  MappingU1=(sp[0]->eMappingU<<Texture->Width)*TriFace->Material->EnvScaling;\
  MappingV1=(sp[0]->eMappingV<<Texture->Height)*TriFace->Material->EnvScaling;\
  MappingU2=(sp[1]->eMappingU<<Texture->Width)*TriFace->Material->EnvScaling;\
  MappingV2=(sp[1]->eMappingV<<Texture->Height)*TriFace->Material->EnvScaling;\
  MappingU3=(sp[2]->eMappingU<<Texture->Width)*TriFace->Material->EnvScaling;\
  MappingV3=(sp[2]->eMappingV<<Texture->Height)*TriFace->Material->EnvScaling;

#define PUTFACE_SORT_TEX() \
  pl_VertexType *v; \
  unsigned char i0, i1, i2; \
  sp[0] = TriFace->Vertices[0]; \
  sp[1] = TriFace->Vertices[1]; \
  sp[2] = TriFace->Vertices[2]; \
  i0 = 0; i1 = 1; i2 = 2; \
  if (sp[0]->scry > sp[1]->scry) { \
     i0 ^= i1; i1 ^= i0; i0 ^= i1; \
     v = sp[0]; sp[0] = sp[1]; sp[1] = v; \
  } \
  if (sp[0]->scry > sp[2]->scry) { \
     i0 ^= i2; i2 ^= i0; i0 ^= i2; \
     v = sp[0]; sp[0] = sp[2]; sp[2] = v; \
  } \
  if (sp[1]->scry > sp[2]->scry) { \
     i1 ^= i2; i2 ^= i1; i1 ^= i2; \
     v = sp[1]; sp[1] = sp[2]; sp[2] = v; \
  } \
  MappingU1=(TriFace->MappingU[i0]<<Texture->Width);\
  MappingV1=(TriFace->MappingV[i0]<<Texture->Height);\
  MappingU2=(TriFace->MappingU[i1]<<Texture->Width);\
  MappingV2=(TriFace->MappingV[i1]<<Texture->Height);\
  MappingU3=(TriFace->MappingU[i2]<<Texture->Width);\
  MappingV3=(TriFace->MappingV[i2]<<Texture->Height);

#define PUTFACE_SORT() \
  pl_VertexType *v; \
  sp[0] = TriFace->Vertices[0]; \
  sp[1] = TriFace->Vertices[1]; \
  sp[2] = TriFace->Vertices[2]; \
  if (sp[0]->scry > sp[1]->scry) { \
     v = sp[0]; sp[0] = sp[1]; sp[1] = v; \
  } \
  if (sp[0]->scry > sp[2]->scry) { \
     v = sp[0]; sp[0] = sp[2]; sp[2] = v; \
  } \
  if (sp[1]->scry > sp[2]->scry) { \
     v = sp[1]; sp[1] = sp[2]; sp[2] = v; \
  }

#define PUTFACE_SORT_TEXENV() { \
  register pl_VertexType *v; \
  unsigned char i0, i1, i2; \
  sp[0] = TriFace->Vertices[0]; \
  sp[1] = TriFace->Vertices[1]; \
  sp[2] = TriFace->Vertices[2]; \
  i0 = 0; i1 = 1; i2 = 2; \
  if (sp[0]->scry > sp[1]->scry) { \
     i0 ^= i1; i1 ^= i0; i0 ^= i1; \
     v = sp[0]; sp[0] = sp[1]; sp[1] = v; \
  } \
  if (sp[0]->scry > sp[2]->scry) { \
     i0 ^= i2; i2 ^= i0; i0 ^= i2; \
     v = sp[0]; sp[0] = sp[2]; sp[2] = v; \
  } \
  if (sp[1]->scry > sp[2]->scry) { \
     i1 ^= i2; i2 ^= i1; i1 ^= i2; \
     v = sp[1]; sp[1] = sp[2]; sp[2] = v; \
  } \
  MappingU1=(TriFace->MappingU[i0]<<Texture->Width);\
  MappingV1=(TriFace->MappingV[i0]<<Texture->Height);\
  MappingU2=(TriFace->MappingU[i1]<<Texture->Width);\
  MappingV2=(TriFace->MappingV[i1]<<Texture->Height);\
  MappingU3=(TriFace->MappingU[i2]<<Texture->Width);\
  MappingV3=(TriFace->MappingV[i2]<<Texture->Height);\
  eMappingU1=(sp[0]->eMappingU<<Environment->Width)*TriFace->Material->EnvScaling;\
  eMappingV1=(sp[0]->eMappingV<<Environment->Height)*TriFace->Material->EnvScaling;\
  eMappingU2=(sp[1]->eMappingU<<Environment->Width)*TriFace->Material->EnvScaling;\
  eMappingV2=(sp[1]->eMappingV<<Environment->Height)*TriFace->Material->EnvScaling;\
  eMappingU3=(sp[2]->eMappingU<<Environment->Width)*TriFace->Material->EnvScaling;\
  eMappingV3=(sp[2]->eMappingV<<Environment->Height)*TriFace->Material->EnvScaling;\
}
