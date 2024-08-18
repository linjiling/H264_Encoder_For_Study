#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLK_WIDTH 4 //当前块宽度
#define BLK_HIGHT 4 //当前块高度
#define BLOCK_SIZE 4
#define BLOCK_SHIFT 2

// Predictor array index definitions
#define P_X (PredPel[0])
#define P_A (PredPel[1])
#define P_B (PredPel[2])
#define P_C (PredPel[3])
#define P_D (PredPel[4])
#define P_E (PredPel[5])
#define P_F (PredPel[6])
#define P_G (PredPel[7])
#define P_H (PredPel[8])
#define P_I (PredPel[9])
#define P_J (PredPel[10])
#define P_K (PredPel[11])
#define P_L (PredPel[12])

typedef unsigned char imgpel;

enum {
  VERT_PRED            = 0,
  HOR_PRED             = 1,
  DC_PRED              = 2,
  DIAG_DOWN_LEFT_PRED  = 3,
  DIAG_DOWN_RIGHT_PRED = 4,
  VERT_RIGHT_PRED      = 5,
  HOR_DOWN_PRED        = 6,
  VERT_LEFT_PRED       = 7,
  HOR_UP_PRED          = 8
} I4x4PredModes;
/*!
 ************************************************************************
 * \brief
 *    Vertical 4x4 Prediction
 ************************************************************************
 */
static inline void get_i4x4_vertical(imgpel **cur_pred, imgpel *PredPel)
{
  memcpy(cur_pred[0], &PredPel[1], BLOCK_SIZE * sizeof(imgpel));
  memcpy(cur_pred[1], &PredPel[1], BLOCK_SIZE * sizeof(imgpel));
  memcpy(cur_pred[2], &PredPel[1], BLOCK_SIZE * sizeof(imgpel));
  memcpy(cur_pred[3], &PredPel[1], BLOCK_SIZE * sizeof(imgpel));
}
/*!
 ************************************************************************
 * \brief
 *    Horizontal 4x4 Prediction
 ************************************************************************
 */
static inline void get_i4x4_horizontal(imgpel **cur_pred, imgpel *PredPel)
{
  int i;

  for (i=0; i < BLOCK_SIZE; i++)
  {
    cur_pred[i][0]  =
    cur_pred[i][1]  =
    cur_pred[i][2]  =
    cur_pred[i][3]  = (imgpel) (&P_I)[i];
  }
}

/*!
 ************************************************************************
 * \brief
 *    DC 4x4 Prediction
 ************************************************************************
 */
static inline void get_i4x4_dc(imgpel **cur_pred, imgpel *PredPel, int left_available, int up_available)
{
  int i, j, s0 = 0;
  if (up_available && left_available)
  {
    // no edge
    s0 = (P_A + P_B + P_C + P_D + P_I + P_J + P_K + P_L + 4) >> (BLOCK_SHIFT + 1);
  }
  else if (!up_available && left_available)
  {
    // upper edge
    s0 = (P_I + P_J + P_K + P_L + 2) >> BLOCK_SHIFT;;
  }
  else if (up_available && !left_available)
  {
    // left edge
    s0 = (P_A + P_B + P_C + P_D + 2) >> BLOCK_SHIFT;
  }
  else //if (!up_available && !left_available)
  {
    // top left corner, nothing to predict from
    s0 = P_A; // P_A already set to p_Vid->dc_pred_value;
  }

  for (j=0; j < BLOCK_SIZE; j++)
  {
    for (i=0; i < BLOCK_SIZE; i++)
      cur_pred[j][i] = (imgpel) s0;
  }
}
/*!
 ************************************************************************
 * \brief
 *    Diagonal Down Left 4x4 Prediction
 ************************************************************************
 */
