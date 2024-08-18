#ifndef _MB_ACCESS_H_
#define _MB_ACCESS_H_

#include "global.h"
extern void getNonAffNeighbour(Macroblock *currMB, int xN, int yN, int mb_size[2], PixelPos *pix);

void get4x4Neighbour (Macroblock *currMB, int block_x, int block_y, int mb_size[2], PixelPos *pix);
#endif
