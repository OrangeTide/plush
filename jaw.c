/******************************************************************************
**                         Plush Version 1.0                                 **
**                                                                           **
**                        .JAW Object Reader                                 **
**                                                                           **
**             All code copyright (c) 1996-1997, Justin Frankel              **
**          Based on source by Jawed Karim, creator of the format            **
******************************************************************************/

/*
** Notes on .JAW files:
**   This is a file format created by Jawed Karim for Jaw3D 
**     (http://jaw3d.home.ml.org).
**   It is very simple, and lets one easily create ones own models using only
**   a text editor. The format is pretty simple:
**     The first line must be "Light: (x,y,z)" where x,y, and z are the x y and
**        z components of the lightsource vector (I think ;)
**     A series of lines, numbered 0 to n, in the format of 
**        "i: x y z", where i is the vertex number (which should be listed in 
**        order, and x y and z are the coordinates of that vertex.
**     A series of lines, having the format "tri a, b, c" where a b and c are 
**        the vertices that the face uses. It is unclear at this time which
**        way the vertices are listed (ccw or cw), so just make 'em consistent
**        and you can always use plFlipObjectNormals() on the loaded object.
**   That's it! (I told ya it was simple).
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../source/plush.h"

pl_ObjectType *plReadObjectJAW(char *filename, pl_MaterialType *m) {
  FILE *jawfile;
  pl_ObjectType *obj;
  int i, crap;
  char line[256];
  int total_points = 0, total_polys = 0;
  if ((jawfile = fopen(filename, "r")) == NULL) return 0;
  fgets(line, 256, jawfile); /* Plush ignores lightsource info */
  while (fgets(line, 256, jawfile) != NULL)
    if (strstr(line, ":") != NULL) total_points++;

  rewind(jawfile); fgets(line, 256, jawfile);
  while (fgets(line, 256, jawfile) != NULL) 
    if (strstr(line, "tri") != NULL) total_polys++;

  rewind(jawfile); fgets(line, 256, jawfile);
  obj = plAllocObject(total_points,total_polys);

  i = 0;
  while (fgets(line, 256, jawfile) != NULL) if (strstr(line, ":") != NULL) {
    float x, y, z;
    sscanf(line, "%d: %f %f %f",&crap,&x,&y,&z);
    obj->Vertices[i].x = (pl_FloatType) x;
    obj->Vertices[i].y = (pl_FloatType) y;
    obj->Vertices[i].z = (pl_FloatType) z;
    i++;
  }
  rewind(jawfile); fgets(line, 256, jawfile);
  i = 0;
  while (fgets(line, 256, jawfile) != NULL) if (strstr(line, "tri") != NULL) {
    int a,b,c;
    sscanf(line, "tri %d, %d, %d", &a, &b, &c);
    obj->Faces[i].Vertices[0] = obj->Vertices + a;
    obj->Faces[i].Vertices[1] = obj->Vertices + c;
    obj->Faces[i].Vertices[2] = obj->Vertices + b;
    obj->Faces[i].Material = m;
    i++;
  }
  fclose(jawfile);
  plCalcNormals(obj);
  return obj;
}	
