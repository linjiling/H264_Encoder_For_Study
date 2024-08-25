#include "rdopt.h"
#include "ifunctions.h"

distblk compute_sad4x4_cost(VideoParameters *p_Vid, imgpel **cur_img, imgpel **prd_img, int pic_opix_x, distblk min_cost)
{
  imgpel *cur_line, *prd_line;
  int i32Cost = 0;  
  int imin_cost = dist_down(min_cost);
  
  int j;
  for (j = 0; j < BLOCK_SIZE; j++)
  {
    cur_line = &cur_img[j][pic_opix_x];
    prd_line = prd_img[j];

    i32Cost += iabs(cur_line[0] - prd_line[0]);
    i32Cost += iabs(cur_line[1] - prd_line[1]);
    i32Cost += iabs(cur_line[2] - prd_line[2]);
    i32Cost += iabs(cur_line[3] - prd_line[3]);

    if (i32Cost > imin_cost)
    {
      return(min_cost);
    }
  }
  return dist_scale(i32Cost);
}
