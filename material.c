/******************************************************************************
**                         Plush Version 1.0                                 **
**                                                                           **
**                   Material Color Ramp Generation                          **
**                                                                           **
**             All code copyright (c) 1996-1997, Justin Frankel              **
******************************************************************************/

#include <math.h>
#include <malloc.h>
#include "plush.h"

static void _plGenerateSinglePalette(unsigned char *, pl_MaterialType *, char);
static void _plGeneratePhongPalette(unsigned char *, pl_MaterialType *, char);
static void _plGenerateTextureEnvPalette(unsigned char *, pl_MaterialType *, 
                                         char);
static void _plGenerateTexturePalette(unsigned char *, pl_MaterialType *,
                                      pl_TextureType *, char);
static void _plGeneratePhongTexturePalette(unsigned char *, pl_MaterialType *,
                                           pl_TextureType *, char);
static void _plGeneratePhongTransparentPalette(unsigned char *pal,
                                              pl_MaterialType *m, char g);
static void  _plGenerateTransparentPalette(unsigned char *, pl_MaterialType *,
                                           char);


void plInitializeMaterial(unsigned char *pal, pl_MaterialType *m, char g) {
  m->_ft = ((m->Environment ? PL_FILL_ENVIRONMENT : 0) |
           (m->Texture ? PL_FILL_TEXTURE : 0));
  m->_st = m->ShadeType;

  if (m->Transparent) m->_ft = PL_FILL_TRANSPARENT;

  if (m->_ft == (PL_FILL_ENVIRONMENT|PL_FILL_TEXTURE) &&
      m->Environment->NumColors*m->Texture->NumColors > m->NumGradients)  
        m->_ft = PL_FILL_TEXTURE;
  if (m->_ft == PL_FILL_TEXTURE &&
      m->Texture->NumColors > m->NumGradients) m->_ft = PL_FILL_SOLID;
  if (m->_ft == PL_FILL_ENVIRONMENT &&
      m->Environment->NumColors > m->NumGradients) m->_ft = PL_FILL_SOLID;

  if (m->_ft == PL_FILL_TEXTURE && m->Texture->NumColors*2 > m->NumGradients)
      m->_st = PL_SHADE_NONE;
  if (m->_ft == PL_FILL_ENVIRONMENT && 
      m->Environment->NumColors*2 > m->NumGradients) m->_st = PL_SHADE_NONE;
  if (m->_ft == PL_FILL_SOLID && m->NumGradients == 1) 
    m->_st = PL_SHADE_NONE;
  if (m->_ft == (PL_FILL_TEXTURE|PL_FILL_ENVIRONMENT)) 
     m->_st = PL_SHADE_NONE;

  if (m->_ft == PL_FILL_SOLID) { 
    if (m->_st == PL_SHADE_NONE) _plGenerateSinglePalette(pal,m,1);
    else _plGeneratePhongPalette(pal,m,1);
  } else if (m->_ft == PL_FILL_TEXTURE) {
    if (m->_st == PL_SHADE_NONE)
      _plGenerateTexturePalette(pal,m,m->Texture,1);
    else _plGeneratePhongTexturePalette(pal,m,m->Texture,1);
  } else if (m->_ft == PL_FILL_ENVIRONMENT) {
    if (m->_st == PL_SHADE_NONE) 
      _plGenerateTexturePalette(pal,m,m->Environment,1);
    else _plGeneratePhongTexturePalette(pal,m,m->Environment,1);
  } else if (m->_ft == (PL_FILL_ENVIRONMENT|PL_FILL_TEXTURE))
    _plGenerateTextureEnvPalette(pal,m,1);
  else if (m->_ft == PL_FILL_TRANSPARENT) {
    if (m->_st == PL_SHADE_NONE) _plGenerateTransparentPalette(pal,m,1);
    else _plGeneratePhongTransparentPalette(pal,m,1);
  }
}


static void _plGenerateSinglePalette(unsigned char *pal, 
                                     pl_MaterialType *m, char g) {
  m->_ColorsUsed = 1;
  if (!g) return;
  pal += m->_ColorBase*3;
  *pal++ = m->Red;
  *pal++ = m->Green;
  *pal++ = m->Blue;
}

static void _plGeneratePhongPalette(unsigned char *pal, 
                                    pl_MaterialType *m, char g) {
  register pl_uInt32Type i = m->NumGradients;
  register pl_sInt32Type c;
  pl_FloatType a, da;
  pl_FloatType ca,cb;
  m->_ColorsUsed = m->NumGradients;
  if (!g) return;
  pal += m->_ColorBase*3;
  a = PL_PI/2;
  da = -PL_PI/(2 * (m->NumGradients-1));
  do {
    if (m->NumGradients == 1) ca = 1;
    else {
      ca = cos((double) a);
      a += da;
    }
    cb = pow((double) ca, (double) m->Shininess);

    c = (pl_uInt32Type) (cb * ((pl_FloatType) m->RedSpec)) +
        (ca * ((pl_FloatType) m->Red)) + m->AmbientLight;
    *(pal++) = plMin(c,255);

    c = (pl_uInt32Type) (cb * ((pl_FloatType) m->GreenSpec)) +
        (ca * ((pl_FloatType) m->Green)) + m->AmbientLight;
    *(pal++) = plMin(c,255);

    c = (pl_uInt32Type) (cb * ((pl_FloatType) m->BlueSpec)) +
        (ca * ((pl_FloatType) m->Blue)) + m->AmbientLight;
    *(pal++) = plMin(c,255);
  } while (--i); 
}

