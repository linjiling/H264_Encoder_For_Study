#ifndef _MEMALLOC_H_
#define _MEMALLOC_H_

#include "global.h"

int get_mem2Dpel(imgpel ***array2D, int dim0, int dim1);
int get_mem3Dpel(imgpel ****array3D, int dim0, int dim1, int dim2);
int get_mem4Dpel(imgpel *****array4D, int dim0, int dim1, int dim2, int dim3);
int get_mem2Dint(int ***array2D, int dim0, int dim1);
int get_mem3Dint(int ****array3D, int dim0, int dim1, int dim2);
int get_mem4Dint(int *****array4D, int dim0, int dim1, int dim2, int dim3);
int get_mem2D(byte ***array2D, int dim0, int dim1);

int  get_mem2Dquant(LevelQuantParams ***array2D, int dim0, int dim1);
int  get_mem3Dquant(LevelQuantParams ****array3D, int dim0, int dim1, int dim2);
int  get_mem4Dquant(LevelQuantParams *****array4D, int dim0, int dim1, int dim2, int dim3);
int  get_mem5Dquant(LevelQuantParams ******array5D, int dim0, int dim1, int dim2, int dim3, int dim4);
int get_mem2Dshort(short ***array2D, int dim0, int dim1);
int get_mem3Dshort(short ****array3D,int dim0, int dim1, int dim2);
#endif