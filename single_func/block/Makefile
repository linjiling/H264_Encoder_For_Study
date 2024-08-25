CC=gcc
CFLAGS=-I. -g -O0

%.o: %.c %.h
	$(CC) -c -o $@ $< $(CFLAGS)

intra_pred: block4x4_enc.o intra_pred4x4.o mb_access.o memalloc.o rdopt.o transform.o md_common.o blk_prediction.o
	$(CC) -o block4x4_enc block4x4_enc.o intra_pred4x4.o mb_access.o memalloc.o rdopt.o transform.o md_common.o blk_prediction.o

.PHONY: clean

clean:
	rm -f block4x4_enc.o intra_pred4x4.o mb_access.o memalloc.o rdopt.o transform.o md_common.o blk_prediction.o
