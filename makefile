# Plush DJGPP v2 MAKEFILE
# Makes ../libplush.a

# Archiver
AR = ar
# C compiler
CC=gcc
# Debug options
CFLAGS = -s
# Warning options
CFLAGS += -Wall -Wno-sign-compare

# Don't use when doing PL_i386_GCC
CFLAGS += -ansi -pedantic

# Normal optimization flags
CFLAGS += -O4 -mpentium -frisc
# Additional optimization flags
# good to use
CFLAGS += -fomit-frame-pointer
CFLAGS += -ffast-math
# Probably blows the cache on 486s, but is good for iP5's...
CFLAGS += -funroll-loops
#CFLAGS += -S
#CFLAGS += -pg -O2 -m486

default: ../libplush.a

plush.o: plush.h pl_types.h pl_conf.h pl_defs.h
read_pcx.o: plush.h pl_types.h pl_conf.h pl_defs.h
read_cob.o: plush.h pl_types.h pl_conf.h pl_defs.h
read_3ds.o: plush.h pl_types.h pl_conf.h pl_defs.h
read_jaw.o: plush.h pl_types.h pl_conf.h pl_defs.h
mat.o: plush.h pl_types.h pl_conf.h pl_defs.h
obj.o: plush.h pl_types.h pl_conf.h pl_defs.h
light.o: plush.h pl_types.h pl_conf.h pl_defs.h
cam.o: plush.h pl_types.h pl_conf.h pl_defs.h
make.o: plush.h pl_types.h pl_conf.h pl_defs.h
render.o: plush.h pl_types.h pl_conf.h pl_defs.h
math.o: plush.h pl_types.h pl_conf.h pl_defs.h
spline.o: plush.h pl_types.h pl_conf.h pl_defs.h
clip.o: plush.h pl_types.h pl_conf.h pl_defs.h
text.o: plush.h pl_types.h pl_conf.h pl_defs.h
pf_solid.o: plush.h pl_types.h pl_conf.h pl_defs.h putface.h
pf_tex.o: plush.h pl_types.h pl_conf.h pl_defs.h putface.h
pf_ptex.o: plush.h pl_types.h pl_conf.h pl_defs.h putface.h
pf_trans.o: plush.h pl_types.h pl_conf.h pl_defs.h putface.h

OBJECT0 = plush.o read_pcx.o read_cob.o read_3ds.o read_jaw.o mat.o obj.o
OBJECT1 = light.o cam.o make.o render.o math.o spline.o clip.o text.o
RAST = pf_solid.o pf_trans.o pf_tex.o pf_ptex.o

../libplush.a: $(OBJECT0) $(OBJECT1) $(RAST)
	-rm ../libplush.a
	$(AR) rc ../libplush.a $(OBJECT0)
	$(AR) r ../libplush.a $(OBJECT1)
	$(AR) r ../libplush.a $(RAST)
	ranlib ../libplush.a

clean:
	-rm $(OBJECT0) 
	-rm $(OBJECT1) 
	-rm $(RAST)
