#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "typedefs.h"
#include "defines.h"

typedef struct block_pos BlockPos;

struct block_pos
{
  short x;
  short y;
};

typedef struct pix_pos
{
  int   available;
  int   mb_addr;
  short x;
  short y;
  short pos_x;
  short pos_y;
} PixelPos;

typedef struct storable_picture
{
    imgpel **   p_curr_img;
} StorablePicture;

typedef struct inp_par_enc
{
    int UseConstrainedIntraPred;
} InputParameters;

typedef struct video_par
{
    uint32 dc_pred_value;
    struct storable_picture       *enc_picture;
    int mb_size[MAX_PLANE][2];
    unsigned int PicSizeInMbs;
    BlockPos *PicPos;
    unsigned int PicWidthInMbs;
    int width;
    int height;
    int width_blk;
    int height_blk;
    imgpel    **pCurImg;
    char **ipredmode;
    unsigned int FrameSizeInMbs;
} VideoParameters;

typedef struct slice
{
    struct video_par    *p_Vid;
    imgpel ****mpr_4x4;
    int ***mb_ores;
    imgpel ***mb_pred;
} Slice;


typedef struct macroblock_enc
{
    struct video_par   *p_Vid;
    InputParameters    *p_Inp;
    struct slice       *p_Slice;

    imgpel              **intra4x4_pred;
    int                 mbAddrX;
    int                 mbAddrA, mbAddrB, mbAddrC, mbAddrD;
    byte                mbAvailA, mbAvailB, mbAvailC, mbAvailD;
    short               pix_x;
    short               pix_y;
    short               opix_y;
    int                 DeblockCall;
    char                intra_pred_modes   [MB_BLOCK_PARTITIONS];
} Macroblock;

#endif
