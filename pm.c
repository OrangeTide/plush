/******************************************************************************
**                         Plush Version 1.0                                 **
**                                                                           **
**                      The Palette Manager Code                             **
**                                                                           **
**             All code copyright (c) 1996-1997, Justin Frankel              **
******************************************************************************/

#include <stdlib.h>
#include "plush.h"

static pl_MaterialType **_Materials;
static pl_sInt16Type _numMaterials;
static unsigned char _begin, _len;
static unsigned char *_pal;

void plPMBegin(unsigned char *palette, unsigned char begin, unsigned char end) {
  _begin = begin;
  _len = end - begin;
  _pal = palette;
  _Materials = 0;
  _numMaterials = 0;
}

void plPMAddMaterial(pl_MaterialType *m) {
  _numMaterials++;
  _Materials = (pl_MaterialType **) 
               realloc(_Materials, _numMaterials*sizeof(pl_MaterialType *));
  _Materials[_numMaterials-1] = m;
}

void plPMEnd() {
  unsigned char *nc = (unsigned char *) malloc(_numMaterials);
  pl_sInt32Type x, total, highest;
  total = 0;
  for (x = 0; x < _numMaterials; x ++) {
    nc[x] = _Materials[x]->NumGradients;
    plInitializeMaterial(_pal,_Materials[x],0);
    total += _Materials[x]->_ColorsUsed;
  }

  while (total > _len) {
    highest = 0;
    for (x = 0; x < _numMaterials; x ++)
      if ((_Materials[x]->Priority+1)*_Materials[x]->NumGradients >=
          (_Materials[highest]->Priority+1)*_Materials[highest]->NumGradients)
            highest = x;
    if (_Materials[highest]->NumGradients) _Materials[highest]->NumGradients--;
    total -= _Materials[highest]->_ColorsUsed;
    plInitializeMaterial(_pal,_Materials[highest],0);
    total += _Materials[highest]->_ColorsUsed;
  }

  for (x = 0; x < _numMaterials; x ++) {
    _Materials[x]->_ColorBase = _begin;
    plInitializeMaterial(_pal,_Materials[x],1);
    _begin += _Materials[x]->_ColorsUsed;
  }
  for (x = 0; x < _numMaterials; x ++) {
    if (_Materials[x]->Transparent) 
      plInitializeMaterial(_pal,_Materials[x],1);
    _Materials[x]->NumGradients = nc[x];
  }
  _numMaterials = 0;
  free(_Materials);
  free(nc);
  _Materials = 0;
}
