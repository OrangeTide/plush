#include "plush.h"

void plSplineInit(pl_SplineType *s) {
  pl_FloatType a,b,c,d;

  a = (1-s->tens)*(1+s->cont)*(1+s->bias);
  b = (1-s->tens)*(1-s->cont)*(1-s->bias);
  c = (1-s->tens)*(1-s->cont)*(1+s->bias);
  d = (1-s->tens)*(1+s->cont)*(1-s->bias);

  s->v1 = s->t1 = -a / 2.0; s->u1 = a; 
  s->u2 = (-6-2*a+2*b+c)/2.0; s->v2 = (a-b)/2.0; s->t2 = (4+a-b-c) / 2.0; 
  s->t3 = (-4+b+c-d) / 2.0; 
  s->u3 = (6-2*b-c+d)/2.0; 
  s->v3 = b/2.0; 
  s->t4 = d/2.0; s->u4 = -s->t4;
}

void plSplineGetPoint(pl_SplineType *s, pl_FloatType frame, 
                         pl_FloatType *out) {
  pl_uInt32Type i,w;
  pl_FloatType A,B,C,t,t2,t3;
  pl_FloatType *keys;

  frame = plMax(frame,1.0);
  i = w = s->numElem;
  keys = s->keys + ((pl_uInt32Type) frame)*w;
  t = frame - (pl_FloatType) ((pl_uInt32Type) frame);
  t2 = t*t;
  t3 = t2*t;

  while (i--) {
    A = s->t1*keys[-w]+s->t2*keys[0]+s->t3*keys[w]+s->t4*keys[w*2];
    B = s->u1*keys[-w]+s->u2*keys[0]+s->u3*keys[w]+s->u4*keys[w*2];
    C = s->v1*keys[-w]+s->v2*keys[0]+s->v3*keys[w];
    *out = A*t3 + B*t2 + C*t + keys[0];
    out++;
    keys++;
  }
}