static inline void get_i4x4_downleft(imgpel **cur_pred, imgpel *PredPel)
{
  cur_pred[0][0] = (imgpel) ((P_A + P_C + ((P_B) << 1) + 2) >> 2);
  cur_pred[0][1] =
  cur_pred[1][0] = (imgpel) ((P_B + P_D + ((P_C) << 1) + 2) >> 2);
  cur_pred[0][2] =
  cur_pred[1][1] =
  cur_pred[2][0] = (imgpel) ((P_C + P_E + ((P_D) << 1) + 2) >> 2);
  cur_pred[0][3] =
  cur_pred[1][2] =
  cur_pred[2][1] =
  cur_pred[3][0] = (imgpel) ((P_D + P_F + ((P_E) << 1) + 2) >> 2);
  cur_pred[1][3] =
  cur_pred[2][2] =
  cur_pred[3][1] = (imgpel) ((P_E + P_G + ((P_F)<<1) + 2) >> 2);
  cur_pred[2][3] =
  cur_pred[3][2] = (imgpel) ((P_F + P_H + ((P_G)<<1) + 2) >> 2);
  cur_pred[3][3] = (imgpel) ((P_G + 3*(P_H) + 2) >> 2);
}

/*!
 ************************************************************************
 * \brief
 *    Diagonal Down Right 4x4 Prediction
 ************************************************************************
 */
static inline void get_i4x4_downright(imgpel **cur_pred, imgpel *PredPel)
{
  cur_pred[3][0] = (imgpel) ((P_L + 2*P_K + P_J + 2) >> 2);
  cur_pred[2][0] =
  cur_pred[3][1] = (imgpel) ((P_K + 2*P_J + P_I + 2) >> 2);
  cur_pred[1][0] =
  cur_pred[2][1] =
  cur_pred[3][2] = (imgpel) ((P_J + 2*P_I + P_X + 2) >> 2);
  cur_pred[0][0] =
  cur_pred[1][1] =
  cur_pred[2][2] =
  cur_pred[3][3] = (imgpel) ((P_I + 2*P_X + P_A + 2) >> 2);
  cur_pred[0][1] =
  cur_pred[1][2] =
  cur_pred[2][3] = (imgpel) ((P_X + 2*P_A + P_B + 2) >> 2);
  cur_pred[0][2] =
  cur_pred[1][3] = (imgpel) ((P_A + 2*P_B + P_C + 2) >> 2);
  cur_pred[0][3] = (imgpel) ((P_B + 2*P_C + P_D + 2) >> 2);
}
/*!
 ************************************************************************
 * \brief
 *    Vertical Left 4x4 Prediction
 ************************************************************************
 */
static inline void get_i4x4_vertleft(imgpel **cur_pred, imgpel *PredPel)
{
  cur_pred[0][0] = (imgpel) ((P_A + P_B + 1) >> 1);
  cur_pred[0][1] =
  cur_pred[2][0] = (imgpel) ((P_B + P_C + 1) >> 1);
  cur_pred[0][2] =
  cur_pred[2][1] = (imgpel) ((P_C + P_D + 1) >> 1);
  cur_pred[0][3] =
  cur_pred[2][2] = (imgpel) ((P_D + P_E + 1) >> 1);
  cur_pred[2][3] = (imgpel) ((P_E + P_F + 1) >> 1);
  cur_pred[1][0] = (imgpel) ((P_A + ((P_B)<<1) + P_C + 2) >> 2);
  cur_pred[1][1] =
  cur_pred[3][0] = (imgpel) ((P_B + ((P_C)<<1) + P_D + 2) >> 2);
  cur_pred[1][2] =
  cur_pred[3][1] = (imgpel) ((P_C + ((P_D)<<1) + P_E + 2) >> 2);
  cur_pred[1][3] =
  cur_pred[3][2] = (imgpel) ((P_D + ((P_E)<<1) + P_F + 2) >> 2);
  cur_pred[3][3] = (imgpel) ((P_E + ((P_F)<<1) + P_G + 2) >> 2);
}
/*!
 ************************************************************************
 * \brief
 *    Vertical Right 4x4 Prediction
 ************************************************************************
 */
