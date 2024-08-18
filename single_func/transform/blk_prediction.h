#ifndef _BLK_PREDICTION_H_
#define _BLK_PREDICTION_H_
#include "typedefs.h"

void compute_residue    (imgpel **curImg, imgpel **mb_pred, int **mb_rres, int mb_x, int opix_x, int width, int height);
void sample_reconstruct (imgpel **curImg, imgpel **mb_pred, int **mb_rres, int mb_x, int opix_x, int width, int height, int max_imgpel_value, int dq_bits);
#endif
