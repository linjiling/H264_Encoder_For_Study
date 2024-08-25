#include <stdio.h>
#include "global.h"
#include "mb_access.h"
/*!
 ************************************************************************
 * \brief
 *    Get the Prediction from the Neighboring Blocks for Number of Nonzero Coefficients
 *
 *    Luma Blocks
 ************************************************************************
 */
static inline int is_intra(Macroblock *curr_MB)
{
  return ((curr_MB)->mb_type==SI4MB || (curr_MB)->mb_type==I4MB || (curr_MB)->mb_type==I16MB || (curr_MB)->mb_type==I8MB || (curr_MB)->mb_type==IPCM);
}

int predict_nnz(Macroblock *currMB, int block_type, int i,int j)
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  PixelPos pix;

  int pred_nnz = 0;
  int cnt      = 0;

  // left block
  get4x4Neighbour(currMB, (i << 2) - 1, (j << 2), p_Vid->mb_size[IS_LUMA], &pix);

  if (is_intra(currMB) && pix.available && p_Vid->active_pps->constrained_intra_pred_flag && ((currSlice->partition_mode != 0) && !currSlice->idr_flag))
  {
    pix.available &= p_Vid->intra_block[pix.mb_addr];
    if (!pix.available)
      cnt++;
  }

  if (pix.available)
  {
    switch (block_type)
    {
    case LUMA:
      pred_nnz += p_Vid->nz_coeff [pix.mb_addr ][pix.x][pix.y];
      cnt++;
      break;
    case CB:
      pred_nnz += p_Vid->nz_coeff [pix.mb_addr ][pix.x][4+pix.y];
      cnt++;
      break;
    case CR:
      pred_nnz += p_Vid->nz_coeff [pix.mb_addr ][pix.x][8+pix.y];
      cnt++;
      break;
    default:
      printf("writeCoeff4x4_CAVLC: Invalid block type\n");
      break;
    }
  }

  // top block
  get4x4Neighbour(currMB, (i<<2), (j<<2) - 1, p_Vid->mb_size[IS_LUMA], &pix);

  if (is_intra(currMB) && pix.available && p_Vid->active_pps->constrained_intra_pred_flag && ((currSlice->partition_mode != 0) && !currSlice->idr_flag))
  {
    pix.available &= p_Vid->intra_block[pix.mb_addr];
    if (!pix.available)
      cnt++;
  }

  if (pix.available)
  {
    switch (block_type)
    {
    case LUMA:
      pred_nnz += p_Vid->nz_coeff [pix.mb_addr ][pix.x][pix.y];
      cnt++;
      break;
    case CB:
      pred_nnz += p_Vid->nz_coeff [pix.mb_addr ][pix.x][4+pix.y];
      cnt++;
      break;
    case CR:
      pred_nnz += p_Vid->nz_coeff [pix.mb_addr ][pix.x][8+pix.y];
      cnt++;
      break;
    default:
      printf("writeCoeff4x4_CAVLC: Invalid block type\n");
      break;
    }
  }

  if (cnt==2)
  {
    pred_nnz++;
    pred_nnz>>=1;
  }

  return pred_nnz;
}


/*!
 ************************************************************************
 * \brief
 *    Get the Prediction from the Neighboring Blocks for Number of Nonzero Coefficients
 *
 *    Chroma Blocks
 ************************************************************************
 */
int predict_nnz_chroma(Macroblock *currMB, int i,int j)
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  PixelPos pix;

  int pred_nnz = 0;
  int cnt      = 0;

  if (p_Vid->yuv_format != YUV444)
  {
    //YUV420 and YUV422
    // left block
    get4x4Neighbour(currMB, ((i & 0x01)<<2) - 1, ((j-4)<<2), p_Vid->mb_size[IS_CHROMA], &pix);

    if (is_intra(currMB) && pix.available && p_Vid->active_pps->constrained_intra_pred_flag && ((currSlice->partition_mode != 0) && !currSlice->idr_flag))
    {
      pix.available &= p_Vid->intra_block[pix.mb_addr];
      if (!pix.available)
        cnt++;
    }

    if (pix.available)
    {
      pred_nnz = p_Vid->nz_coeff [pix.mb_addr ][2 * (i >> 1) + pix.x][4 + pix.y];
      cnt++;
    }

    // top block
    get4x4Neighbour(currMB, ((i & 0x01)<<2), ((j-4)<<2) -1, p_Vid->mb_size[IS_CHROMA],  &pix);

    if (is_intra(currMB) && pix.available && p_Vid->active_pps->constrained_intra_pred_flag && ((currSlice->partition_mode != 0) && !currSlice->idr_flag))
    {
      pix.available &= p_Vid->intra_block[pix.mb_addr];
      if (!pix.available)
        cnt++;
    }

    if (pix.available)
    {
      pred_nnz += p_Vid->nz_coeff [pix.mb_addr ][2 * (i >> 1) + pix.x][4 + pix.y];
      cnt++;
    }
  }


  if (cnt==2)
  {
    pred_nnz++;
    pred_nnz>>=1;
  }

  return pred_nnz;
}
