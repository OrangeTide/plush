/******************************************************************************
**                         Plush Version 1.0                                 **
**                                                                           **
**                       Object Primitives Code                              **
**                                                                           **
**             All code copyright (c) 1996-1997, Justin Frankel              **
******************************************************************************/


/*
** Notes:
**   Most of these routines are highly unoptimized.
**   They could all use some work, such as more capable divisions (Box is
**   most notable), etc... The mapping coordinates are all set up nicely, 
**   though.
*/

#include <math.h>
#include "plush.h"

pl_ObjectType *plMakeTorus(pl_FloatType r1, pl_FloatType r2,
                           unsigned char divrot, unsigned char divrad, 
                           pl_MaterialType *m) {
  pl_ObjectType *o;
  pl_VertexType *v;
  pl_FaceType *f;
  pl_uInt32Type x, y;
  pl_FloatType ravg, rt;
  pl_FloatType a, da, al, dal;
  pl_sInt32Type U,V,dU,dV;
  if (divrot < 3) divrot = 3;
  if (divrad < 3) divrad = 3;
  ravg = (r1+r2)*0.5;
  rt = (r2-r1)*0.5;
  o = plAllocObject(divrad*divrot,divrad*divrot*2);
  if (!o) return 0;
  v = o->Vertices;
  a = 0.0;
  da = 2*PL_PI/divrot;
  for (y = 0; y < divrot; y ++) {
    al = 0.0;
    dal = 2*PL_PI/divrad;
    for (x = 0; x < divrad; x ++) {
      v->x = cos(a)*(ravg + cos(al)*rt);
      v->z = sin(a)*(ravg + cos(al)*rt);
      v->y = sin(al)*rt;
      v++;
      al += dal;
    }
    a += da;
  }
  v = o->Vertices;
  f = o->Faces;
  dV = 65535/divrad;
  dU = 65535/divrot;
  U = 0;
  for (y = 0; y < divrot; y ++) {
    V = -32768;
    for (x = 0; x < divrad; x ++) {
      f->Vertices[0] = v+x+y*divrad;
      f->MappingU[0] = U;
      f->MappingV[0] = V;
      f->Vertices[1] = v+(x+1==divrad?0:x+1)+y*divrad;
      f->MappingU[1] = U;
      f->MappingV[1] = V+dV;
      f->Vertices[2] = v+x+(y+1==divrot?0:(y+1)*divrad);
      f->MappingU[2] = U+dU;
      f->MappingV[2] = V;
      f->Material = m;
      f++;
      f->Vertices[0] = v+x+(y+1==divrot?0:(y+1)*divrad);
      f->MappingU[0] = U+dU;
      f->MappingV[0] = V;
      f->Vertices[1] = v+(x+1==divrad?0:x+1)+y*divrad;
      f->MappingU[1] = U;
      f->MappingV[1] = V+dV;
      f->Vertices[2] = v+(x+1==divrad?0:x+1)+(y+1==divrot?0:(y+1)*divrad);
      f->MappingU[2] = U+dU;
      f->MappingV[2] = V+dV;
      f->Material = m;
      f++;
      V += dV;
    }
    U += dU;
  }
  plCalcNormals(o);
  return (o);
}

