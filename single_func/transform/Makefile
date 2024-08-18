CC=gcc
CFLAGS=-I. -g -O0

%.o: %.c %.h
	$(CC) -c -o $@ $< $(CFLAGS)

transform: transform.o md_common.o blk_prediction.o memalloc.o 
	$(CC) -o transform transform.o md_common.o blk_prediction.o memalloc.o

.PHONY: clean

clean:
	rm -f transform.o md_common.o blk_prediction.o memalloc.o
