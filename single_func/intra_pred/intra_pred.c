#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "intra_pred4x4.h"
#include "mb_access.h"
#include "ifunctions.h"
#include "memalloc.h"
#include "rdopt.h"

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

int main(int argc, char* argv)
{
    Macroblock currMB;
    Slice p_Slice;
    VideoParameters p_Vid;
    InputParameters p_Inp;
    StorablePicture enc_picture;
    int i, j;
    distblk  min_cost;

    currMB.p_Slice = &p_Slice;
    currMB.p_Vid = &p_Vid;
    currMB.p_Inp = &p_Inp;
    p_Vid.enc_picture = &enc_picture;

    p_Vid.FrameSizeInMbs = 16;
    p_Vid.PicWidthInMbs = 4;
    p_Vid.width_blk = 16;
    p_Vid.height_blk = 16;
    p_Vid.height = 64;
    p_Vid.width = 64;
    p_Vid.mb_size[IS_LUMA][0] = 16;
    p_Vid.mb_size[IS_LUMA][1] = 16;
    p_Vid.dc_pred_value = 128;
    p_Inp.UseConstrainedIntraPred = 0;
    currMB.pix_x =  0;
    currMB.pix_y =  0;
    currMB.mbAddrA = 0;
    currMB.mbAvailA = 0;
    currMB.mbAddrB = 0;
    currMB.mbAvailB = 0;
    currMB.mbAddrC = 0;
    currMB.mbAvailC = 0;
    currMB.mbAddrD = 0;
    currMB.mbAvailD = 0;
    currMB.DeblockCall = 0;

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
    for (i = 0; i < p_Vid.height; i++) {
        for (j = 0; j < p_Vid.height; j++) {
            p_Vid.pCurImg[i][j] = rand() % 32;
            printf("%d ", p_Vid.pCurImg[i][j]);
        }
        printf("\n");
    }

    get_mem3Dpel(&p_Slice.mb_pred, MAX_PLANE, MB_BLOCK_SIZE, MB_BLOCK_SIZE);
    get_mem3Dint(&p_Slice.mb_ores, MAX_PLANE, MB_BLOCK_SIZE, MB_BLOCK_SIZE);

    get_mem2Dpel(&enc_picture.p_curr_img, p_Vid.height, p_Vid.width);

    encode_one_I4x4_block(&currMB, 0, 0, 570, &min_cost);

    printf("output !!!!!!!!!!!!!!!!\n");
    for (i = 0; i < MB_BLOCK_SIZE; i++) {
        for (j = 0; j < MB_BLOCK_SIZE; j++) {
            printf("%d ", p_Slice.mb_ores[0][i][j]);
        }
        printf("\n");
    }
    return 0;
}