pl_ObjectType *plMakeSphere(pl_FloatType r, unsigned char divr,
                            unsigned char divh, pl_MaterialType *m) {
  pl_ObjectType *o;
  pl_VertexType *v;
  pl_FaceType *f;
  pl_uInt32Type x, y;
  pl_FloatType a, da, yp, ya, yda, yf;
  pl_sInt32Type U,V,dU,dV;
  if (divh < 3) divh = 3;
  if (divr < 3) divr = 3;
  o = plAllocObject(2+(divh-2)*(divr),2*divr+(divh-3)*divr*2);
  if (!o) return 0;
  v = o->Vertices;
  v->x = v->z = 0.0; v->y = r; v++;
  v->x = v->z = 0.0; v->y = -r; v++;
  ya = 0.0;
  yda = PL_PI/(divh-1);
  da = (PL_PI*2.0)/divr;
  for (y = 0; y < divh - 2; y ++) {
    ya += yda;
    yp = cos(ya)*r;
    yf = sin(ya)*r;
    a = 0.0;
    for (x = 0; x < divr; x ++) {
      v->y = yp;
      v->x = cos(a)*yf;
      v->z = sin(a)*yf;
      v++;
      a += da;
    }
  }
  f = o->Faces;
  v = o->Vertices + 2;
  a = 0.0; 
  U = 0;
  dU = 65536/divr;
  dV = V = 65536/divh;
  for (x = 0; x < divr; x ++) {
    f->Vertices[0] = o->Vertices;
    f->Vertices[1] = v + (x+1==divr ? 0 : x+1);
    f->Vertices[2] = v + x;
    f->MappingU[0] = U;
    f->MappingV[0] = 0;
    f->MappingU[1] = U+dU;
    f->MappingV[1] = V;
    f->MappingU[2] = U;
    f->MappingV[2] = V;
    f->Material = m;
    f++;
    U += dU;
  }
  da = 1.0/(divr+1);
  v = o->Vertices + 2;
  for (x = 0; x < (divh-3); x ++) {
    U = 0;
    for (y = 0; y < divr; y ++) {
      f->Vertices[0] = v+y;
      f->Vertices[1] = v+divr+(y+1==divr?0:y+1);
      f->Vertices[2] = v+y+divr;
      f->MappingU[0] = U;
      f->MappingV[0] = V;
      f->MappingU[1] = U+dU;
      f->MappingV[1] = V+dV;
      f->MappingU[2] = U;
      f->MappingV[2] = V+dV;
      f->Material = m; f++;
      f->Vertices[0] = v+y;
      f->Vertices[1] = v+(y+1==divr?0:y+1);
      f->Vertices[2] = v+(y+1==divr?0:y+1)+divr;
      f->MappingU[0] = U;
      f->MappingV[0] = V;
      f->MappingU[1] = U+dU;
      f->MappingV[1] = V;
      f->MappingU[2] = U+dU;
      f->MappingV[2] = V+dV;
      f->Material = m; f++;
      U += dU;
    }
    V += dV;
    v += divr;
  }
  v = o->Vertices + o->NumVertices - divr;
  U = 0;
  for (x = 0; x < divr; x ++) {
    f->Vertices[0] = o->Vertices + 1;
    f->Vertices[1] = v + x;
    f->Vertices[2] = v + (x+1==divr ? 0 : x+1);
    f->MappingU[0] = U;
    f->MappingV[0] = 65535;
    f->MappingU[1] = U;
    f->MappingV[1] = V;
    f->MappingU[2] = U+dU;
    f->MappingV[2] = V;
    f->Material = m;
    f++;
    U += dU;
  }
  plCalcNormals(o);
  return (o);
}

