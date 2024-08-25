#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "intra_pred4x4.h"
#include "mb_access.h"
#include "ifunctions.h"
#include "memalloc.h"
#include "rdopt.h"
#include "transform.h"

static const byte COEFF_COST4x4[3][16] =
{
  {3,2,2,1,1,1,0,0,0,0,0,0,0,0,0,0},
  {9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9},
  {3,2,2,1,1,1,0,0,0,0,0,0,0,0,0,0},
};
//! single scan pattern
static const byte SNGL_SCAN[16][2] =
{
  {0,0},{1,0},{0,1},{0,2},
  {1,1},{2,0},{3,0},{2,1},
  {1,2},{0,3},{1,3},{2,2},
  {3,1},{3,2},{2,3},{3,3}
};

//! field scan pattern
static const byte FIELD_SCAN[16][2] =
{
  {0,0},{0,1},{1,0},{0,2},
  {0,3},{1,1},{1,2},{1,3},
  {2,0},{2,1},{2,2},{2,3},
  {3,0},{3,1},{3,2},{3,3}
};

static const int quant_coef[6][4][4] = {
  {{13107, 8066,13107, 8066},{ 8066, 5243, 8066, 5243},{13107, 8066,13107, 8066},{ 8066, 5243, 8066, 5243}},
  {{11916, 7490,11916, 7490},{ 7490, 4660, 7490, 4660},{11916, 7490,11916, 7490},{ 7490, 4660, 7490, 4660}},
  {{10082, 6554,10082, 6554},{ 6554, 4194, 6554, 4194},{10082, 6554,10082, 6554},{ 6554, 4194, 6554, 4194}},
  {{ 9362, 5825, 9362, 5825},{ 5825, 3647, 5825, 3647},{ 9362, 5825, 9362, 5825},{ 5825, 3647, 5825, 3647}},
  {{ 8192, 5243, 8192, 5243},{ 5243, 3355, 5243, 3355},{ 8192, 5243, 8192, 5243},{ 5243, 3355, 5243, 3355}},
  {{ 7282, 4559, 7282, 4559},{ 4559, 2893, 4559, 2893},{ 7282, 4559, 7282, 4559},{ 4559, 2893, 4559, 2893}}
};

const int dequant_coef[6][4][4] = {
  {{10, 13, 10, 13},{ 13, 16, 13, 16},{10, 13, 10, 13},{ 13, 16, 13, 16}},
  {{11, 14, 11, 14},{ 14, 18, 14, 18},{11, 14, 11, 14},{ 14, 18, 14, 18}},
  {{13, 16, 13, 16},{ 16, 20, 16, 20},{13, 16, 13, 16},{ 16, 20, 16, 20}},
  {{14, 18, 14, 18},{ 18, 23, 18, 23},{14, 18, 14, 18},{ 18, 23, 18, 23}},
  {{16, 20, 16, 20},{ 20, 25, 20, 25},{16, 20, 16, 20},{ 20, 25, 20, 25}},
  {{18, 23, 18, 23},{ 23, 29, 23, 29},{18, 23, 18, 23},{ 23, 29, 23, 29}}
};

// Default offsets
static const short Offset_intra_default_intra[16] = {
  682, 682, 682, 682,
  682, 682, 682, 682,
  682, 682, 682, 682,
  682, 682, 682, 682
};

static const short Offset_intra_flat_chroma[16] = {
 1024, 1024, 1024, 1024,
 1024, 1024, 1024, 1024,
 1024, 1024, 1024, 1024,
 1024, 1024, 1024, 1024
};

// 输入：
// currMB->pix_x: 当前宏块x坐标，像素为单位
// currMB->pix_y：当前宏块y坐标，像素为单位
// currMB->p_Vid->mb_size[IS_LUMA][2]：当前宏块在亮度分量情况下的宽高尺寸
// p_Vid->pCurImg: 当前帧元素像素buffer，二维imgpel数组形式
// currSlice->mb_pred[0][block_y]: 

