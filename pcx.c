/******************************************************************************
**                         Plush Version 1.0                                 **
**                                                                           **
**                        PCX Texture Reader                                 **
**                                                                           **
**             All code copyright (c) 1996-1997, Justin Frankel              **
******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "plush.h"

static char _plHiBit(pl_sInt32Type);
static pl_sInt32Type _plOptimizeImage(unsigned char *, unsigned char *,
                                      pl_sInt32Type);
static pl_sInt16Type _plReadPCX(char *filename, 
                                pl_sInt16Type *width, pl_sInt16Type *height, 
                                unsigned char **pal, unsigned char **data);

pl_TextureType *plReadTexturePCX(char *fn) {
  unsigned char *data;
  unsigned char *pal;
  pl_sInt16Type x,y;
  pl_TextureType *t;
  if (_plReadPCX(fn,&x,&y,&pal,&data) < 0) return 0;
  t = (pl_TextureType *) malloc(sizeof(pl_TextureType));
  if (!t) return 0;
  t->Width = _plHiBit(x);
  t->Height = _plHiBit(y);
  if (1 << t->Width != x || 1 << t->Height != y) {
    free(data);
    free(pal);
    free(t);
    return 0;
  }
  t->NumColors = _plOptimizeImage(pal, data,x*y);
  t->Data = data;
  t->PaletteData = pal;
  return t;
}


static char _plHiBit(pl_sInt32Type x) {
  pl_sInt32Type i = 16, mask = 1<<16;
  while (mask) {
    if (x & mask) return i;
    mask >>= 1; i--;
  }
  return 0;
}

static pl_sInt32Type _plOptimizeImage(unsigned char *pal, unsigned char *data,
                                      pl_sInt32Type len) {
  unsigned char colors[256];
  unsigned char *dd = data;
  pl_sInt32Type x, lastused, firstunused;
  memset(colors,0,256);
  for (x = 0; x < len; x ++) colors[(int) *dd++] = 1;
  do {
    firstunused = -1;
    for (x = 0; x < 256 && firstunused == -1; x ++) {
      if (!colors[x]) firstunused = x;
    }
    if (firstunused == -1) return 256;
    lastused = -1;
    for (x = 0; x < 256; x ++) {
      if (colors[x]) lastused = x;
    }
    if (lastused == -1) return 0;
    if (lastused > firstunused) {
      dd = data;
      pal[firstunused*3] = pal[lastused*3];
      pal[firstunused*3+1] = pal[lastused*3+1];
      pal[firstunused*3+2] = pal[lastused*3+2];
      colors[lastused] = 0;
      colors[firstunused] = 1;
      for (x = 0; x < len; x ++) {
        if (*dd == lastused) *dd = firstunused;
        dd++;
      }
    }
  } while (firstunused < lastused);
  return (lastused+1);
}

static pl_sInt16Type _plReadPCX(char *filename, pl_sInt16Type *width,
                                pl_sInt16Type *height, 
                                unsigned char **pal, unsigned char **data) {
  pl_sInt16Type sx, sy, ex, ey;
  FILE *fp = fopen(filename,"rb");
  unsigned char *pall, *data2;
  if (!fp) return -1;
  fgetc(fp);
  if (fgetc(fp) != 5) { fclose(fp); return -2; }
  if (fgetc(fp) != 1) { fclose(fp); return -2; }
  if (fgetc(fp) != 8) { fclose(fp); return -3; }
  sx = fgetc(fp); sx |= fgetc(fp)<<8;
  sy = fgetc(fp); sy |= fgetc(fp)<<8;
  ex = fgetc(fp); ex |= fgetc(fp)<<8;
  ey = fgetc(fp); ey |= fgetc(fp)<<8;
  *width = ex - sx + 1;
  *height = ey - sy + 1;
  fseek(fp,128,SEEK_SET);
  if (feof(fp)) { fclose(fp); return -4; }
  *data = (unsigned char *) malloc((*width) * (*height));
  if (!*data) { fclose(fp); return -128; }
  sx = *height;
  data2 = *data;
  do { 
    int xpos = 0;
    do {
      char c = fgetc(fp);
      if ((c & 192) == 192) {
        char oc = fgetc(fp);
        c &= ~192;
        do {
          *(data2++) = oc;
          xpos++;
        } while (--c && xpos < *width);
      } else {
        *(data2++) = c;
        xpos++;
      }
    } while (xpos < *width);
  } while (--sx);
  if (feof(fp)) { fclose(fp); free(*data); return -5; }
  fseek(fp,-769,SEEK_END);
  if (fgetc(fp) != 12) { fclose(fp); free(*data); return -6; }
  *pal = (unsigned char *) malloc(768);
  if (!*pal) { fclose(fp); free(*data); return -7; }
  pall = *pal;
  fread(pall,3,256,fp);
  fclose(fp);
  return 0;
}
