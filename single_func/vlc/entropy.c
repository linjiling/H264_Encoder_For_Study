#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "global.h"
#include "vlc_base.h"
#include "types.h"
#include "defines.h"
#include "ifunction.h"
#include "block_com.h"
#include "memalloc.h"

const int * assignSE2partition[2] ;
const int assignSE2partition_NoDP[SE_MAX_ELEMENTS] =
  {  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
const int assignSE2partition_DP[SE_MAX_ELEMENTS] =
  // 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17
  {  0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 0, 0, 0 } ;

static inline int is_intra(Macroblock *curr_MB)
{
  return ((curr_MB)->mb_type==SI4MB || (curr_MB)->mb_type==I4MB || (curr_MB)->mb_type==I16MB || (curr_MB)->mb_type==I8MB || (curr_MB)->mb_type==IPCM);
}

 /*!
 ************************************************************************
 * \brief
 *    Writes coeff of an 4x4 block (CAVLC)
 *
 * \output:
 *    currMB->bits: 
 *    
 ************************************************************************
 */
int writeCoeff4x4_CAVLC_normal (Macroblock* currMB, int block_type, int b8, int b4, int param)
{
  Slice* currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currSlice->p_Vid;
  int           no_bits    = 0;
  SyntaxElement se;
  DataPartition *dataPart;
  const int     *partMap;

  int k,level = 1,run = 0, vlcnum;
  int numcoeff = 0, lastcoeff = 0, numtrailingones = 0; 
  int numones = 0, totzeros = 0, zerosleft, numcoef;
  int numcoeff_vlc;
  int code, level_two_or_higher;
  int dptype = 0;
  int nnz, max_coeff_num = 0, cdc = 0, cac = 0;
  int subblock_x, subblock_y;
  int *mb_bits_coeff = &currMB->bits.mb_y_coeff;
#if TRACE
  char type[15];
#endif

  static const int incVlc[] = {0, 3, 6, 12, 24, 48, 32768};  // maximum vlc = 6


  int*  pLevel = NULL;
  int*  pRun = NULL;

  assignSE2partition[0] = assignSE2partition_NoDP;
  assignSE2partition[1] =  assignSE2partition_NoDP;
  partMap   = assignSE2partition[currSlice->partition_mode];

  switch (block_type)
  {
  case LUMA:
    max_coeff_num = 16;

    pLevel = currSlice->cofAC[b8][b4][0];
    pRun   = currSlice->cofAC[b8][b4][1];
#if TRACE
    sprintf(type, "%s", "Luma");
#endif
    dptype = (is_intra (currMB)) ? SE_LUM_AC_INTRA : SE_LUM_AC_INTER;
    break;

  case CHROMA_AC:
    max_coeff_num = 15;
    mb_bits_coeff = &currMB->bits.mb_uv_coeff;
    cac = 1;

    pLevel = currSlice->cofAC[b8][b4][0];
    pRun   = currSlice->cofAC[b8][b4][1];
#if TRACE
    sprintf(type, "%s", "ChrAC");
#endif
    dptype = (is_intra (currMB)) ? SE_CHR_AC_INTRA : SE_CHR_AC_INTER;
    break;

  case CHROMA_DC:
    max_coeff_num = p_Vid->num_cdc_coeff;
    mb_bits_coeff = &currMB->bits.mb_uv_coeff;
    cdc = 1;

    pLevel = currSlice->cofDC[param + 1][0];
    pRun   = currSlice->cofDC[param + 1][1];
#if TRACE
    sprintf(type, "%s", "ChrDC");
#endif
    dptype = (is_intra (currMB)) ? SE_CHR_DC_INTRA : SE_CHR_DC_INTER;
    break;

  case LUMA_INTRA16x16AC:
    max_coeff_num = 15;

    pLevel = currSlice->cofAC[b8][b4][0];
    pRun   = currSlice->cofAC[b8][b4][1];
#if TRACE
    sprintf(type, "%s", "Lum16AC");
#endif
    dptype = SE_LUM_AC_INTRA;
    break;

  case LUMA_INTRA16x16DC:
    max_coeff_num = 16;

    pLevel = currSlice->cofDC[0][0];
    pRun   = currSlice->cofDC[0][1];
#if TRACE
    sprintf(type, "%s", "Lum16DC");
#endif
    dptype = SE_LUM_DC_INTRA;
    break;


  default:
    printf("writeCoeff4x4_CAVLC: Invalid block type\n");
    break;
  }

  dataPart = &(currSlice->partArr[partMap[dptype]]);

  for(k = 0; (k <= ((cdc) ? p_Vid->num_cdc_coeff : 16)) && level != 0; k++)
  {
    level = pLevel[k]; // level
    run   = pRun[k];   // run

    if (level)
    {

      totzeros += run; 
      if (iabs(level) == 1)
      {
        numones ++;
        numtrailingones ++;
        numtrailingones = imin(numtrailingones, 3); // clip to 3
      }
      else
      {
        numtrailingones = 0;
      }
      numcoeff ++;
      lastcoeff = k;
    }
  }

  if (!cdc)
  {
    if (!cac)
    {
      // luma
      subblock_x = ((b8 & 0x1) == 0) ? (((b4 & 0x1) == 0) ? 0 : 1) : (((b4 & 0x1) == 0) ? 2 : 3);
      // horiz. position for coeff_count context
      subblock_y = (b8 < 2) ? ((b4 < 2) ? 0 : 1) : ((b4 < 2) ? 2 : 3);
      // vert.  position for coeff_count context
      nnz = predict_nnz(currMB, LUMA, subblock_x,subblock_y);
    }
    else
    {
      // chroma AC
      subblock_x = param >> 4;
      subblock_y = param & 15;
      nnz = predict_nnz_chroma(currMB, subblock_x, subblock_y);
    }
    p_Vid->nz_coeff [currMB->mbAddrX ][subblock_x][subblock_y] = numcoeff;

    numcoeff_vlc = (nnz < 2) ? 0 : ((nnz < 4) ? 1 : ((nnz < 8) ? 2 : 3));
  }
  else
  {
    // chroma DC (has its own VLC)
    // numcoeff_vlc not relevant
    numcoeff_vlc = 0;

    subblock_x = param;
    subblock_y = param;
  }

  se.type  = dptype;

  se.value1 = numcoeff;
  se.value2 = numtrailingones;
  se.len    = numcoeff_vlc; /* use len to pass vlcnum */

#if TRACE
  snprintf(se.tracestring,
    TRACESTRING_SIZE, "%s # c & tr.1s(%d,%d) vlc=%d #c=%d #t1=%d",
    type, subblock_x, subblock_y, numcoeff_vlc, numcoeff, numtrailingones);
#endif

  if (!cdc)
    writeSyntaxElement_NumCoeffTrailingOnes(&se, dataPart);
  else
    writeSyntaxElement_NumCoeffTrailingOnesChromaDC(p_Vid, &se, dataPart);

  *mb_bits_coeff += se.len;
  no_bits                += se.len;

  if (!numcoeff)
    return no_bits;

  if (numcoeff)
  {
    code = 0;
    for (k = lastcoeff; k > lastcoeff - numtrailingones; k--)
    {
      level = pLevel[k]; // level
#ifdef  _DEBUG
      if (iabs(level) > 1)
      {
        printf("ERROR: level > 1\n");
        exit(-1);
      }
#endif
      code <<= 1;

      code |= (level < 0);
    }

    if (numtrailingones)
    {
      se.type  = dptype;

      se.value2 = numtrailingones;
      se.value1 = code;

#if TRACE
      snprintf(se.tracestring,
        TRACESTRING_SIZE, "%s trailing ones sign (%d,%d)",
        type, subblock_x, subblock_y);
#endif

      writeSyntaxElement_VLC (&se, dataPart);
      *mb_bits_coeff += se.len;
      no_bits                += se.len;

    }

    // encode levels
    level_two_or_higher = (numcoeff > 3 && numtrailingones == 3) ? 0 : 1;

    vlcnum = (numcoeff > 10 && numtrailingones < 3) ? 1 : 0;

    for (k = lastcoeff - numtrailingones; k >= 0; k--)
    {
      level = pLevel[k]; // level

      se.value1 = level;
      se.type  = dptype;

#if TRACE
      snprintf(se.tracestring,
        TRACESTRING_SIZE, "%s lev (%d,%d) k=%d vlc=%d lev=%3d",
        type, subblock_x, subblock_y, k, vlcnum, level);
#endif

      if (level_two_or_higher)
      {
        level_two_or_higher = 0;

        if (se.value1 > 0)
          se.value1 --;
        else
          se.value1 ++;        
      }

      //    encode level

      if (vlcnum == 0)
        writeSyntaxElement_Level_VLC1(&se, dataPart, p_Vid->active_sps->profile_idc);
      else
        writeSyntaxElement_Level_VLCN(&se, vlcnum, dataPart, p_Vid->active_sps->profile_idc);

      // update VLC table
      if (iabs(level) > incVlc[vlcnum])
        vlcnum++;

      if ((k == lastcoeff - numtrailingones) && iabs(level) > 3)
        vlcnum = 2;

      *mb_bits_coeff += se.len;
      no_bits                += se.len;
    }

    // encode total zeroes
    if (numcoeff < max_coeff_num)
    {

      se.type  = dptype;
      se.value1 = totzeros;

      vlcnum = numcoeff - 1;

      se.len = vlcnum;

#if TRACE
      snprintf(se.tracestring,
        TRACESTRING_SIZE, "%s totalrun (%d,%d) vlc=%d totzeros=%3d",
        type, subblock_x, subblock_y, vlcnum, totzeros);
#endif
      if (!cdc)
        writeSyntaxElement_TotalZeros(&se, dataPart);
      else
        writeSyntaxElement_TotalZerosChromaDC(p_Vid, &se, dataPart);

      *mb_bits_coeff += se.len;
      no_bits                += se.len;
    }

    // encode run before each coefficient
    zerosleft = totzeros;
    numcoef = numcoeff;
    for (k = lastcoeff; k >= 0; k--)
    {
      run = pRun[k]; // run

      se.value1 = run;
      se.type   = dptype;

      // for last coeff, run is remaining totzeros
      // when zerosleft is zero, remaining coeffs have 0 run
      if ((!zerosleft) || (numcoeff <= 1 ))
        break;

      if (numcoef > 1 && zerosleft)
      {
        vlcnum = imin(zerosleft - 1, RUNBEFORE_NUM_M1);
        se.len = vlcnum;

#if TRACE
        snprintf(se.tracestring,
          TRACESTRING_SIZE, "%s run (%d,%d) k=%d vlc=%d run=%2d",
          type, subblock_x, subblock_y, k, vlcnum, run);
#endif

        writeSyntaxElement_Run(&se, dataPart);

        *mb_bits_coeff += se.len;
        no_bits                += se.len;

        zerosleft -= run;
        numcoef --;
      }
    }
  }

  return no_bits;
}

int main(int argc, char argv[])
{
    Macroblock currMB;
    Slice p_Slice;
    VideoParameters p_Vid;
    DataPartition partArr[3];
    byte *pstream;
    Bitstream     bitstream;
    pic_parameter_set_rbsp_t active_pps;
    seq_parameter_set_rbsp_t active_sps;
    int i, j, z, x;

    memset(&currMB, 0, sizeof(Macroblock));
    memset(&p_Vid, 0, sizeof(VideoParameters));
    memset(&p_Slice, 0, sizeof(Slice));
    memset(&partArr, 0, sizeof(DataPartition)*3);

    currMB.p_Slice = &p_Slice;
    currMB.p_Vid = &p_Vid;

    currMB.mbAddrA = 0;
    currMB.mbAvailA = 0;
    currMB.mbAddrB = 0;
    currMB.mbAvailB = 0;
    currMB.mbAddrC = 0;
    currMB.mbAvailC = 0;
    currMB.mbAddrD = 0;
    currMB.mbAvailD = 0;
    currMB.DeblockCall = 0;
    currMB.mb_type = I4MB;

    active_sps.profile_idc = 100; 
    active_pps.constrained_intra_pred_flag = FALSE;
    p_Vid.active_pps = &active_pps;
    p_Vid.active_sps = &active_sps;
    p_Vid.num_blk8x8_uv = 2;
    p_Vid.mb_size[IS_LUMA][0] = 16;
    p_Vid.mb_size[IS_LUMA][1] = 16;
    p_Vid.yuv_format = YUV420;
    p_Vid.num_cdc_coeff = 4;
    p_Vid.FrameSizeInMbs = 16;
    p_Vid.PicWidthInMbs = 4;

    p_Slice.partArr = partArr;
    p_Slice.partition_mode = 0;
    p_Slice.p_Vid = &p_Vid;
    p_Slice.idr_flag = TRUE;

    pstream = (byte *)malloc(1024);
    memset(pstream, 0, 1024);
    memset(&bitstream, 0, sizeof(Bitstream));
    bitstream.streamBuffer = pstream;
    bitstream.bits_to_go = 8;
    partArr[0].bitstream = &bitstream;

    get_mem4Dint(&p_Slice.cofAC, BLOCK_SIZE + p_Vid.num_blk8x8_uv, BLOCK_SIZE, 2, 65);
    get_mem3Dint(&p_Slice.cofDC, 3, 2, 18);

    // currMB->p_Vid->PicPos：宏块左上角像素一维数组，以BlockPos封装携带x，y坐标,以宏块为单位。
    p_Vid.PicPos = calloc(p_Vid.FrameSizeInMbs + 1, sizeof(BlockPos));
    for (j = 0; j < (int) p_Vid.FrameSizeInMbs + 1; j++) {
        p_Vid.PicPos[j].x = (short) (j % p_Vid.PicWidthInMbs);
        p_Vid.PicPos[j].y = (short) (j / p_Vid.PicWidthInMbs);
    }
 
    p_Vid.intra_block = (short*) calloc(p_Vid.FrameSizeInMbs, sizeof(short));
    get_mem3Dint(&(p_Vid.nz_coeff), p_Vid.FrameSizeInMbs, 4, 4+p_Vid.num_blk8x8_uv);

    int cofAC[4][4][2][65] = {
       {
          {
             {9, -12, 3, 3, -3, -11, -5, 1, -1, -2, 1},
	     {0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1},
	  },
	  {
	     {3, 2, 2, -1, 2, 1, -2, -1},
	     {0, 0, 1, 1, 0, 0, 0, 4},
	  },
	  {
	     {-4, -6, 2, 3, 7, 2, -2, 1, -1, -1},
	     {0, 0, 1, 0, 0, 0, 0, 1, 1},
	  },
	  {
	     {2, 1, 2, 1, 2, -1, -1, -1},
	     {0, 0, 0, 0, 0, 0, 1, 3},
	  }

       },
       {
          {
	     {5, 1, -2, -3, -2, 1, -2, 1, 1, -2, 1},
	     {0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1},
	  },
	  {
	     {-3, 1, -1, 5, -2, 1, 1, 1},
	     {0, 0, 0, 0, 1, 1, 1, 1},
	  },
	  {
	     {1, 0},
	     {2, 0},
	  },
	  {
	     {-2, 0},
	     {0, 0},
	  },
       },
       {
          {
	     {0, 0},
	     {0, 0},
	  },
          {
	     {0, 0},
	     {0, 0},
	  },
	  {
	     {-2, 2, 3, -2, -3, -1, 1, 1},
	     {0, 0, 0, 0, 0, 0, 1},
	  },
          {
	     {3, -2, 1, -1, -1},
	     {2, 1, 2, 1, 1},
	  },
       },
       {
          {
	     {-2, 2, 2, -1, -2, 1, 1, -1},
	     {0, 0, 0, 0, 0, 3},
	  },
	  {
	     {1, -1, -1, -1, 1, 1, -1},
	     {0, 1, 1, 0, 1},
	  },
	  {
	     {4, -1, -4, 2, -1, -2, 2, -1, 1},
	     {0, 0, 0, 1, 0, 3},
	  },
	  {
	     {4, -1, -2, 2, 1, 2},
	     {0, 0, 0, 0, 0, 3},
	  },
       },
    };
    for (i = 0; i < BLOCK_SIZE; i++) {
        for (j = 0; j < BLOCK_SIZE; j++) {
	    for(z = 0; z < 2; z++) {
	        for (x = 0; x < 65; x++) {
		    p_Slice.cofAC[i][j][z][x] = cofAC[i][j][z][x];
		}
	    } 
	}
    }

    for (i = 0; i < BLOCK_SIZE; i++) {
        for (j = 0; j < BLOCK_SIZE; j++) {
	    writeCoeff4x4_CAVLC_normal(&currMB, LUMA, i, j, 1);
	}
    }

    for (i = 0; i < bitstream.byte_pos; i++) {
        printf("0x%x ", pstream[i]);
    }
    printf("\n");
    return 0;
}