int prediction_for_I4x4(Macroblock *currMB, int  lambda,
    int  block_x, int  block_y, int mostProbableMode, int *best_ipmode, distblk*  min_cost)
{
    VideoParameters *p_Vid = currMB->p_Vid;
    Slice *currSlice = currMB->p_Slice;

    int  pic_pix_x   = currMB->pix_x  + block_x;
    int  pic_pix_y   = currMB->pix_y  + block_y;
    int  pic_opix_y  = currMB->opix_y + block_y;
    imgpel ***mpr4x4 = currSlice->mpr_4x4[0];
    imgpel **cur_img = &p_Vid->pCurImg[pic_opix_y];

    int left_available, up_available, all_available;
    int available_mode;
    int ipmode;
    distblk cost;

    distblk  fixedcost = weighted_cost(lambda, 4); //(int) floor(4 * lambda );
    distblk  onecost   = weighted_cost(lambda, 1); //(int) floor( lambda );

    *min_cost = DISTBLK_MAX;
    set_intrapred_4x4(currMB, PLANE_Y, pic_pix_x, pic_pix_y, &left_available, &up_available, &all_available);

    for (ipmode = 0; ipmode < NO_INTRA_PMODE; ipmode++) {
        available_mode =  (all_available) || (ipmode == DC_PRED) ||
        (up_available && (ipmode == VERT_PRED || ipmode == VERT_LEFT_PRED || ipmode == DIAG_DOWN_LEFT_PRED)) ||
        (left_available && (ipmode == HOR_PRED || ipmode == HOR_UP_PRED));

        if (available_mode) {
            // generate intra 4x4 prediction block given availability
            get_intrapred_4x4(currMB, PLANE_Y, ipmode, left_available, up_available);
            cost  = (ipmode == mostProbableMode) ? onecost : fixedcost;
            if (cost < *min_cost) {
                cost += compute_sad4x4_cost(p_Vid, cur_img, mpr4x4[ipmode], pic_pix_x, *min_cost - cost);
                if (cost < *min_cost) {
                    *best_ipmode = ipmode;
                    *min_cost   = cost;
                }
            }
        }
    }
    generate_pred_error_4x4(cur_img, mpr4x4[*best_ipmode], &currSlice->mb_pred[0][block_y], &currSlice->mb_ores[0][block_y], pic_pix_x, block_x);
}

/*!
 ************************************************************************
 * \brief
 *    生成I4x4块的16个像素值残差
 *
 *  \par Input:
 *     b8：当前宏块的第N个8x8块
 *     b4：第N个8x8块中的第M个4x4块。
 *     lambda：lambda值
 *     currMB->pix_x: 当前宏块相对图像的绝对x坐标
 *     currMB->pix_y：当前宏块相对图像的绝对y坐标
 *     p_Vid->mb_size[IS_LUMA]：亮度分量宏块宽度和宏块高度，注意亮度和色度情况这个变量是不尽相同的。
 *     p_Vid->ipredmode：I4x4预测模式数组，共16个元素。
 *  \par Output:
 *      min_cost：最小代价值。
 *      p_Vid->ipredmode: I4x4预测模式数组，共16个元素。
 *      currMB->intra_pred_modes: I4x4预测模式数组，共16个元素。
 ************************************************************************
 */

