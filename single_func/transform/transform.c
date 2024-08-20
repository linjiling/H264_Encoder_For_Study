#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "defines.h"
#include "types.h"
#include "ifunctions.h"
#include "md_common.h"
#include "blk_prediction.h"
#include "memalloc.h"

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

void forward4x4(int **block, int **tblock, int pos_y, int pos_x)
{
  int i, ii;  
  int tmp[16];
  int *pTmp = tmp, *pblock;
  int p0,p1,p2,p3;
  int t0,t1,t2,t3;

  // Horizontal
  for (i=pos_y; i < pos_y + BLOCK_SIZE; i++)
  {
    pblock = &block[i][pos_x];
    p0 = *(pblock++);
    p1 = *(pblock++);
    p2 = *(pblock++);
    p3 = *(pblock  );

    t0 = p0 + p3;
    t1 = p1 + p2;
    t2 = p1 - p2;
    t3 = p0 - p3;

    *(pTmp++) =  t0 + t1;
    *(pTmp++) = (t3 << 1) + t2;
    *(pTmp++) =  t0 - t1;    
    *(pTmp++) =  t3 - (t2 << 1);
  }

  // Vertical 
  for (i=0; i < BLOCK_SIZE; i++)
  {
    pTmp = tmp + i;
    p0 = *pTmp;
    p1 = *(pTmp += BLOCK_SIZE);
    p2 = *(pTmp += BLOCK_SIZE);
    p3 = *(pTmp += BLOCK_SIZE);

    t0 = p0 + p3;
    t1 = p1 + p2;
    t2 = p1 - p2;
    t3 = p0 - p3;

    ii = pos_x + i;
    tblock[pos_y    ][ii] = t0 +  t1;
    tblock[pos_y + 1][ii] = t2 + (t3 << 1);
    tblock[pos_y + 2][ii] = t0 -  t1;
    tblock[pos_y + 3][ii] = t3 - (t2 << 1);
  }
}

void inverse4x4(int **tblock, int **block, int pos_y, int pos_x)
{
  int i, ii;  
  int tmp[16];
  int *pTmp = tmp, *pblock;
  int p0,p1,p2,p3;
  int t0,t1,t2,t3;

  // Horizontal
  for (i = pos_y; i < pos_y + BLOCK_SIZE; i++)
  {
    pblock = &tblock[i][pos_x];
    t0 = *(pblock++);
    t1 = *(pblock++);
    t2 = *(pblock++);
    t3 = *(pblock  );

    p0 =  t0 + t2;
    p1 =  t0 - t2;
    p2 = (t1 >> 1) - t3;
    p3 =  t1 + (t3 >> 1);

    *(pTmp++) = p0 + p3;
    *(pTmp++) = p1 + p2;
    *(pTmp++) = p1 - p2;
    *(pTmp++) = p0 - p3;
  }

  //  Vertical 
  for (i = 0; i < BLOCK_SIZE; i++)
  {
    pTmp = tmp + i;
    t0 = *pTmp;
    t1 = *(pTmp += BLOCK_SIZE);
    t2 = *(pTmp += BLOCK_SIZE);
    t3 = *(pTmp += BLOCK_SIZE);

    p0 = t0 + t2;
    p1 = t0 - t2;
    p2 =(t1 >> 1) - t3;
    p3 = t1 + (t3 >> 1);

    ii = i + pos_x;
    block[pos_y    ][ii] = p0 + p3;
    block[pos_y + 1][ii] = p1 + p2;
    block[pos_y + 2][ii] = p1 - p2;
    block[pos_y + 3][ii] = p0 - p3;
  }
}

/*!
 ************************************************************************
 * \brief
 *    Quantization process for All coefficients for a 4x4 block
 *
 ************************************************************************
 */
