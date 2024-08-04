#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "global.h"
#include "ifunction.h"

/*!
 ************************************************************************
 * \brief
 *    mapping for ue(v) syntax elements
 * \param ue
 *    value to be mapped
 * \param dummy
 *    dummy parameter
 * \param info
 *    returns mapped value
 * \param len
 *    returns mapped value length
 ************************************************************************
 */
static void ue_linfo(int ue, int dummy, int *len,int *info)
{
  int i, nn =(ue + 1) >> 1;

  for (i=0; i < 33 && nn != 0; i++)
  {
    nn >>= 1;
  }
  *len  = (i << 1) + 1;
  *info = ue + 1 - (1 << i);
}


/*!
 ************************************************************************
 * \brief
 *    mapping for se(v) syntax elements
 * \param se
 *    value to be mapped
 * \param dummy
 *    dummy parameter
 * \param len
 *    returns mapped value length
 * \param info
 *    returns mapped value
 ************************************************************************
 */
static void se_linfo(int se, int dummy, int *len,int *info)
{  
  int sign = (se <= 0) ? 1 : 0;
  int n = iabs(se) << 1;   //  n+1 is the number in the code table.  Based on this we find length and info
  int nn = (n >> 1);
  int i;
  for (i = 0; i < 33 && nn != 0; i++)
  {
    nn >>= 1;
  }
  *len  = (i << 1) + 1;
  *info = n - (1 << i) + sign;
}

/*!
 ************************************************************************
 * \brief
 *    Makes code word and passes it back
 *    A code word has the following format: 0 0 0 ... 1 Xn ...X2 X1 X0.
 *
 * \par Input:
 *    Info   : Xn..X2 X1 X0                                             \n
 *    Length : Total number of bits in the codeword
 ************************************************************************
 */
 // NOTE this function is called with sym->inf > (1<<(sym->len >> 1)).  The upper bits of inf are junk
int symbol2uvlc(SyntaxElement *sym)
{
  int suffix_len = sym->len >> 1;
  //assert (suffix_len < 32);
  suffix_len = (1 << suffix_len);
  sym->bitpattern = suffix_len | (sym->inf & (suffix_len - 1));
  return 0;
}

/*!
 ************************************************************************
 * \brief
 *    writes UVLC code to the appropriate buffer
 ************************************************************************
 */
static void  writeUVLC2buffer(SyntaxElement *se, Bitstream *currStream)
{
  unsigned int mask = 1 << (se->len - 1);
  byte *byte_buf  = &currStream->byte_buf;
  int *bits_to_go = &currStream->bits_to_go;
  int i;

  // Add the new bits to the bitstream.
  // Write out a byte if it is full
  if ( se->len < 33 )
  {
    for (i = 0; i < se->len; i++)
    {
      *byte_buf <<= 1;

      if (se->bitpattern & mask)
        *byte_buf |= 1;

      mask >>= 1;

      if ((--(*bits_to_go)) == 0)
      {
        *bits_to_go = 8;
        currStream->streamBuffer[currStream->byte_pos++] = *byte_buf;
        *byte_buf = 0;
      }
    }
  }
  else
  {
    // zeros
    for (i = 0; i < (se->len - 32); i++)
    {
      *byte_buf <<= 1;

      if ((--(*bits_to_go)) == 0)
      {
        *bits_to_go = 8;
        currStream->streamBuffer[currStream->byte_pos++] = *byte_buf;
        *byte_buf = 0;
      }
    }
    // actual info
    mask = (unsigned int) 1 << 31;
    for (i = 0; i < 32; i++)
    {
      *byte_buf <<= 1;

      if (se->bitpattern & mask)
        *byte_buf |= 1;

      mask >>= 1;

      if ((--(*bits_to_go)) == 0)
      {
        *bits_to_go = 8;
        currStream->streamBuffer[currStream->byte_pos++] = *byte_buf;
        *byte_buf = 0;
      }
    }
  }
}

/*!
 *************************************************************************************
 * \brief
 *    write_ue_v, writes an ue(v) syntax element, returns the length in bits
 *
 * \param tracestring
 *    the string for the trace file
 * \param value
 *    the value to be coded
 *  \param bitstream
 *    the target bitstream the value should be coded into
 *
 * \return
 *    Number of bits used by the coded syntax element
 *
 * \ note
 *    This function writes always the bit buffer for the progressive scan flag, and
 *    should not be used (or should be modified appropriately) for the interlace crap
 *    When used in the context of the Parameter Sets, this is obviously not a
 *    problem.
 *
 *************************************************************************************
 */