static void _plGenerateTextureEnvPalette(unsigned char *pal,
                                         pl_MaterialType *m, char g) {
  pl_sInt32Type whichlevel, whichindex, c;
  unsigned char *texpal, *envpal;
  m->_ColorsUsed = m->Texture->NumColors*m->Environment->NumColors;
  if (!g) return;
  pal += 3*m->_ColorBase;
  envpal = m->Environment->PaletteData;
  for (whichlevel = 0; whichlevel < m->Environment->NumColors; whichlevel++) {
    texpal = m->Texture->PaletteData;
    for (whichindex = 0; whichindex < m->Texture->NumColors; whichindex++) {
      c = (*texpal++) + *(envpal); *pal++ = plMin(255,c);
      c = (*texpal++) + *(envpal+1); *pal++ = plMin(255,c);
      c = (*texpal++) + *(envpal+2); *pal++ = plMin(255,c);
    }
    envpal += 3;
    m->_AddTable[whichlevel] = m->_ColorBase + whichlevel*m->Texture->NumColors;
  }
}

static void _plGenerateTexturePalette(unsigned char *pal,
                                      pl_MaterialType *m,
                                      pl_TextureType *t, char g) {
  register unsigned char *ppal;
  register pl_sInt32Type c, i;
  m->_ColorsUsed = t->NumColors;
  if (!g) return;
  pal += 3*m->_ColorBase;
  ppal = t->PaletteData;
  i = t->NumColors;
  do {
    c = m->AmbientLight + m->Red + *ppal++;
    *(pal++) = plMin(c,255);
    c = m->AmbientLight + m->Green + *ppal++;
    *(pal++) = plMin(c,255);
    c = m->AmbientLight + m->Blue + *ppal++;
    *(pal++) = plMin(c,255);
  } while (--i);
  m->_AddTable[0] = m->_ColorBase;
}

static void _plGeneratePhongTexturePalette(unsigned char *pal,
                                       pl_MaterialType *m,
                                       pl_TextureType *t, char g) {
  pl_FloatType a, ca, cb, da;
  register unsigned char *ppal;
  register pl_sInt32Type c, i, i2;
  pl_uInt32Type num_shades = (m->NumGradients / t->NumColors);
  m->_ColorsUsed = num_shades*t->NumColors;
  if (!g) return;
  pal += 3*m->_ColorBase;
  a = PL_PI/2;
  da = (-PL_PI/2)/(num_shades-1);
  i2 = num_shades;
  do {
    ppal = t->PaletteData;
    ca = cos((double) a);
    a += da;
    cb = pow(ca, (double) m->Shininess);
    i = t->NumColors;
    do {
      c = (pl_uInt32Type) (cb * ((pl_FloatType) m->RedSpec)) +
          (ca*((pl_FloatType)m->Red)) + m->AmbientLight + *ppal++;
      *(pal++) = plMin(c,255);

      c = (pl_uInt32Type) (cb * ((pl_FloatType) m->GreenSpec)) +
          (ca*(pl_FloatType)m->Green) + m->AmbientLight + *ppal++;
      *(pal++) = plMin(c,255);

      c = (pl_uInt32Type) (cb * ((pl_FloatType) m->BlueSpec)) +
          (ca*(pl_FloatType)m->Blue) + m->AmbientLight + *ppal++;
      *(pal++) = plMin(c,255);
    } while (--i);
  } while (--i2);
  ca = 0;
  ppal = m->_AddTable;
  i = 256;
  do {
    a = pow(sin(ca), (double) m->Shininess) * num_shades;
    ca += PL_PI/512.0;
    *ppal++ = (unsigned char)
      (m->_ColorBase + ((pl_sInt32Type) a)*t->NumColors);
  } while (--i);
}

static void _plGeneratePhongTransparentPalette(unsigned char *pal,
                                              pl_MaterialType *m, char g) {
  pl_sInt32Type i;
  pl_sInt32Type intensity;
  m->_tsfact = m->NumGradients*(1.0/(1+m->Transparent));
  _plGeneratePhongPalette(pal,m,g);
  if (!g) return;
  for (i = 0; i < 256; i ++) {
    intensity = *pal++;
    intensity += *pal++;
    intensity += *pal++;
    m->_AddTable[i] = m->_ColorBase + 
      ((intensity*(m->NumGradients-m->_tsfact))/768);
  }
}

static void  _plGenerateTransparentPalette(unsigned char *pal,
                                           pl_MaterialType *m, char g) {
  pl_sInt32Type i;
  pl_sInt32Type intensity;
  _plGeneratePhongPalette(pal,m,g);
  if (!g) return;
  for (i = 0; i < 256; i ++) {
    intensity = *pal++;
    intensity += *pal++;
    intensity += *pal++;
    m->_AddTable[i] = m->_ColorBase + ((intensity*m->NumGradients)/768);
  }
  m->_tsfact = 0;
}
