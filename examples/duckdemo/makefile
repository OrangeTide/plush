CC = gcc
CFLAGS = -Wall -g
CFLAGS += -O3 -m486 -fomit-frame-pointer
CFLAGS += -fstrength-reduce -ffast-math -I.. -L.. 

default: duckdemo.exe

duckdemo.exe: duckdemo.o
	gcc $(CFLAGS) -o duckdemo duckdemo.o -lplush -lalleg

clean:
	-del duckdemo.o 
	-del duckdemo
	-djp duckdemo.exe
