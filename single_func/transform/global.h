#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "quant_params.h"
#include "typedefs.h"
#include "defines.h"
#include "types.h"

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
    FrameFormat output; 
    int AdaptRoundingFixed;
} InputParameters;

typedef struct video_par
{
    InputParameters *p_Inp;
    struct quant_params           *p_Quant;
    int ****ARCofAdj4x4;
    int AdaptiveRounding;
    char colour_plane_id;
    ColorFormat yuv_format;
    short max_imgpel_value;
    struct storable_picture       *enc_picture;
    int width;
    int height;
    int num_blk8x8_uv;
    int bitdepth_luma;
    int bitdepth_chroma;
    int bitdepth_luma_qp_scale;
    int bitdepth_chroma_qp_scale;
} VideoParameters;

typedef struct slice
{
    struct video_par    *p_Vid;
    int symbol_mode;
    int     ****cofAC;
    int **tblk16x16;
    int                 disthres;
    int P444_joined;
    imgpel ***mb_pred;
    int ***mb_rres;               //!< the diff pixel values between the original macroblock/block and its prediction (reconstructed)
    int ***mb_ores;
} Slice;

typedef struct macroblock_enc
{
    struct video_par   *p_Vid;
    struct slice       *p_Slice;
    short               pix_x;
    short               pix_y;
    short               subblock_x;
    short               subblock_y;
    byte                is_field_mode;
    short               ar_mode;
    short               qp_scaled[MAX_PLANE];
} Macroblock;

#endif