int write_ue_v (char *tracestring, int value, Bitstream *bitstream)
{
  SyntaxElement symbol, *sym=&symbol;
  sym->value1 = value;
  sym->value2 = 0;

  ue_linfo(sym->value1,sym->value2,&(sym->len),&(sym->inf));
  symbol2uvlc(sym);

  writeUVLC2buffer (sym, bitstream);

  return (sym->len);
}


/*!
 *************************************************************************************
 * \brief
 *    write_se_v, writes an se(v) syntax element, returns the length in bits
 *
 * \param tracestring
 *    the string for the trace file
 * \param value
 *    the value to be coded
 *  \param bitstream
 *    the target bitstream the value should be coded into
 *
 * \return
 *    Number of bits used by the coded syntax element
 *
 * \ note
 *    This function writes always the bit buffer for the progressive scan flag, and
 *    should not be used (or should be modified appropriately) for the interlace crap
 *    When used in the context of the Parameter Sets, this is obviously not a
 *    problem.
 *
 *************************************************************************************
 */
int write_se_v (char *tracestring, int value, Bitstream *bitstream)
{
  SyntaxElement symbol, *sym=&symbol;
  sym->value1 = value;
  sym->value2 = 0;

  se_linfo(sym->value1,sym->value2,&(sym->len),&(sym->inf));
  symbol2uvlc(sym);

  writeUVLC2buffer (sym, bitstream);

  return (sym->len);
}


/*!
 *************************************************************************************
 * \brief
 *    write_u_1, writes a flag (u(1) syntax element, returns the length in bits,
 *    always 1
 *
 * \param tracestring
 *    the string for the trace file
 * \param value
 *    the value to be coded
 *  \param bitstream
 *    the target bitstream the value should be coded into
 *
 * \return
 *    Number of bits used by the coded syntax element (always 1)
 *
 * \ note
 *    This function writes always the bit buffer for the progressive scan flag, and
 *    should not be used (or should be modified appropriately) for the interlace crap
 *    When used in the context of the Parameter Sets, this is obviously not a
 *    problem.
 *
 *************************************************************************************
 */
Boolean write_u_1 (char *tracestring, int value, Bitstream *bitstream)
{
  SyntaxElement symbol, *sym=&symbol;

  sym->bitpattern = value;
  sym->value1 = value;
  sym->len = 1;

  writeUVLC2buffer(sym, bitstream);

  return ((Boolean) sym->len);
}


/*!
 *************************************************************************************
 * \brief
 *    write_u_v, writes a n bit fixed length syntax element, returns the length in bits,
 *
 * \param n
 *    length in bits
 * \param tracestring
 *    the string for the trace file
 * \param value
 *    the value to be coded
 *  \param bitstream
 *    the target bitstream the value should be coded into
 *
 * \return
 *    Number of bits used by the coded syntax element
 *
 * \ note
 *    This function writes always the bit buffer for the progressive scan flag, and
 *    should not be used (or should be modified appropriately) for the interlace crap
 *    When used in the context of the Parameter Sets, this is obviously not a
 *    problem.
 *
 *************************************************************************************
 */

int write_u_v (int n, char *tracestring, int value, Bitstream *bitstream)
{
  SyntaxElement symbol, *sym=&symbol;

  sym->bitpattern = value;
  sym->value1 = value;
  sym->len = n;  

  writeUVLC2buffer(sym, bitstream);


  return (sym->len);
}


int main(int argc, char *argv[])
{
    byte *pstream = (byte *)malloc(1024);
    Bitstream *bs_ctx = (Bitstream *)malloc(sizeof(Bitstream));
    int i;

    memset(pstream, 0, 1024);
    memset(bs_ctx, 0, sizeof(Bitstream));
    bs_ctx->bits_to_go = 8;
    bs_ctx->streamBuffer = pstream;

    write_u_v(8, NULL, 32, bs_ctx);
    write_u_1(NULL, 1, bs_ctx);
    write_u_1(NULL, 1, bs_ctx);
    write_u_1(NULL, 1, bs_ctx);
    write_u_1(NULL, 1, bs_ctx);
    write_u_1(NULL, 1, bs_ctx);
    write_u_1(NULL, 1, bs_ctx);
    write_u_1(NULL, 1, bs_ctx);
    write_u_1(NULL, 1, bs_ctx);
    write_ue_v(NULL, 5, bs_ctx);
    write_se_v(NULL, 3, bs_ctx);

    for (i = 0; i < 10; i++) {
   	printf("0x%x ", pstream[i]);
    }

    printf("\n");

    return 0;
}
