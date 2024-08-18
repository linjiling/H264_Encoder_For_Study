#ifndef _MEMALLOC_H_
#define _MEMALLOC_H_

#include "global.h"

int get_mem2Dpel(imgpel ***array2D, int dim0, int dim1);
int get_mem3Dpel(imgpel ****array3D, int dim0, int dim1, int dim2);
int get_mem4Dpel(imgpel *****array4D, int dim0, int dim1, int dim2, int dim3);
int get_mem2Dint(int ***array2D, int dim0, int dim1);
int get_mem3Dint(int ****array3D, int dim0, int dim1, int dim2);
int get_mem2D(byte ***array2D, int dim0, int dim1);
#endif
