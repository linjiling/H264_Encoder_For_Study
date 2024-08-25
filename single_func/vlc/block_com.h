#ifndef _BLOCK_COM_H_
#define _BLOCK_COM_H_

int predict_nnz(Macroblock *currMB, int block_type, int i,int j);
int predict_nnz_chroma(Macroblock *currMB, int i,int j);
#endif
