#include <string.h>
#include "global.h"

/*!
 *************************************************************************************
 * \brief
 *    Copy ImgPel Data from one structure to another (4x4)
 *************************************************************************************
 */
void copy_image_data_4x4(imgpel  **imgBuf1, imgpel  **imgBuf2, int off1, int off2)
{
  int j;
  for(j = 0; j < BLOCK_SIZE; j++)
  {
    memcpy(&imgBuf1[j][off1], &imgBuf2[j][off2], BLOCK_SIZE * sizeof (imgpel));
  }
}
