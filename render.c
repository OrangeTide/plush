/******************************************************************************
**                         Plush Version 1.0                                 **
**                                                                           **
**                       The Render Manager Code                             **
**                                                                           **
**             All code copyright (c) 1996-1997, Justin Frankel              **
******************************************************************************/

#include <math.h>
#include <stdlib.h>

#include "plush.h"

typedef struct {
  pl_FloatType zd;
  pl_FaceType *face;
} _faceInfoType;

static pl_uInt32Type _numfaces;
static _faceInfoType *_faces;
static pl_FloatType _cMatrix[16];
static pl_FloatType *_sl;
static pl_FloatType *_l;
static pl_sInt32Type _cx, _cy; 
static pl_FloatType _fov;
static pl_FloatType _swbig, _shbig;
static pl_FloatType _adj_asp;
static pl_LightType **_lights;
static pl_uInt16Type _numlights;
static pl_CameraType *_cam;
static int _plSortFunc(const void *, const void *);
static int _plRevSortFunc(const void *, const void *);
static char _plFindClipStat(pl_sInt32Type, pl_sInt32Type);
static void _plRenderObject(pl_ObjectType *, pl_FloatType *, pl_FloatType *);

void plRenderBegin(pl_CameraType *Camera) {
  pl_FloatType tempMatrix[16];
  _cam = Camera;
  _numlights = 0;
  _numfaces = 0;
  plMatrixRotate(_cMatrix,2,-Camera->Ya);
  plMatrixRotate(tempMatrix,1,-Camera->Xa);
  plMatrixMultiply(_cMatrix,tempMatrix);
  plMatrixRotate(tempMatrix,3,-Camera->Za);
  plMatrixMultiply(_cMatrix,tempMatrix);
  _cx = Camera->CenterX<<16;
  _cy = Camera->CenterY<<16;
  _shbig = Camera->ScreenHeight * 20.0;
  _swbig = Camera->ScreenWidth * 20.0;
  _fov = plMin(plMax(Camera->Fov,1),179);
  _fov = (1.0/tan(_fov*(PL_PI/360.0)))*(Camera->ClipRight-Camera->ClipLeft);
  _adj_asp = 1.0 / Camera->AspectRatio;
}

void plRenderLight(pl_LightType *light) {
  if (light->Type == PL_LIGHT_NONE) return;
  _numlights ++;
  _l = (pl_FloatType *) realloc(_l,_numlights*sizeof(pl_FloatType)*3);
  _sl = (pl_FloatType *) realloc(_sl,_numlights*sizeof(pl_FloatType)*3);
  _lights = (pl_LightType **) 
             realloc(_lights, _numlights*sizeof(pl_LightType *));
  if (light->Type == PL_LIGHT_VECTOR)
    plMatrixApply(_cMatrix,light->Xp,light->Yp,light->Zp,
      _l+(_numlights-1)*3,_l+(_numlights-1)*3+1,_l+(_numlights-1)*3+2);
  if (light->Type == PL_LIGHT_POINT)
    plMatrixApply(_cMatrix,light->Xp-_cam->Xp,light->Yp-_cam->Yp,
                  light->Zp-_cam->Zp,_sl+(_numlights-1)*3,
                  _sl+(_numlights-1)*3+1,_sl+(_numlights-1)*3+2);
  _lights[_numlights-1] = light;
}