static inline void get_i4x4_vertright(imgpel **cur_pred, imgpel *PredPel)
{
  cur_pred[0][0] =
  cur_pred[2][1] = (imgpel) ((P_X + P_A + 1) >> 1);
  cur_pred[0][1] =
  cur_pred[2][2] = (imgpel) ((P_A + P_B + 1) >> 1);
  cur_pred[0][2] =
  cur_pred[2][3] = (imgpel) ((P_B + P_C + 1) >> 1);
  cur_pred[0][3] = (imgpel) ((P_C + P_D + 1) >> 1);
  cur_pred[1][0] =
  cur_pred[3][1] = (imgpel) ((P_I + 2*P_X + P_A + 2) >> 2);
  cur_pred[1][1] =
  cur_pred[3][2] = (imgpel) ((P_X + 2*P_A + P_B + 2) >> 2);
  cur_pred[1][2] =
  cur_pred[3][3] = (imgpel) ((P_A + 2*P_B + P_C + 2) >> 2);
  cur_pred[1][3] = (imgpel) ((P_B + 2*P_C + P_D + 2) >> 2);
  cur_pred[2][0] = (imgpel) ((P_X + 2*P_I + P_J + 2) >> 2);
  cur_pred[3][0] = (imgpel) ((P_I + 2*P_J + P_K + 2) >> 2);
}
/*!
 ************************************************************************
 * \brief
 *    Horizontal Down 4x4 Prediction
 ************************************************************************
 */
static inline void get_i4x4_hordown(imgpel **cur_pred, imgpel *PredPel)
{
  cur_pred[0][0] =
  cur_pred[1][2] = (imgpel) ((P_X + P_I + 1) >> 1);
  cur_pred[0][1] =
  cur_pred[1][3] = (imgpel) ((P_I + 2*P_X + P_A + 2) >> 2);
  cur_pred[0][2] = (imgpel) ((P_X + 2*P_A + P_B + 2) >> 2);
  cur_pred[0][3] = (imgpel) ((P_A + 2*P_B + P_C + 2) >> 2);
  cur_pred[1][0] =
  cur_pred[2][2] = (imgpel) ((P_I + P_J + 1) >> 1);
  cur_pred[1][1] =
  cur_pred[2][3] = (imgpel) ((P_X + 2*P_I + P_J + 2) >> 2);
  cur_pred[2][0] =
  cur_pred[3][2] = (imgpel) ((P_J + P_K + 1) >> 1);
  cur_pred[2][1] =
  cur_pred[3][3] = (imgpel) ((P_I + 2*P_J + P_K + 2) >> 2);
  cur_pred[3][0] = (imgpel) ((P_K + P_L + 1) >> 1);
  cur_pred[3][1] = (imgpel) ((P_J + 2*P_K + P_L + 2) >> 2);
}
/*!
 ************************************************************************
 * \brief
 *    Horizontal Up 4x4 Prediction
 ************************************************************************
 */
static inline void get_i4x4_horup(imgpel **cur_pred, imgpel *PredPel)
{
  cur_pred[0][0] = (imgpel) ((P_I + P_J + 1) >> 1);
  cur_pred[0][1] = (imgpel) ((P_I + 2*P_J + P_K + 2) >> 2);
  cur_pred[0][2] =
  cur_pred[1][0] = (imgpel) ((P_J + P_K + 1) >> 1);
  cur_pred[0][3] =
  cur_pred[1][1] = (imgpel) ((P_J + 2*P_K + P_L + 2) >> 2);
  cur_pred[1][2] =
  cur_pred[2][0] = (imgpel) ((P_K + P_L + 1) >> 1);
  cur_pred[1][3] =
  cur_pred[2][1] = (imgpel) ((P_K + 2*P_L + P_L + 2) >> 2);
  cur_pred[3][0] =
  cur_pred[2][2] =
  cur_pred[2][3] =
  cur_pred[3][1] =
  cur_pred[3][2] =
  cur_pred[3][3] = (imgpel) P_L;
}

/*!
 ************************************************************************
 * \brief
 *    I4x4
 *
 *  \par Input:
 *    intra4x4_pred: 
 *
 *  \par Output:
 *    mpr_4x4:
 ************************************************************************
 */
