
#ifndef _RDO_H_
#define _RDO_H_
#include "global.h"

distblk compute_sad4x4_cost(VideoParameters *p_Vid, imgpel **cur_img, imgpel **prd_img, int pic_opix_x, distblk min_cost);

#endif
