#include <stdio.h>
#include <stdlib.h>

#define PIC_WIDTH 16 //图像宽度
#define PIC_HIGHT 16 //图像高度

#define BLK_WIDTH 4 //当前块宽度
#define BLK_HIGHT 4 //当前块高度

#define REF_POS_X 2 //参考块左上角像素点G的横坐标
#define REF_POS_Y 2 //参考块左上角像素点G的纵坐标

#define MAX_PIX_VALUE 32 //像素点最大像素值

typedef char imgpel;

static inline int imin(int a, int b)
{
  return ((a) < (b)) ? (a) : (b);
}

static inline int imax(int a, int b)
{
  return ((a) > (b)) ? (a) : (b);
}

static inline int iClip1(int high, int x)
{
  x = imax(x, 0);
  x = imin(x, high);

  return x;
}

/* 该函数以MVf的值命名，因为所有MVf相同的帧内差值过程都是一样的，只是使用的整数像素点不一样而已 */
/* 参数说明：                                                                            */
/* (1) block: 当前块左上角P0像素地址                                                      */
/* (2) cur_imgY: 参考块左上角G像素相对与整幅图像所在行地址                                  */
/* (3) block_size_y: 当前块高度                                                          */
/* (4) block_size_y: 当前块宽度                                                          */
/* (5) x_pos: 参考块左上角G像素的横坐标x                                                  */
/* (6) shift_x: 整个图像Buffer的宽度，注意这里不一定和图像宽度一致                          */
/* (7) max_imgpel_value: 像素值最大值                                                    */
static void get_luma_13(imgpel **block, imgpel **cur_imgY, int block_size_y, int block_size_x, int x_pos, int shift_x, int max_imgpel_value)
{
  /* Diagonal interpolation */
  int i, j;
  imgpel *p0, *p1, *p2, *p3, *p4, *p5;
  imgpel *orig_line;
  int result;

  int jj = 1;

  /* 由于MVf(1,3)表示分像素p位置，根据原理可知它是由对角线上两个半像素计算而来 */
  /* 因此这里先横向内插 */
  for (j = 0; j < block_size_y; j++)
  {
    p0 = &cur_imgY[jj++][x_pos - 2];
    p1 = p0 + 1;
    p2 = p1 + 1;
    p3 = p2 + 1;
    p4 = p3 + 1;
    p5 = p4 + 1;

    //printf("ver %02d %02d %02d %02d %02d %02d\n", *p0, *p1, *p2, *p3, *p4, *p5);
    orig_line = block[j];

    for (i = 0; i < block_size_x; i++)
    {
      //计算s分像素点，公式见原理说明
      result  = (*(p0++) + *(p5++)) - 5 * (*(p1++) + *(p4++)) + 20 * (*(p2++) + *(p3++));
      *(orig_line++) = (imgpel) iClip1(max_imgpel_value, ((result + 16)>>5));
    }
  }

  /* 这里开始进行垂直方向内插 */
  p0 = &(cur_imgY[-2][x_pos]);
  for (j = 0; j < block_size_y; j++)
  {
    p1 = p0 + shift_x;
    p2 = p1 + shift_x;
    p3 = p2 + shift_x;
    p4 = p3 + shift_x;
    p5 = p4 + shift_x;
    orig_line = block[j];
    //printf("hor %02d %02d %02d %02d %02d %02d\n", *p0, *p1, *p2, *p3, *p4, *p5);

    for (i = 0; i < block_size_x; i++)
    {
      //计算h分像素点，公式见原理说明
      result  = (*(p0++) + *(p5++)) - 5 * (*(p1++) + *(p4++)) + 20 * (*(p2++) + *(p3++));
      //计算p分像素点，注意这里将h的取平均操作放在这里了
      *orig_line = (imgpel) ((*orig_line + iClip1(max_imgpel_value, ((result + 16) >> 5)) + 1) >> 1);
      orig_line++;
    }
    p0 = p1 - block_size_x ;
  }
}

int main(int argc, char*argv[])
{
        int i;
        int j;
        char **img_y;
        char *img_y_vir;
        char *cur_block_y_vir;
        char **cur_block_y;

	// 分配参考图像空间，这样分配可以使用二维数组访问方式来访问数据，如img_y[0][0]
        img_y = (char **)malloc(sizeof(char*) * PIC_HIGHT);
        img_y_vir = (char*)malloc(PIC_WIDTH * PIC_HIGHT);
        for(i = 0; i < PIC_HIGHT; i++) {
                img_y[i] = img_y_vir + i * PIC_WIDTH;
        }

	// 分配当前块空间，这样分配可以使用二维数组访问方式来访问数据，如cur_block_y[0][0]
        cur_block_y = (char **)malloc(sizeof(char*) * BLK_HIGHT);
        cur_block_y_vir = (char*)malloc(BLK_WIDTH * BLK_HIGHT);
        for(i = 0; i < BLK_HIGHT; i++) {
                cur_block_y[i] = cur_block_y_vir + i * BLK_WIDTH;
        }

	// 这里使用随机函数随机生成参考图像数据
	printf("ref pic data：\n");
        for(i = 0; i < PIC_HIGHT; i++) {
                for(j = 0; j < PIC_WIDTH; j++) {
                        img_y[i][j] =  rand() % 32;
                        printf("%02d ", img_y[i][j]);
                }
                printf("\n");
        }

	//开始针对MVf为(1,3)的情况进行帧间预测，注意如果MV计算出来的MVf是其他值，这里暂不支持。
	printf("ref left top point G is (%d, %d)\n", REF_POS_X, REF_POS_Y);
        get_luma_13(cur_block_y, &img_y[REF_POS_X], BLK_HIGHT, BLK_WIDTH, REF_POS_Y, PIC_WIDTH, MAX_PIX_VALUE);

	printf("cur block data：\n");
        for(i = 0; i < BLK_HIGHT; i++) {
                for(j = 0; j < BLK_WIDTH; j++) {
                        printf("%02d ", cur_block_y[i][j]);
                }
                printf("\n");
        }

        free(img_y);
        free(img_y_vir);
        free(cur_block_y);
        free(cur_block_y_vir);

        return 0;
}
