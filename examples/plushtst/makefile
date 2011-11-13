CC = gcc
CFLAGS = -Wall -g
CFLAGS += -O2 -m486 
CFLAGS += -fomit-frame-pointer -fstrength-reduce -ffast-math -I..

default: plushtst.exe

plushtst.o: ../source/plush.h

plushtst.exe: plushtst.o
	gcc -g -L.. -o plushtst plushtst.o -lplush

clean:
	-rm plushtst.o
	-djp plushtst