int encode_one_I4x4_block(Macroblock *currMB, int  b8,  int  b4,  int  lambda,  distblk*  min_cost)
{
    VideoParameters *p_Vid = currMB->p_Vid;

    // block_x/block_y是当前4x4块相对于当前宏块的坐标，即以宏块左上角为原点的坐标，像素为单位。
    int  block_x     = ((b8 & 0x01) << 3) + ((b4 & 0x01) << 2);
    int  block_y     = ((b8 >> 1) << 3)  + ((b4 >> 1) << 2);
    // pic_pix_x/pic_pix_y是当前4x4块相对于整图像的坐标，即以图像左上角为原点的坐标，像素为单位。
    int  pic_pix_x   = currMB->pix_x  + block_x;
    int  pic_pix_y   = currMB->pix_y  + block_y;

    int  *mb_size    = p_Vid->mb_size[IS_LUMA];
    
    int best_ipmode = 0;
    PixelPos left_block, top_block;
    int mostProbableMode;
    char upMode, leftMode;

    get4x4Neighbour(currMB, block_x - 1, block_y    , mb_size, &left_block);
    get4x4Neighbour(currMB, block_x,     block_y - 1, mb_size, &top_block );

    upMode = top_block.available ? p_Vid->ipredmode[top_block.pos_y ][top_block.pos_x ] : (char)  -1;
    leftMode = left_block.available ? p_Vid->ipredmode[left_block.pos_y][left_block.pos_x] : (char) -1;
    mostProbableMode = (upMode < 0 || leftMode < 0) ? DC_PRED : upMode < leftMode ? upMode : leftMode;

    prediction_for_I4x4(currMB, lambda, block_x, block_y, mostProbableMode, &best_ipmode, min_cost);

    p_Vid->ipredmode[pic_pix_y >> 2][pic_pix_x >> 2] = (char) best_ipmode;
    currMB->intra_pred_modes[4*b8+b4] = (char) (mostProbableMode == best_ipmode ? -1 :
                                        (best_ipmode < mostProbableMode ? best_ipmode : best_ipmode - 1));

    return 0;
}

static void allocate_QMatrix (QuantParameters *p_Quant, InputParameters *p_Inp)
{
  int max_bitdepth = imax(p_Inp->output.bit_depth[0], p_Inp->output.bit_depth[1]);
  int max_qp = (3 + 6*(max_bitdepth));

  int bitdepth_qp_scale = 6*(p_Inp->output.bit_depth[0] - 8);
  int i;

  get_mem5Dquant(&p_Quant->q_params_4x4, 3, 2, max_qp + 1, 4, 4);
  get_mem5Dquant(&p_Quant->q_params_8x8, 3, 2, max_qp + 1, 8, 8);

  if ((p_Quant->qp_per_matrix = (int*)malloc((MAX_QP + 1 +  bitdepth_qp_scale)*sizeof(int))) == NULL)
    printf("allocate_QMatrix: p_Quant->qp_per_matrix");
  if ((p_Quant->qp_rem_matrix = (int*)malloc((MAX_QP + 1 +  bitdepth_qp_scale)*sizeof(int))) == NULL)
    printf("allocate_QMatrix: p_Quant->qp_per_matrix");

  for (i = 0; i < MAX_QP + bitdepth_qp_scale + 1; i++)
  {
    p_Quant->qp_per_matrix[i] = i / 6;
    p_Quant->qp_rem_matrix[i] = i % 6;
  }
}

static void set_default_quant4x4(LevelQuantParams **q_params_4x4,  const int (*quant)[4], const int (*dequant)[4])
{
  int i, j;
  for(j=0; j<4; j++)
  {
    for(i=0; i<4; i++)
    {
      q_params_4x4[j][i].ScaleComp    = quant[j][i];
      q_params_4x4[j][i].InvScaleComp = dequant[j][i]<<4;
    }
  }
}

void CalculateQuant4x4Param(VideoParameters *p_Vid)
{
    int k, k_mod;
    QuantParameters *p_Quant  = p_Vid->p_Quant;
    int max_bitdepth = imax(p_Vid->bitdepth_luma, p_Vid->bitdepth_chroma);
    int max_qp = (3 + 6*(max_bitdepth));

    for(k_mod = 0; k_mod <= max_qp; k_mod++)
    {
      k = k_mod % 6;
      set_default_quant4x4(p_Quant->q_params_4x4[0][0][k_mod],  quant_coef[k], dequant_coef[k]);
      set_default_quant4x4(p_Quant->q_params_4x4[0][1][k_mod],  quant_coef[k], dequant_coef[k]);
      set_default_quant4x4(p_Quant->q_params_4x4[1][0][k_mod],  quant_coef[k], dequant_coef[k]);
      set_default_quant4x4(p_Quant->q_params_4x4[1][1][k_mod],  quant_coef[k], dequant_coef[k]);
      set_default_quant4x4(p_Quant->q_params_4x4[2][0][k_mod],  quant_coef[k], dequant_coef[k]);
      set_default_quant4x4(p_Quant->q_params_4x4[2][1][k_mod],  quant_coef[k], dequant_coef[k]);
    }
}

