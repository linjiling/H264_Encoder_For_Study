#include "global.h"
#include "mb_access.h"

/*!
 ************************************************************************
 * \brief
 *    get neighbouring positions for non-aff coding
 * \param currMB
 *   current macroblock
 * \param xN
 *    input x position
 * \param yN
 *    input y position
 * \param mb_size
 *    Macroblock size in pixel (according to luma or chroma MB access)
 * \param pix
 *    returns position informations
 ************************************************************************
 */
void getNonAffNeighbour(Macroblock *currMB, int xN, int yN, int mb_size[2], PixelPos *pix)
{
  BlockPos *PicPos = currMB->p_Vid->PicPos;
  if (xN < 0)
  {
    if (yN < 0)
    {
      pix->mb_addr   = currMB->mbAddrD;
      pix->available = currMB->mbAvailD;
    }
    else if ((yN >= 0)&&(yN < mb_size[1]))
    {
      pix->mb_addr   = currMB->mbAddrA;
      pix->available = currMB->mbAvailA;
    }
    else
    {
      pix->available = FALSE;
    }
  }
  else if ((xN >= 0)&&(xN < mb_size[0]))
  {
    if (yN<0)
    {
      pix->mb_addr   = currMB->mbAddrB;
      pix->available = currMB->mbAvailB;
    }
    else if (((yN >= 0)&&(yN < mb_size[1])))
    {
      pix->mb_addr   = currMB->mbAddrX;
      pix->available = TRUE;
    }
    else
    {
      pix->available = FALSE;
    }
  }
  else if ((xN >= mb_size[0])&&(yN < 0))
  {
    pix->mb_addr   = currMB->mbAddrC;
    pix->available = currMB->mbAvailC;
  }
  else
  {
    pix->available = FALSE;
  }

  if (pix->available || currMB->DeblockCall)
  {
    pix->x     = (short) (xN & (mb_size[0] - 1));
    pix->y     = (short) (yN & (mb_size[1] - 1)); 
    pix->pos_x = (short) (pix->x + PicPos[ pix->mb_addr ].x * mb_size[0]);    
    pix->pos_y = (short) (pix->y + PicPos[ pix->mb_addr ].y * mb_size[1]);
  }
}

/*!
 ************************************************************************
 * \brief
 *    get neighboring 4x4 block
 * \param currMB
 *   current macroblock
 * \param block_x
 *    input x block position
 * \param block_y
 *    input y block position
 * \param mb_size
 *    Macroblock size in pixel (according to luma or chroma MB access)
 * \param pix
 *    returns position informations
 ************************************************************************
 */
void get4x4Neighbour (Macroblock *currMB, int block_x, int block_y, int mb_size[2], PixelPos *pix)
{
  getNonAffNeighbour(currMB, block_x, block_y, mb_size, pix);

  if (pix->available)
  {
    pix->x >>= 2;
    pix->y >>= 2;
    pix->pos_x >>= 2;
    pix->pos_y >>= 2;
  }
}