pl_ObjectType *plMakeCylinder(pl_FloatType r, pl_FloatType h,
                              unsigned char divr, char captop, 
                              char capbottom, pl_MaterialType *m) {
  pl_ObjectType *o;
  pl_VertexType *v, *topverts, *bottomverts, *topcapvert=0, *bottomcapvert=0;
  pl_FaceType *f;
  pl_sInt32Type i;
  pl_FloatType a, da;
  if (divr < 3) divr = 3;
  o = plAllocObject(divr*2+((divr==3)?0:(captop?1:0)+(capbottom?1:0)),
                    divr*2+(divr==3 ? (captop ? 1 : 0) + (capbottom ? 1 : 0) :
                            (captop ? divr : 0) + (capbottom ? divr : 0)));
  if (!o) return 0;
  a = 0.0;
  da = (2.0*PL_PI)/divr;
  v = o->Vertices;
  topverts = v;
  for (i = 0; i < divr; i ++) {
    v->y = h/2.0; v->x = r*cos(a); v->z = r*sin(a);
    v->eMappingU = 32768 + (32768.0*cos(a));
    v->eMappingV = 32768 + (32768.0*sin(a));
    v++; a += da;
  }
  bottomverts = v;
  a = 0.0;
  for (i = 0; i < divr; i ++) {
    v->y = -h/2.0; v->x = r*cos(a); v->z = r*sin(a);
    v->eMappingU = 32768 + (32768.0*cos(a));
    v->eMappingV = 32768 + (32768.0*sin(a));
    v++; a += da;
  }
  if (captop && divr != 3) {
    topcapvert = v;
    v->y = h / 2.0; v->x = v->z = 0;
    v++;
  }
  if (capbottom && divr != 3) {
    bottomcapvert = v;
    v->y = -h / 2.0; v->x = v->z = 0;
    v++;
  }
  f = o->Faces;
  for (i = 0; i < divr; i ++) {
    f->Vertices[0] = bottomverts + i;
    f->Vertices[1] = topverts + i;
    f->Vertices[2] = bottomverts + (i == divr-1 ? 0 : i+1);
    f->MappingV[0] = f->MappingV[2] = 65536; f->MappingV[1] = 0;
    f->MappingU[0] = f->MappingU[1] = (i<<16)/divr;
    f->MappingU[2] = ((i+1)<<16)/divr;
    f->Material = m; f++;
    f->Vertices[0] = bottomverts + (i == divr-1 ? 0 : i+1);
    f->Vertices[1] = topverts + i;
    f->Vertices[2] = topverts + (i == divr-1 ? 0 : i+1);
    f->MappingV[1] = f->MappingV[2] = 0; f->MappingV[0] = 65536;
    f->MappingU[0] = f->MappingU[2] = ((i+1)<<16)/divr;
    f->MappingU[1] = (i<<16)/divr;
    f->Material = m; f++;
  }
  if (captop) {
    if (divr == 3) {
      f->Vertices[0] = topverts + 0;
      f->Vertices[1] = topverts + 2;
      f->Vertices[2] = topverts + 1;
      f->MappingU[0] = topverts[0].eMappingU;
      f->MappingV[0] = topverts[0].eMappingV;
      f->MappingU[1] = topverts[1].eMappingU;
      f->MappingV[1] = topverts[1].eMappingV;
      f->MappingU[2] = topverts[2].eMappingU;
      f->MappingV[2] = topverts[2].eMappingV;
      f->Material = m; f++;
    } else {
      for (i = 0; i < divr; i ++) {
        f->Vertices[0] = topverts + (i == divr-1 ? 0 : i + 1);
        f->Vertices[1] = topverts + i;
        f->Vertices[2] = topcapvert;
        f->MappingU[0] = topverts[(i==divr-1?0:i+1)].eMappingU;
        f->MappingV[0] = topverts[(i==divr-1?0:i+1)].eMappingV;
        f->MappingU[1] = topverts[i].eMappingU;
        f->MappingV[1] = topverts[i].eMappingV;
        f->MappingU[2] = f->MappingV[2] = 32768;
        f->Material = m; f++;
      }
    }
  }
  if (capbottom) {
    if (divr == 3) {
      f->Vertices[0] = bottomverts + 0;
      f->Vertices[1] = bottomverts + 1;
      f->Vertices[2] = bottomverts + 2;
      f->MappingU[0] = bottomverts[0].eMappingU;
      f->MappingV[0] = bottomverts[0].eMappingV;
      f->MappingU[1] = bottomverts[1].eMappingU;
      f->MappingV[1] = bottomverts[1].eMappingV;
      f->MappingU[2] = bottomverts[2].eMappingU;
      f->MappingV[2] = bottomverts[2].eMappingV;
      f->Material = m; f++;
    } else {
      for (i = 0; i < divr; i ++) {
        f->Vertices[0] = bottomverts + i;
        f->Vertices[1] = bottomverts + (i == divr-1 ? 0 : i + 1);
        f->Vertices[2] = bottomcapvert;
        f->MappingU[0] = bottomverts[i].eMappingU;
        f->MappingV[0] = bottomverts[i].eMappingV;
        f->MappingU[1] = bottomverts[(i==divr-1?0:i+1)].eMappingU;
        f->MappingV[1] = bottomverts[(i==divr-1?0:i+1)].eMappingV;
        f->MappingU[2] = f->MappingV[2] = 32768;
        f->Material = m; f++;
      }
    }
  }
  plCalcNormals(o);
  return (o);
}