int quant_4x4_normal(Macroblock *currMB, int **tblock, struct quant_methods *q_method)
{
  VideoParameters *p_Vid = currMB->p_Vid;
  QuantParameters *p_Quant = p_Vid->p_Quant;
  Slice *currSlice = currMB->p_Slice;
  Boolean is_cavlc = (Boolean) (currSlice->symbol_mode == CAVLC);

  int   block_x = q_method->block_x;
  int  qp = q_method->qp;
  int*  ACL = &q_method->ACLevel[0];
  int*  ACR = &q_method->ACRun[0];  
  LevelQuantParams **q_params_4x4 = q_method->q_params;
  const byte (*pos_scan)[2] = q_method->pos_scan;
  const byte *c_cost = q_method->c_cost;
  int *coeff_cost = q_method->coeff_cost;

  
  LevelQuantParams *q_params = NULL;
  int i,j, coeff_ctr;

  int *m7;
  int scaled_coeff;

  int   level, run = 0;
  int   nonzero = FALSE;
  int   qp_per = p_Quant->qp_per_matrix[qp];
  int   q_bits = Q_BITS + qp_per;
  const byte *p_scan = &pos_scan[0][0];

  // Quantization
  for (coeff_ctr = 0; coeff_ctr < 16; ++coeff_ctr)
  {
    i = *p_scan++;  // horizontal position
    j = *p_scan++;  // vertical position

    m7 = &tblock[j][block_x + i];

    if (*m7 != 0)
    {
      q_params = &q_params_4x4[j][i];
      scaled_coeff = iabs (*m7) * q_params->ScaleComp;
      level = (scaled_coeff + q_params->OffsetComp) >> q_bits;

      if (level != 0)
      {
        if (is_cavlc)
          level = imin(level, CAVLC_LEVEL_LIMIT);

        *coeff_cost += (level > 1) ? MAX_VALUE : c_cost[run];

        level  = isignab(level, *m7);
        *m7     = rshift_rnd_sf(((level * q_params->InvScaleComp) << qp_per), 4);
        // inverse scale can be alternative performed as follows to ensure 16bit
        // arithmetic is satisfied.
        // *m7 = (qp_per<4) ? rshift_rnd_sf((level*q_params->InvScaleComp),4-qp_per) : (level*q_params->InvScaleComp)<<(qp_per-4);
        *ACL++ = level;
        *ACR++ = run; 
        // reset zero level counter
        run    = 0;
        nonzero = TRUE;        
      }
      else
      {
        *m7 = 0;
        ++run;
      }
    }
    else
    {
      ++run;
    } 
  }

  *ACL = 0;

  return nonzero;
}

static inline int check_zero(int **mb_ores, int block_x)
{
  int i, j, k = 0;

  for (j = 0; (j < BLOCK_SIZE) && (k == 0); ++j)
  {
    for (i = block_x; (i< block_x + BLOCK_SIZE) && (k == 0); ++i)
    {
      //k |= (mb_ores[j][i] != 0);
      k |= mb_ores[j][i];
    }
  }
  return k;
}

