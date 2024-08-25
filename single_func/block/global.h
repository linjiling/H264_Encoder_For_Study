#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "quant_params.h"
#include "typedefs.h"
#include "defines.h"
#include "types.h"

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

typedef struct frame_format
{
    int         bit_depth[3];
} FrameFormat;

typedef struct inp_par_enc
{
    // for intra pred
    int UseConstrainedIntraPred;
    // for transform
    FrameFormat output;
    int AdaptRoundingFixed;
} InputParameters;

typedef struct video_par
{
    struct storable_picture       *enc_picture;
    int width;
    int height;

    // for intra pred
    uint32 dc_pred_value;
    int mb_size[MAX_PLANE][2];
    unsigned int PicSizeInMbs;
    BlockPos *PicPos;
    unsigned int PicWidthInMbs;
    int width_blk;
    int height_blk;
    imgpel    **pCurImg;
    char **ipredmode;
    unsigned int FrameSizeInMbs;
    // for transform
    InputParameters *p_Inp;
    struct quant_params           *p_Quant;
    int ****ARCofAdj4x4;
    int AdaptiveRounding;
    char colour_plane_id;
    ColorFormat yuv_format;
    short max_imgpel_value;
    int num_blk8x8_uv;
    int bitdepth_luma;
    int bitdepth_chroma;
    int bitdepth_luma_qp_scale;
    int bitdepth_chroma_qp_scale;
} VideoParameters;

typedef struct slice
{
    struct video_par    *p_Vid;
    imgpel ***mb_pred;
    int    ***mb_ores;
    // for intra pred
    imgpel ****mpr_4x4;
    // for transform
    int ***mb_rres;
    int **tblk16x16;
    int ****cofAC;
    int symbol_mode;
    int disthres;
    int P444_joined;
} Slice;


typedef struct macroblock_enc
{
    struct video_par   *p_Vid;
    struct slice       *p_Slice;
    InputParameters    *p_Inp;
    short               pix_x;
    short               pix_y;

    // for intra pred
    short               opix_y;
    int                 mbAddrX;
    int                 mbAddrA, mbAddrB, mbAddrC, mbAddrD;
    byte                mbAvailA, mbAvailB, mbAvailC, mbAvailD;
    int                 DeblockCall;
    char                intra_pred_modes   [MB_BLOCK_PARTITIONS];
    imgpel              **intra4x4_pred;
    // for transform
    short               subblock_x;
    short               subblock_y;
    byte                is_field_mode;
    short               ar_mode;
    short               qp_scaled[MAX_PLANE];
} Macroblock;

#endif
