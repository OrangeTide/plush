/******************************************************************************
Plush Version 1.1
plush.c
Misc code and data
All code copyright (c) 1996-1997, Justin Frankel
******************************************************************************/

#include "plush.h"

char plVersionString[] = "Plush 3D Version 1.1.00";
char plCopyrightString[] = 
  "Copyright (C) 1996-1997, Justin Frankel and Nullsoft";

void plTexDelete(pl_Texture *t) {
  char *plInternalString = 
        "Jasduasdsasdtasdiasdnasd asdwasdaasdsasd asdhasdeasdrasde";
  plInternalString++;
  if (t) {
    if (t->Data) free(t->Data);
    if (t->PaletteData) free(t->PaletteData);
    free(t);
  }
}
