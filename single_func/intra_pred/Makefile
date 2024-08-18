CC=gcc
CFLAGS=-I.

%.o: %.c %.h
	$(CC) -c -o $@ $< $(CFLAGS)

intra_pred: intra_pred.o intra_pred4x4.o mb_access.o memalloc.o rdopt.o
	$(CC) -o intra_pred intra_pred.o intra_pred4x4.o mb_access.o memalloc.o rdopt.o

.PHONY: clean

clean:
	rm -f intra_pred.o intra_pred4x4.o mb_access.o memalloc.o rdopt.o