static void _plRenderObject(pl_ObjectType *obj, pl_FloatType *basematrix, 
                            pl_FloatType *bnmatrix) {
  pl_uInt32Type i, x;
  pl_FloatType nx = 0.0, ny = 0.0, nz = 0.0;
  pl_FloatType tmp, tmp2;
  pl_FloatType oMatrix[16], nMatrix[16], tempMatrix[16];

  pl_VertexType *vertex;
  pl_FaceType *face;
  pl_LightType *light;

  plMatrixRotate(nMatrix,1,obj->Xa);
  plMatrixRotate(tempMatrix,2,obj->Ya);
  plMatrixMultiply(nMatrix,tempMatrix);
  plMatrixRotate(tempMatrix,3,obj->Za);
  plMatrixMultiply(nMatrix,tempMatrix);

  memcpy(oMatrix,nMatrix,sizeof(pl_FloatType)<<4);
  if (bnmatrix) plMatrixMultiply(nMatrix,bnmatrix);

  plMatrixTranslate(tempMatrix, obj->Xp, obj->Yp, obj->Zp);
  plMatrixMultiply(oMatrix,tempMatrix);
  if (basematrix) plMatrixMultiply(oMatrix,basematrix);

  for (i = 0; i < PL_MAX_CHILDREN; i ++)
    if (obj->Children[i]) _plRenderObject(obj->Children[i],oMatrix,nMatrix);
  if (!obj->NumFaces || !obj->NumVertices) return;

  plMatrixTranslate(tempMatrix, -_cam->Xp, -_cam->Yp, -_cam->Zp);
  plMatrixMultiply(oMatrix,tempMatrix);
  plMatrixMultiply(oMatrix,_cMatrix);
  plMatrixMultiply(nMatrix,_cMatrix);
  
  x = obj->NumVertices;
  face = obj->Faces;
  vertex = obj->Vertices;

  do {
    plMatrixApply(oMatrix,vertex->x,vertex->y,vertex->z, &nx, &ny, &nz); 
    vertex->xformedx = nx; 
    vertex->xformedy = ny; 
    vertex->xformedz = nz;
    if (nz > _cam->ClipFront && nz < _cam->ClipBack) {
      tmp2 = _fov / nz;
      tmp = tmp2*nx;
      tmp2 *= ny;
      if (tmp < _swbig && tmp > -_swbig &&
          tmp2 < _shbig && tmp2 > -_shbig) {
        vertex->scrx = _cx + ((pl_sInt32Type)(tmp*65536.0));
        vertex->scry = _cy - ((pl_sInt32Type)(tmp2*_adj_asp*65536.0));
      } else vertex->scrx = 1<<30;
    } else vertex->scrx = 1<<30;
    if (_cam->EnvEnabled || _cam->GouraudEnabled)
      plMatrixApply(nMatrix,vertex->nx,vertex->ny,vertex->nz, &nx,&ny,&nz);
    if (_cam->EnvEnabled) {
      vertex->eMappingU = 32768 + (nx*32768.0);
      vertex->eMappingV = 32768 + (ny*32768.0);
    } 
    if (_cam->GouraudEnabled) {
      tmp = vertex->sLighting;
      for (i = 0; i < _numlights ; i++) {
        light = _lights[i];
        if (light->Type == PL_LIGHT_POINT) {
          _l[i*3] = _sl[i*3] - vertex->xformedx; 
          _l[i*3+1] = _sl[i*3+1] - vertex->xformedy; 
          _l[i*3+2] = _sl[i*3+2] - vertex->xformedz;
          plNormalizeVector(_l+i*3,_l+i*3+1,_l+i*3+2);
        }
        tmp2 = plDotProduct(nx,ny,nz,_l[i*3],_l[i*3+1],_l[i*3+2])
               * light->Intensity;
        if (tmp2 > 0.0) tmp += tmp2;
        else if (obj->BackfaceIllumination) tmp -= tmp2;
      }
      vertex->Shade = plMin(tmp,1.0);
    }
    vertex++;
  } while (--x);
  x = obj->NumFaces;
  _faces = 
    (_faceInfoType *) realloc(_faces,(_numfaces+x)*sizeof(_faceInfoType));
  do {
    if (obj->BackfaceCull || face->Material->_st & PL_SHADE_FLAT)
      plMatrixApply(nMatrix,face->nx,face->ny,face->nz,&nx,&ny,&nz);
    if (!(face->Vertices[0]->scrx == 1<<30 ||
          face->Vertices[1]->scrx == 1<<30 ||
          face->Vertices[2]->scrx == 1<<30)) {
      char c1,c2,c3;
      c1 = _plFindClipStat(face->Vertices[0]->scrx,face->Vertices[0]->scry);
      c2 = _plFindClipStat(face->Vertices[1]->scrx,face->Vertices[1]->scry);
      c3 = _plFindClipStat(face->Vertices[2]->scrx,face->Vertices[2]->scry);
      if (!(c1|c2|c3) || !(c1&c2&c3))
        if (!obj->BackfaceCull || 
            (plDotProduct(nx,ny,nz, 
                  face->Vertices[0]->xformedx, face->Vertices[0]->xformedy,
                  face->Vertices[0]->xformedz) < -0.001)) {
        tmp = face->sLighting = 0.0;
        if (face->Material->_st & PL_SHADE_FLAT) {
          for (i = 0; i < _numlights; i ++) {
            light = _lights[i];
            if (light->Type == PL_LIGHT_POINT) {
              _l[i*3+0] = _sl[i*3+0] - face->Vertices[0]->xformedx; 
              _l[i*3+1] = _sl[i*3+1] - face->Vertices[0]->xformedy; 
              _l[i*3+2] = _sl[i*3+2] - face->Vertices[0]->xformedz;
              plNormalizeVector(_l+i*3,_l+i*3+1,_l+i*3+2);
            }
            tmp2= plDotProduct(nx,ny,nz,_l[i*3],_l[i*3+1],_l[i*3+2])
                  * light->Intensity;
            if (tmp2 > 0.0) tmp += tmp2;
            else if (obj->BackfaceIllumination) tmp -= tmp2;
          }
        }
        face->Shade = plMin(tmp,1.0);
        _faces[_numfaces].zd = face->Vertices[0]->xformedz+
                               face->Vertices[1]->xformedz+
                               face->Vertices[2]->xformedz;
        _faces[_numfaces].face = face;
        _numfaces++;
      } 
    }
    face++;
  } while (--x);
}

void plRenderObject(pl_ObjectType *obj) {
  _plRenderObject(obj,0,0);
}

void plRenderEnd() {
  _faceInfoType *f;
  if (_cam->Sort > 0) 
    qsort(_faces,_numfaces,sizeof(_faceInfoType),_plSortFunc); 
  else if (_cam->Sort < 0) 
    qsort(_faces,_numfaces,sizeof(_faceInfoType),_plRevSortFunc); 
  f = _faces;
  while (_numfaces--) { plPutFace(_cam,f->face); f++; }
  free(_faces); free(_sl); free(_l); free(_lights);
  _sl = _l = 0;
  _faces = 0;
  _lights = 0;
  _numlights = 0;
}

static char _plFindClipStat(pl_sInt32Type x, pl_sInt32Type y) {
  char c =0;
  x >>= 16;
  y >>= 16;
  if (x < _cam->ClipLeft) c |= 1;
  if (x > _cam->ClipRight) c |= 2;
  if (y > _cam->ClipBottom) c |= 4;
  if (y < _cam->ClipTop) c |= 8;
  return c;
}

static int _plSortFunc(const void *a, const void *b) {
  return ((((_faceInfoType *)b)->zd > ((_faceInfoType *)a)->zd) ? 1 : -1);
}

static int _plRevSortFunc(const void *a, const void *b) {
  return ((((_faceInfoType *)b)->zd < ((_faceInfoType *)a)->zd) ? 1 : -1);
}
