CC=gcc
CFLAGS=-I. -g -O0

%.o: %.c %.h
	$(CC) -c -o $@ $< $(CFLAGS)

entropy: entropy.o vlc_base.o  block_com.o mb_access.o memalloc.o
	$(CC) -o entropy entropy.o vlc_base.o block_com.o mb_access.o memalloc.o

.PHONY: clean

clean:
	rm -f entropy.o vlc_base.o block_com.o mb_access.o memalloc.o