static inline void update_q_offset4x4(LevelQuantParams **q_params, short *offsetList, int q_bits)
{
  int i, j;
  short *p_list = &offsetList[0];
  for (j = 0; j < 4; j++)
  {
    for (i = 0; i < 4; i++)
    {
      q_params[j][i].OffsetComp = (int) *p_list++ << q_bits;
    }
  }
}

void CalculateOffset4x4Param (VideoParameters *p_Vid)
{
    QuantParameters *p_Quant = p_Vid->p_Quant;
    InputParameters *p_Inp = p_Vid->p_Inp;
    int k;
    int qp_per, qp;
    int OffsetBits = 11;

    int max_qp_scale = imax(p_Vid->bitdepth_luma_qp_scale, p_Vid->bitdepth_chroma_qp_scale);
    int max_qp = 51 + max_qp_scale;

    for (qp = 0; qp < max_qp + 1; qp++)
    {
      k = p_Quant->qp_per_matrix [qp];
      qp_per = Q_BITS + k - OffsetBits;
      k = p_Inp->AdaptRoundingFixed ? 0 : qp;

      // Intra4x4 luma
      update_q_offset4x4(p_Quant->q_params_4x4[0][1][qp], p_Quant->OffsetList4x4[k][ 0], qp_per);
      // Intra4x4 chroma u
      update_q_offset4x4(p_Quant->q_params_4x4[1][1][qp], p_Quant->OffsetList4x4[k][ 1], qp_per);
      // Intra4x4 chroma v
      update_q_offset4x4(p_Quant->q_params_4x4[2][1][qp], p_Quant->OffsetList4x4[k][ 2], qp_per);
    }
}

static void allocate_QOffsets (QuantParameters *p_Quant, InputParameters *p_Inp)
{
  int max_bitdepth = imax(p_Inp->output.bit_depth[0], p_Inp->output.bit_depth[1]);
  int max_qp = (3 + 6*(max_bitdepth));

  get_mem3Dshort(&p_Quant->OffsetList4x4, max_qp + 1, 25, 16);

}

void InitOffsetParam (QuantParameters *p_Quant, InputParameters *p_Inp)
{
    int i, k;
    int max_qp_luma = (4 + 6*(p_Inp->output.bit_depth[0]));
    int max_qp_cr   = (4 + 6*(p_Inp->output.bit_depth[1]));

    for (i = 0; i < (p_Inp->AdaptRoundingFixed ? 1 : imax(max_qp_luma, max_qp_cr)); i++)
    {
        memcpy(&(p_Quant->OffsetList4x4[i][0][0]),&(Offset_intra_default_intra[0]), 16 * sizeof(short));
        for (k = 1; k < 3; k++) // 1,2 (INTRA4X4_CHROMA_INTRA)
          memcpy(&(p_Quant->OffsetList4x4[i][k][0]),&(Offset_intra_flat_chroma[0]),  16 * sizeof(short));
    }
}

