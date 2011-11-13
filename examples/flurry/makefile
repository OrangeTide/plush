CC=gcc
CFLAGS=-m486 -O3 -fomit-frame-pointer -ffast-math -g
CFLAGS += -I.. -L..

default: flurry.exe flurry2.exe

flurry.exe: flurry.o
	gcc $(CFLAGS) -o flurry.exe flurry.o -lplush -lalleg

flurry2.exe: flurry2.o
	gcc $(CFLAGS) -o flurry2.exe flurry2.o -lplush
