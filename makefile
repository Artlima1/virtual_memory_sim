CC=gcc
CFLAGS=-I.

tp2virtual: main.o virtual_memory.o
	$(CC) -o tp2virtual main.o virtual_memory.o
