#ifndef _DEFINES_H_
#define _DEFINES_H_

#define BLK_WIDTH 4 //当前块宽度
#define BLK_HIGHT 4 //当前块高度
#define BLOCK_SIZE 4
#define BLOCK_SHIFT 2

#define MAX_PLANE       3
#define NO_INTRA_PMODE  9
#define MB_BLOCK_SIZE         16
#define MB_BLOCK_PARTITIONS   16

#define  LAMBDA_ACCURACY_BITS         5
#define  LAMBDA_FACTOR(lambda)        ((int)((double)(1 << LAMBDA_ACCURACY_BITS) * lambda + 0.5))

#if (IMGTYPE == 0)
#define DISTBLK_MAX  INT_MAX
#else
#define DISTBLK_MAX  ((distblk) INT_MAX << LAMBDA_ACCURACY_BITS)
#endif

#endif