/*!
************************************************************************
* \brief
*    The routine performs transform,quantization,inverse transform, 
*    adds the diff to the prediction and writes the result to the 
*    decoded luma frame. 
*
* \par Input:
*    currMB:          Current macroblock.
*    pl:              Color plane for 4:4:4 coding.
*    block_x,block_y: Block position inside a macro block (0,4,8,12).
*    intra:           Intra block indicator.
*
* \par Output_
*    nonzero:         0 if no levels are nonzero. \n
*                     1 if there are nonzero levels.\n
*    coeff_cost:      Coeff coding cost for thresholding consideration.\n
************************************************************************
*/
int residual_transform_quant_luma_4x4(Macroblock *currMB, ColorPlane pl, int block_x,int block_y, int *coeff_cost, int intra)
{
  int nonzero = FALSE;

  int   pos_x   = block_x >> BLOCK_SHIFT;
  int   pos_y   = block_y >> BLOCK_SHIFT;
  int   b8      = 2*(pos_y >> 1) + (pos_x >> 1) + (pl<<2);
  int   b4      = 2*(pos_y & 0x01) + (pos_x & 0x01);
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currSlice->p_Vid;

  imgpel **img_enc = p_Vid->enc_picture->p_curr_img;
  imgpel **mb_pred = currSlice->mb_pred[pl];
  int    **mb_ores = currSlice->mb_ores[pl];  

  if (check_zero(&mb_ores[block_y], block_x) != 0) // check if any coefficients in block
  {
    int   **mb_rres = currSlice->mb_rres[pl];   
    int   max_imgpel_value = p_Vid->max_imgpel_value;
    int   qp = (p_Vid->yuv_format==YUV444 && !currSlice->P444_joined)? currMB->qp_scaled[(int)(p_Vid->colour_plane_id)]: currMB->qp_scaled[pl]; 
    QuantParameters   *p_Quant = p_Vid->p_Quant;
    QuantMethods quant_methods;
    quant_methods.ACLevel = currSlice->cofAC[b8][b4][0];
    quant_methods.ACRun   = currSlice->cofAC[b8][b4][1];

    quant_methods.block_x    = block_x;
    quant_methods.block_y    = block_y;
    quant_methods.qp         = qp;
    quant_methods.q_params   = p_Quant->q_params_4x4[pl][intra][qp]; 
    quant_methods.fadjust    = p_Vid->AdaptiveRounding ? (&p_Vid->ARCofAdj4x4[pl][currMB->ar_mode][block_y]) : NULL;
    quant_methods.coeff_cost = coeff_cost;
    quant_methods.pos_scan   = currMB->is_field_mode ? FIELD_SCAN : SNGL_SCAN;    
    quant_methods.c_cost     = COEFF_COST4x4[currSlice->disthres];

    currMB->subblock_x = ((b8&0x1)==0) ? (((b4&0x1)==0)? 0: 4) : (((b4&0x1)==0)? 8: 12); // horiz. position for coeff_count context
    currMB->subblock_y = (b8<2)        ? ((b4<2)       ? 0: 4) : ((b4<2)       ? 8: 12); // vert.  position for coeff_count context

    //  Forward 4x4 transform
    forward4x4(mb_ores, currSlice->tblk16x16, block_y, block_x);

    // Quantization process
    nonzero = quant_4x4_normal(currMB, &currSlice->tblk16x16[block_y], &quant_methods);

    //  Decoded block moved to frame memory
    if (nonzero)
    {
      // Inverse 4x4 transform
      inverse4x4(currSlice->tblk16x16, mb_rres, block_y, block_x);

      // generate final block
      sample_reconstruct (&img_enc[currMB->pix_y + block_y], &mb_pred[block_y], &mb_rres[block_y], block_x, currMB->pix_x + block_x, BLOCK_SIZE, BLOCK_SIZE, max_imgpel_value, DQ_BITS);
    }
    else // if (nonzero) => No transformed residual. Just use prediction.
    {
      copy_image_data_4x4(&img_enc[currMB->pix_y + block_y], &mb_pred[block_y], currMB->pix_x + block_x, block_x);
    }
  }
  else
  {
    currSlice->cofAC[b8][b4][0][0] = 0;
    copy_image_data_4x4(&img_enc[currMB->pix_y + block_y], &mb_pred[block_y], currMB->pix_x + block_x, block_x);
  }

  return nonzero;
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

int main(int argc, char* argv[])
{
    Macroblock currMB;
    Slice p_Slice;
    VideoParameters p_Vid;
    StorablePicture enc_picture;
    QuantParameters p_Quant;
    InputParameters p_Inp;
    int  block_x = 0;
    int  block_y = 0;
    int dummy;
    int i, j;
    
    currMB.p_Slice = &p_Slice;
    currMB.p_Vid = &p_Vid;
    p_Vid.enc_picture = &enc_picture;
    p_Vid.p_Quant = &p_Quant;
    p_Vid.p_Inp = &p_Inp;
    p_Slice.p_Vid = &p_Vid;

    p_Vid.height = 64;
    p_Vid.width = 64;
    currMB.pix_x =  0;
    currMB.pix_y =  0;
    p_Inp.output.bit_depth[0] = 8;
    p_Inp.output.bit_depth[1] = 8;
    p_Vid.max_imgpel_value = 255;
    p_Vid.yuv_format = YUV420;
    p_Slice.P444_joined = 0;
    p_Vid.colour_plane_id = 0;
    currMB.is_field_mode = 0;
    currMB.ar_mode = I4MB;
    p_Slice.disthres = 1;
    p_Slice.symbol_mode = CAVLC;
    currMB.qp_scaled[PLANE_Y] = 28;
    p_Vid.bitdepth_luma = 8;
    p_Vid.bitdepth_chroma = 8;
    p_Inp.AdaptRoundingFixed = 0;
    p_Vid.bitdepth_luma_qp_scale = 0;
    p_Vid.bitdepth_chroma_qp_scale = 0;
    p_Vid.num_blk8x8_uv = 2;

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

    for (i = 0; i < MB_BLOCK_SIZE; i++) {
        for (j = 0; j < MB_BLOCK_SIZE; j++) {
            p_Slice.mb_pred[0][i][j] = 128;
        }
    }

    int tmp[4][4] = {{-85, 88, 126, 121},
	           {-79, 70, 65, 83},
		   {-80, 66, 49, 43},
		   {-82, 86, 97, 41}};
    for (i = 0; i < BLOCK_SIZE; i++) {
        for (j = 0; j < BLOCK_SIZE; j++) {
            p_Slice.mb_ores[0][i][j] = tmp[i][j];
            printf("%4d ", p_Slice.mb_ores[0][i][j]);
        }
        printf("\n");
    }

    residual_transform_quant_luma_4x4(&currMB, PLANE_Y, block_x, block_y, &dummy, 1);

    printf("output:\n");
    for (i = 0; i < MB_BLOCK_SIZE; i++) {
        for (j = 0; j < MB_BLOCK_SIZE; j++) {
            printf("%4d ", enc_picture.p_curr_img[i][j]);
        }
        printf("\n");
    }
    return 0;
}
