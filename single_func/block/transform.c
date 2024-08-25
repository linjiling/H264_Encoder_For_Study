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
