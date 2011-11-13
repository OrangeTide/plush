CC=gcc
CFLAGS=-m486 -O3 -fomit-frame-pointer -ffast-math -funroll-loops

default: fly.exe

fly.exe: fly.o ../libplush.a
	gcc -s -o fly.exe fly.o -lplush
