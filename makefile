# Plush MAKEFILE
# Makes ../libplush.a

CC=gcc
CFLAGS = -s -Wall
CFLAGS += -fomit-frame-pointer -ffast-math -fstrength-reduce -funroll-loops
CFLAGS += -O3 -m486
CFLAGS += -ansi -pedantic
AR = ar

default: ../libplush.a

plush.o: plush.h
pcx.o: plush.h
cob.o: plush.h
3ds.o: plush.h
jaw.o: plush.h
material.o: plush.h
pm.o: plush.h
math.o: plush.h
make.o: plush.h
render.o: plush.h
objutil.o: plush.h
dscan.o: plush.h
spline.o: plush.h
pf_solid.o: plush.h putface.h
pf_tex.o: plush.h putface.h
pf_ptex.o: plush.h putface.h
pf_trans.o: plush.h putface.h
zf_solid.o: plush.h putface.h
zf_tex.o: plush.h putface.h
zf_ptex.o: plush.h putface.h
zf_trans.o: plush.h putface.h

OBJECT0 = plush.o pcx.o material.o math.o render.o objutil.o 3ds.o cob.o pm.o
OBJECT1 = dscan.o make.o spline.o pf_solid.o pf_tex.o pf_trans.o pf_ptex.o
OBJECT2 = jaw.o zf_solid.o zf_tex.o zf_trans.o zf_ptex.o

../libplush.a: $(OBJECT0) $(OBJECT1) $(OBJECT2)
	-rm ../libplush.a
	$(AR) rc ../libplush.a $(OBJECT0)
	$(AR) r ../libplush.a $(OBJECT1)
	$(AR) r ../libplush.a $(OBJECT2)
	ranlib ../libplush.a

clean:  
	-rm $(OBJECT0) 
	-rm $(OBJECT1) 
	-rm $(OBJECT2)