void get_intrapred_4x4(imgpel *PredPel, imgpel ***curr_mpr_4x4, int i4x4_mode,  int left_available, int up_available)
{

  switch (i4x4_mode)
  {
  case VERT_PRED :
    get_i4x4_vertical(curr_mpr_4x4[VERT_PRED], PredPel);
    break;
  case HOR_PRED :
    get_i4x4_horizontal(curr_mpr_4x4[HOR_PRED], PredPel);
    break;
  case DC_PRED :
    get_i4x4_dc(curr_mpr_4x4[DC_PRED], PredPel, left_available, up_available);
    break;
  case DIAG_DOWN_LEFT_PRED :
    get_i4x4_downleft(curr_mpr_4x4[DIAG_DOWN_LEFT_PRED], PredPel);
    break;
  case DIAG_DOWN_RIGHT_PRED :
    get_i4x4_downright(curr_mpr_4x4[DIAG_DOWN_RIGHT_PRED], PredPel);
    break;
  case VERT_RIGHT_PRED :
    get_i4x4_vertright(curr_mpr_4x4[VERT_RIGHT_PRED], PredPel);
    break;
  case HOR_DOWN_PRED :
    get_i4x4_hordown(curr_mpr_4x4[HOR_DOWN_PRED], PredPel);
    break;
  case VERT_LEFT_PRED :
    get_i4x4_vertleft(curr_mpr_4x4[VERT_LEFT_PRED], PredPel);
    break;
  case HOR_UP_PRED :
    get_i4x4_horup(curr_mpr_4x4[HOR_UP_PRED], PredPel);
    break;
  default:
    printf("invalid prediction mode \n");
    break;
  }
}

int get_mem2Dpel(imgpel ***array2D, int dim0, int dim1)
{
  int i;

  if((*array2D    = (imgpel**)malloc(dim0 *        sizeof(imgpel*))) == NULL)
    printf("get_mem2Dpel: array2D");
  if((*(*array2D) = (imgpel* )malloc(dim0 * dim1 * sizeof(imgpel ))) == NULL)
    printf("get_mem2Dpel: array2D");

  for(i = 1 ; i < dim0; i++)
  {
    (*array2D)[i] = (*array2D)[i-1] + dim1;
  }

  return dim0 * (sizeof(imgpel*) + dim1 * sizeof(imgpel));
}

int get_mem3Dpel(imgpel ****array3D, int dim0, int dim1, int dim2)
{
  int i, mem_size = dim0 * sizeof(imgpel**);

  if(((*array3D) = (imgpel***)malloc(dim0 * sizeof(imgpel**))) == NULL)
    printf("get_mem3Dpel: array3D");

  mem_size += get_mem2Dpel(*array3D, dim0 * dim1, dim2);

  for(i = 1; i < dim0; i++)
    (*array3D)[i] = (*array3D)[i - 1] + dim1;

  return mem_size;
}

int main(int argc, char *argv[])
{
    imgpel        PredPel[17];
    imgpel ***curr_mpr_4x4;
    int i;

    get_mem3Dpel(&curr_mpr_4x4, 9, 4, 4);

    for (i = 0; i < 17; i++) {
        PredPel[i] = rand() % 32;
    }


    for (i = 0; i < 9; i++) {
        get_intrapred_4x4(PredPel, curr_mpr_4x4, i, 1, 1);
	printf("now intra pred mode[%d]:\n", i);
	printf("%2d %2d %2d %2d %2d %2d %2d %2d %2d\n", PredPel[0], PredPel[1], PredPel[2], PredPel[3], PredPel[4],
			                                PredPel[5], PredPel[6], PredPel[7], PredPel[8]);
	printf("%2d %2d %2d %2d %2d\n", PredPel[9], curr_mpr_4x4[i][0][0], curr_mpr_4x4[i][0][1], curr_mpr_4x4[i][0][2], curr_mpr_4x4[i][0][3]);
	printf("%2d %2d %2d %2d %2d\n", PredPel[10], curr_mpr_4x4[i][1][0], curr_mpr_4x4[i][1][1], curr_mpr_4x4[i][1][2], curr_mpr_4x4[i][1][3]);
	printf("%2d %2d %2d %2d %2d\n", PredPel[11], curr_mpr_4x4[i][2][0], curr_mpr_4x4[i][2][1], curr_mpr_4x4[i][2][2], curr_mpr_4x4[i][2][3]);
	printf("%2d %2d %2d %2d %2d\n", PredPel[12], curr_mpr_4x4[i][3][0], curr_mpr_4x4[i][3][1], curr_mpr_4x4[i][3][2], curr_mpr_4x4[i][3][3]);
    }
    
    return 0;
}