pl_ObjectType *plMakeCone(pl_FloatType r, pl_FloatType h,
                          unsigned char div, char cap, pl_MaterialType *m) {
  pl_ObjectType *o;
  pl_VertexType *v;
  pl_FaceType *f;
  pl_sInt32Type i;
  pl_FloatType a, da;
  if (div < 3) div = 3;
  o = plAllocObject(div + (div == 3 ? 1 : (cap ? 2 : 1)),
                    div + (div == 3 ? 1 : (cap ? div : 0)));
  if (!o) return 0;
  v = o->Vertices;
  v->x = v->z = 0; v->y = h/2;
  v->eMappingU = 1<<15;
  v->eMappingV = 1<<15;
  v++;
  a = 0.0;
  da = (2.0*PL_PI)/div;
  for (i = 1; i <= div; i ++) {
    v->y = h/(-2.0);
    v->x = r*cos(a);
    v->z = r*sin(a);
    v->eMappingU = 32768 + (pl_sInt32Type) (cos(a)*32768.0);
    v->eMappingV = 32768 + (pl_sInt32Type) (sin(a)*32768.0);
    a += da;
    v++;
  }
  if (cap && div != 3) {
    v->y = h / -2.0; v->x = v->z = 0;
    v->eMappingU = 1<<15;
    v->eMappingV = 1<<15;
    v++;
  }
  f = o->Faces;
  for (i = 1; i <= div; i ++) {
    f->Vertices[0] = o->Vertices;
    f->Vertices[1] = o->Vertices + (i == div ? 1 : i + 1);
    f->Vertices[2] = o->Vertices + i;
    f->MappingU[0] = o->Vertices[0].eMappingU;
    f->MappingV[0] = o->Vertices[0].eMappingV;
    f->MappingU[1] = o->Vertices[(i==div?1:i+1)].eMappingU;
    f->MappingV[1] = o->Vertices[(i==div?1:i+1)].eMappingV;
    f->MappingU[2] = o->Vertices[i].eMappingU;
    f->MappingV[2] = o->Vertices[i].eMappingV;
    f->Material = m;
    f++;
  }
  if (cap) {
    if (div == 3) {
      f->Vertices[0] = o->Vertices + 1;
      f->Vertices[1] = o->Vertices + 2;
      f->Vertices[2] = o->Vertices + 3;
      f->MappingU[0] = o->Vertices[1].eMappingU;
      f->MappingV[0] = o->Vertices[1].eMappingV;
      f->MappingU[1] = o->Vertices[2].eMappingU;
      f->MappingV[1] = o->Vertices[2].eMappingV;
      f->MappingU[2] = o->Vertices[3].eMappingU;
      f->MappingV[2] = o->Vertices[3].eMappingV;
      f->Material = m;
      f++;
    } else {
      for (i = 1; i <= div; i ++) {
        f->Vertices[0] = o->Vertices + div + 1;
        f->Vertices[1] = o->Vertices + i;
        f->Vertices[2] = o->Vertices + (i==div ? 1 : i+1);
        f->MappingU[0] = o->Vertices[div+1].eMappingU;
        f->MappingV[0] = o->Vertices[div+1].eMappingV;
        f->MappingU[1] = o->Vertices[i].eMappingU;
        f->MappingV[1] = o->Vertices[i].eMappingV;
        f->MappingU[2] = o->Vertices[i==div?1:i+1].eMappingU;
        f->MappingV[2] = o->Vertices[i==div?1:i+1].eMappingV;
        f->Material = m;
        f++;
      }
    }
  }
  plCalcNormals(o);
  return (o);
}

