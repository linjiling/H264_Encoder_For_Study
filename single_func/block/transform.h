#ifndef _TRANSFORM_H_ 
#define _TRANSFORM_H_
#include "global.h"

int residual_transform_quant_luma_4x4(Macroblock *currMB, ColorPlane pl, int block_x,int block_y, int *coeff_cost, int intra);

#endif