int main(int argc, char* argv)
{
    Macroblock currMB;
    Slice p_Slice;
    VideoParameters p_Vid;
    InputParameters p_Inp;
    StorablePicture enc_picture;
    int i, j;
    distblk  min_cost;
    // for transform
    QuantParameters p_Quant;
    int  block_x;
    int  block_y;
    int dummy;

    currMB.p_Slice = &p_Slice;
    currMB.p_Vid = &p_Vid;
    currMB.p_Inp = &p_Inp;
    p_Vid.enc_picture = &enc_picture;
    p_Vid.p_Quant = &p_Quant;
    p_Vid.p_Inp = &p_Inp;
    p_Slice.p_Vid = &p_Vid;

    p_Vid.FrameSizeInMbs = 16;
    p_Vid.PicWidthInMbs = 4;
    p_Vid.width_blk = 16;
    p_Vid.height_blk = 16;
    p_Vid.height = 64;
    p_Vid.width = 64;
    p_Vid.mb_size[IS_LUMA][0] = 16;
    p_Vid.mb_size[IS_LUMA][1] = 16;
    p_Vid.dc_pred_value = 128;
    // for transform
    p_Vid.max_imgpel_value = 255;
    p_Vid.yuv_format = YUV420;
    p_Vid.colour_plane_id = 0;
    p_Vid.bitdepth_luma = 8;
    p_Vid.bitdepth_chroma = 8;
    p_Vid.bitdepth_luma_qp_scale = 0;
    p_Vid.bitdepth_chroma_qp_scale = 0;
    p_Vid.num_blk8x8_uv = 2;

    p_Inp.UseConstrainedIntraPred = 0;
    // for transfrom
    p_Inp.output.bit_depth[0] = 8;
    p_Inp.output.bit_depth[1] = 8;
    p_Inp.AdaptRoundingFixed = 0;

    currMB.pix_x =  0;
    currMB.pix_y =  0;
    currMB.mbAddrX = 0;
    currMB.mbAddrA = 0;
    currMB.mbAvailA = 0;
    currMB.mbAddrB = 0;
    currMB.mbAvailB = 0;
    currMB.mbAddrC = 0;
    currMB.mbAvailC = 0;
    currMB.mbAddrD = 0;
    currMB.mbAvailD = 0;
    currMB.DeblockCall = 0;
    // form transform
    currMB.is_field_mode = 0;
    currMB.ar_mode = I4MB;
    currMB.qp_scaled[PLANE_Y] = 28;
    p_Slice.disthres = 1;
    p_Slice.symbol_mode = CAVLC;
    p_Slice.P444_joined = 0;

    get_mem2Dpel(&currMB.intra4x4_pred, MAX_PLANE, 17);
    get_mem4Dpel(&p_Slice.mpr_4x4, MAX_PLANE, NO_INTRA_PMODE, MB_BLOCK_SIZE, MB_BLOCK_SIZE);

    // currMB->p_Vid->PicPos：宏块左上角像素一维数组，以BlockPos封装携带x，y坐标,以宏块为单位。
    p_Vid.PicPos = calloc(p_Vid.FrameSizeInMbs + 1, sizeof(BlockPos));
    for (j = 0; j < (int) p_Vid.FrameSizeInMbs + 1; j++) {
        p_Vid.PicPos[j].x = (short) (j % p_Vid.PicWidthInMbs);
        p_Vid.PicPos[j].y = (short) (j / p_Vid.PicWidthInMbs);
    }

    get_mem2D((byte***)&(p_Vid.ipredmode), p_Vid.height_blk, p_Vid.width_blk);
    memset(&(p_Vid.ipredmode[0][0]), -1, p_Vid.height_blk * p_Vid.width_blk *sizeof(char));

    get_mem2Dpel(&(p_Vid.pCurImg), p_Vid.height, p_Vid.width);
    imgpel input[16][16] = {
        {43, 216, 254, 249, 251, 254, 254, 253, 251, 252, 254, 254, 254, 254, 254, 253},
	{49, 198, 193, 211, 228, 205, 213, 185, 211, 207, 186, 248, 198, 203, 208, 183},
	{48, 194, 177, 171, 197, 173, 185, 136, 191, 195, 138, 179, 142, 176, 177, 135},
	{46, 214, 225, 169, 177, 189, 198, 160, 203, 208, 177, 165, 173, 196, 191, 156},
	{41, 185, 208, 180, 203, 228, 226, 200, 214, 226, 225, 227, 228, 225, 224, 210},
	{31, 130, 173, 178, 215, 230, 221, 212, 220, 229, 227, 228, 229, 227, 226, 226},
	{29, 119, 194, 216, 211, 213, 219, 222, 225, 223, 220, 219, 218, 218, 218, 218},
	{25, 126, 219, 224, 217, 224, 227, 227, 227, 226, 225, 224, 220, 220, 221, 222},
	{26, 131, 215, 223, 226, 225, 225, 225, 225, 226, 223, 219, 221, 221, 219, 220},
	{30, 136, 216, 226, 223, 224, 225, 225, 224, 221, 217, 221, 222, 219, 220, 226},
	{30, 136, 216, 227, 224, 224, 225, 223, 221, 218, 221, 216, 211, 224, 224, 211},
	{29, 135, 217, 225, 222, 221, 222, 222, 221, 209, 181, 155, 186, 210, 186, 164},
	{29, 134, 216, 224, 226, 230, 230, 227, 206, 177, 146, 113, 149, 162, 147, 150},
	{29, 135, 219, 231, 225, 201, 190, 185, 163, 144, 153, 140, 127, 143, 165, 184},
	{30, 139, 210, 192, 165, 142, 134, 133, 143, 141, 129, 138, 150, 178, 201, 207},
	{30, 125, 166, 145, 144, 154, 132, 111, 118, 161, 175, 180, 204, 214, 213, 209},
    };
    printf("input: \n");
    for (i = 0; i < MB_BLOCK_SIZE; i++) {
        for (j = 0; j < MB_BLOCK_SIZE; j++) {
            p_Vid.pCurImg[i][j] = input[i][j];
            printf("%4d ", p_Vid.pCurImg[i][j]);
        }
        printf("\n");
    }

    // for transform
    get_mem2Dint(&p_Slice.tblk16x16, MB_BLOCK_SIZE, MB_BLOCK_SIZE);
    get_mem4Dint(&p_Slice.cofAC, BLOCK_SIZE + p_Vid.num_blk8x8_uv, BLOCK_SIZE, 2, 65);
    get_mem4Dint(&p_Vid.ARCofAdj4x4, 3, MAXMODE, MB_BLOCK_SIZE, MB_BLOCK_SIZE);

    get_mem3Dpel(&p_Slice.mb_pred, MAX_PLANE, MB_BLOCK_SIZE, MB_BLOCK_SIZE);
    get_mem3Dint(&p_Slice.mb_ores, MAX_PLANE, MB_BLOCK_SIZE, MB_BLOCK_SIZE);
    get_mem3Dint(&p_Slice.mb_rres, MAX_PLANE, MB_BLOCK_SIZE, MB_BLOCK_SIZE);
    get_mem2Dpel(&enc_picture.p_curr_img, p_Vid.height, p_Vid.width);

    allocate_QMatrix(&p_Quant, &p_Inp);
    CalculateQuant4x4Param(&p_Vid);

    allocate_QOffsets(&p_Quant, &p_Inp);
    InitOffsetParam(&p_Quant, &p_Inp);
    CalculateOffset4x4Param(&p_Vid);

    for (i = 0; i < BLOCK_SIZE; i++) {
        for (j = 0; j < BLOCK_SIZE; j++) {
            block_x = ((i & 0x01) << 3) + ((j & 0x01) << 2);
            block_y = ((i >> 1) << 3)  + ((j >> 1) << 2);
            encode_one_I4x4_block(&currMB, i, j, 570, &min_cost);
	    residual_transform_quant_luma_4x4(&currMB, PLANE_Y, block_x, block_y, &dummy, 1);
	}
    }

    printf("output: \n");
    for (i = 0; i < MB_BLOCK_SIZE; i++) {
        for (j = 0; j < MB_BLOCK_SIZE; j++) {
            //printf("%4d ", p_Slice.mb_ores[0][i][j]);
            printf("%4d ", enc_picture.p_curr_img[i][j]);
        }
        printf("\n");
    }
    return 0;
}
