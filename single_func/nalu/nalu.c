#include <assert.h>
#include <stdio.h>

#include "global.h"
#include "typedefs.h"
#include "defines.h"
#include "nalucommon.h"

/*!
 ************************************************************************
 * \brief
 *    Converts String Of Data Bits (SODB) to Raw Byte Sequence
 *    Packet (RBSP)
 * \param currStream
 *        Bitstream which contains data bits.
 * \return None
 * \note currStream is byte-aligned at the end of this function
 *
 ************************************************************************
*/

void SODBtoRBSP(Bitstream *currStream)
{
  currStream->byte_buf <<= 1;
  currStream->byte_buf |= 1;
  currStream->bits_to_go--;
  currStream->byte_buf <<= currStream->bits_to_go;
  currStream->streamBuffer[currStream->byte_pos++] = currStream->byte_buf;
  currStream->bits_to_go = 8;
  currStream->byte_buf = 0;
}


/*!
************************************************************************
*  \brief
*     This function add emulation_prevention_three_byte for all occurrences
*     of the following byte sequences in the stream
*       0x000000  -> 0x00000300
*       0x000001  -> 0x00000301
*       0x000002  -> 0x00000302
*       0x000003  -> 0x00000303
*
*  \param NaluBuffer
*            pointer to target buffer
*  \param rbsp
*            pointer to source buffer
*  \param rbsp_size
*           Size of source
*  \return
*           Size target buffer after emulation prevention.
*
************************************************************************
*/

int RBSPtoEBSP(byte *NaluBuffer, unsigned char *rbsp, int rbsp_size)
{
  int j     = 0;
  int count = 0;
  int i;

  for(i = 0; i < rbsp_size; i++)
  {
    if(count == ZEROBYTES_SHORTSTARTCODE && !(rbsp[i] & 0xFC))
    {
      NaluBuffer[j] = 0x03;
      j++;
      count = 0;
    }
    NaluBuffer[j] = rbsp[i];
    if(rbsp[i] == 0x00)
      count++;
    else
      count = 0;
    j++;
  }

  return j;
}

/*!
 *************************************************************************************
 * \brief
 *    Converts an RBSP to a NALU
 *
 * \param rbsp
 *    byte buffer with the rbsp
 * \param nalu
 *    nalu structure to be filled
 * \param rbsp_size
 *    size of the rbsp in bytes
 * \param nal_unit_type
 *    as in JVT doc
 * \param nal_reference_idc
 *    as in JVT doc
 * \param UseAnnexbLongStartcode
 *    some incomprehensible CABAC stuff
 * \param UseAnnexbLongStartcode
 *    when 1 and when using AnnexB bytestreams, then use a long startcode prefix
 *
 * \return
 *    length of the NALU in bytes
 *************************************************************************************
 */

int RBSPtoNALU (unsigned char *rbsp, NALU_t *nalu, int rbsp_size, int nal_unit_type, int nal_reference_idc, int UseAnnexbLongStartcode)
{
  int len;

  assert (nalu != NULL);
  assert (nal_reference_idc <=3 && nal_reference_idc >=0);
  assert (nal_unit_type > 0 && nal_unit_type <= NALU_TYPE_FILL);
  assert (rbsp_size < MAXRBSPSIZE);

  nalu->startcodeprefix_len = UseAnnexbLongStartcode ? 4 : 3;
  nalu->forbidden_bit       = 0;
  nalu->nal_reference_idc   = (NalRefIdc) nal_reference_idc;
  nalu->nal_unit_type       = (NaluType) nal_unit_type;

  len = RBSPtoEBSP (nalu->buf, rbsp, rbsp_size);
  nalu->len = len;

  return len;
}

int main(int argc, char *argv[])
{

   return 0;
}