pl_ObjectType *plMakeBox(pl_FloatType w, pl_FloatType d, pl_FloatType h, 
                         pl_MaterialType *m) {
  unsigned char verts[6*6] = { 
                          0,4,1, 1,4,5,
                          0,1,2, 3,2,1,
                          2,3,6, 3,7,6,
                          6,7,4, 4,7,5, 
                          1,7,3, 7,1,5,
                          2,6,0, 4,0,6
                          };
  pl_FloatType map[24*2*3] = {
                       1.0,0.0, 1.0,1.0, 0.0,0.0, 0.0,0.0, 1.0,1.0, 0.0,1.0,
                       0.0,0.0, 1.0,0.0, 0.0,1.0, 1.0,1.0, 0.0,1.0, 1.0,0.0,
                       0.0,0.0, 1.0,0.0, 0.0,1.0, 1.0,0.0, 1.0,1.0, 0.0,1.0,
                       0.0,0.0, 1.0,0.0, 0.0,1.0, 0.0,1.0, 1.0,0.0, 1.0,1.0,
                       1.0,0.0, 0.0,1.0, 0.0,0.0, 0.0,1.0, 1.0,0.0, 1.0,1.0,
                       1.0,0.0, 1.0,1.0, 0.0,0.0, 0.0,1.0, 0.0,0.0, 1.0,1.0
                       };
  pl_FloatType *mm = map;
  unsigned char *vv = verts;
  pl_ObjectType *o;
  pl_VertexType *v;
  pl_FaceType *f;
  pl_uInt16Type x;
  o = plAllocObject(8,12);
  if (!o) return 0;
  v = o->Vertices;
  v->x = -w/2; v->y = h/2; v->z = d/2; v++;
  v->x = w/2; v->y = h/2; v->z = d/2; v++;
  v->x = -w/2; v->y = h/2; v->z = -d/2; v++;
  v->x = w/2; v->y = h/2; v->z = -d/2; v++;
  v->x = -w/2; v->y = -h/2; v->z = d/2; v++;
  v->x = w/2; v->y = -h/2; v->z = d/2; v++;
  v->x = -w/2; v->y = -h/2; v->z = -d/2; v++;
  v->x = w/2; v->y = -h/2; v->z = -d/2; v++;
  f = o->Faces;
  for (x = 0; x < 12; x ++) {
    f->Vertices[0] = o->Vertices + *vv++;
    f->Vertices[1] = o->Vertices + *vv++;
    f->Vertices[2] = o->Vertices + *vv++;
    f->MappingU[0] = (pl_sInt32Type) (*mm++) * 65536.0;
    f->MappingV[0] = (pl_sInt32Type) (*mm++) * 65536.0;
    f->MappingU[1] = (pl_sInt32Type) (*mm++) * 65536.0;
    f->MappingV[1] = (pl_sInt32Type) (*mm++) * 65536.0;
    f->MappingU[2] = (pl_sInt32Type) (*mm++) * 65536.0;
    f->MappingV[2] = (pl_sInt32Type) (*mm++) * 65536.0;
    f->Material = m;
    f++;
  }

  plCalcNormals(o);
  return (o);
}

pl_ObjectType *plMakePlane(pl_FloatType w, pl_FloatType d, 
                           pl_uInt16Type res, pl_MaterialType *m) {
  pl_ObjectType *o;
  pl_VertexType *v;
  pl_FaceType *f;
  pl_uInt16Type x, y;
  o = plAllocObject((res+1)*(res+1),res*res*2);
  if (!o) return 0;
  v = o->Vertices;
  for (y = 0; y <= res; y ++) {
    for (x = 0; x <= res; x ++) {
      v->y = 0;
      v->x = ((x*w)/res) - w/2;
      v->z = ((y*d)/res) - d/2;
      v++;
    }
  }
  f = o->Faces;
  for (y = 0; y < res; y ++) {
    for (x = 0; x < res; x ++) {
      f->Vertices[0] = o->Vertices + x+(y*(res+1));
      f->MappingU[0] = (x<<16)/res;
      f->MappingV[0] = (y<<16)/res;
      f->Vertices[2] = o->Vertices + x+1+(y*(res+1));
      f->MappingU[2] = ((x+1)<<16)/res;
      f->MappingV[2] = (y<<16)/res;
      f->Vertices[1] = o->Vertices + x+((y+1)*(res+1));
      f->MappingU[1] = (x<<16)/res;
      f->MappingV[1] = ((y+1)<<16)/res;
      f->Material = m;
      f++;
      f->Vertices[0] = o->Vertices + x+((y+1)*(res+1));
      f->MappingU[0] = (x<<16)/res;
      f->MappingV[0] = ((y+1)<<16)/res;
      f->Vertices[2] = o->Vertices + x+1+(y*(res+1));
      f->MappingU[2] = ((x+1)<<16)/res;
      f->MappingV[2] = (y<<16)/res;
      f->Vertices[1] = o->Vertices + x+1+((y+1)*(res+1));
      f->MappingU[1] = ((x+1)<<16)/res;
      f->MappingV[1] = ((y+1)<<16)/res;
      f->Material = m;
      f++;
    }
  }
  plCalcNormals(o);
  return (o);
}
