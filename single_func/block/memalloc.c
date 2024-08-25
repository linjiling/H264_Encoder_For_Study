#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "memalloc.h"

void no_mem_exit(char *where)
{
   printf("Could not allocate memory: %s",where);
}

static inline void* mem_malloc(size_t nitems)
{
  void *d;
  if((d = malloc(nitems)) == NULL)
  {
    no_mem_exit("malloc failed.\n");
    return NULL;
  }
  return d;
}

/*!
 ************************************************************************
 * \brief
 *    allocate and set memory aligned at SSE_MEMORY_ALIGNMENT
 *
 ************************************************************************/
static inline void* mem_calloc(size_t nitems, size_t size)
{
  size_t padded_size = nitems * size;
  void *d = mem_malloc(padded_size);
  memset(d, 0, (int)padded_size);
  return d;
}

static inline void mem_free(void *a)
{
  free(a);
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

int get_mem4Dpel(imgpel *****array4D, int dim0, int dim1, int dim2, int dim3)
{
  int  i, mem_size = dim0 * sizeof(imgpel***);

  if(((*array4D) = (imgpel****)mem_malloc(dim0 * sizeof(imgpel***))) == NULL)
    no_mem_exit("get_mem4Dpel: array4D");

  mem_size += get_mem3Dpel(*array4D, dim0 * dim1, dim2, dim3);

  for(i = 1; i < dim0; i++)
    (*array4D)[i] = (*array4D)[i - 1] + dim1;

  return mem_size;
}

int get_mem2Dint(int ***array2D, int dim0, int dim1)
{
  int i;

  if((*array2D    = (int**)mem_malloc(dim0 *       sizeof(int*))) == NULL)
    no_mem_exit("get_mem2Dint: array2D");
  if((*(*array2D) = (int* )mem_calloc(dim0 * dim1, sizeof(int ))) == NULL)
    no_mem_exit("get_mem2Dint: array2D");

  for(i = 1 ; i < dim0; i++)
    (*array2D)[i] =  (*array2D)[i-1] + dim1;

  return dim0 * (sizeof(int*) + dim1 * sizeof(int));
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 3D memory array -> int array3D[dim0][dim1][dim2]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem3Dint(int ****array3D, int dim0, int dim1, int dim2)
{
  int  i, mem_size = dim0 * sizeof(int**);

  if(((*array3D) = (int***)mem_malloc(dim0 * sizeof(int**))) == NULL)
    no_mem_exit("get_mem3Dint: array3D");

  mem_size += get_mem2Dint(*array3D, dim0 * dim1, dim2);

  for(i = 1; i < dim0; i++)
    (*array3D)[i] =  (*array3D)[i-1] + dim1;

  return mem_size;
}

int get_mem4Dint(int *****array4D, int dim0, int dim1, int dim2, int dim3)
{
  int  i, mem_size = dim0 * sizeof(int***);

  if(((*array4D) = (int****)mem_malloc(dim0 * sizeof(int***))) == NULL)
    no_mem_exit("get_mem4Dint: array4D");

  mem_size += get_mem3Dint(*array4D, dim0 * dim1, dim2, dim3);

  for(i = 1; i < dim0; i++)
    (*array4D)[i] =  (*array4D)[i-1] + dim1;

  return mem_size;
}

int get_mem2D(byte ***array2D, int dim0, int dim1)
{
  int i;

  if((  *array2D  = (byte**)mem_malloc(dim0 *      sizeof(byte*))) == NULL)
    no_mem_exit("get_mem2D: array2D");
  if((*(*array2D) = (byte* )mem_calloc(dim0 * dim1,sizeof(byte ))) == NULL)
    no_mem_exit("get_mem2D: array2D");

  for(i = 1; i < dim0; i++)
    (*array2D)[i] = (*array2D)[i-1] + dim1;

  return dim0 * (sizeof(byte*) + dim1 * sizeof(byte));
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> LevelQuantParams array2D[dim0][dim1]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************/
int get_mem2Dquant(LevelQuantParams ***array2D, int dim0, int dim1)
{
  int i;

  if((*array2D    = (LevelQuantParams**) mem_malloc(dim0 *      sizeof(LevelQuantParams*))) == NULL)
    no_mem_exit("get_mem2Dquant: array2D");
  if((*(*array2D) = (LevelQuantParams* ) mem_calloc(dim0 * dim1,sizeof(LevelQuantParams ))) == NULL)
    no_mem_exit("get_mem2Dquant: array2D");

  for(i = 1 ; i < dim0; i++)
    (*array2D)[i] =  (*array2D)[i-1] + dim1;

  return dim0 * (sizeof(LevelQuantParams*) + dim1 * sizeof(LevelQuantParams));
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 3D memory array -> LevelQuantParams array3D[dim0][dim1][dim2]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem3Dquant(LevelQuantParams ****array3D, int dim0, int dim1, int dim2)
{
  int i, mem_size = dim0 * sizeof(LevelQuantParams**);

  if(((*array3D) = (LevelQuantParams***)mem_malloc(dim0 * sizeof(LevelQuantParams**))) == NULL)
    no_mem_exit("get_mem3Dquant: array3D");

  mem_size += get_mem2Dquant(*array3D, dim0 * dim1, dim2);

  for(i = 1; i < dim0; i++)
    (*array3D)[i] = (*array3D)[i - 1] + dim1;

  return mem_size;
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 4D memory array -> LevelQuantParams array3D[dim0][dim1][dim2][dim3]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem4Dquant(LevelQuantParams *****array4D, int dim0, int dim1, int dim2, int dim3)
{
  int i, mem_size = dim0 * sizeof(LevelQuantParams***);

  if(((*array4D) = (LevelQuantParams****)mem_malloc(dim0 * sizeof(LevelQuantParams***))) == NULL)
    no_mem_exit("get_mem4Dquant: array4D");

  mem_size += get_mem3Dquant(*array4D, dim0 * dim1, dim2, dim3);

  for(i = 1; i < dim0; i++)
    (*array4D)[i] = (*array4D)[i - 1] + dim1;

  return mem_size;
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 5D memory array -> LevelQuantParams array3D[dim0][dim1][dim2][dim3][dim4]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem5Dquant(LevelQuantParams ******array5D, int dim0, int dim1, int dim2, int dim3, int dim4)
{
  int i, mem_size = dim0 * sizeof(LevelQuantParams***);

  if(((*array5D) = (LevelQuantParams*****)mem_malloc(dim0 * sizeof(LevelQuantParams****))) == NULL)
    no_mem_exit("get_mem5Dquant: array5D");

  mem_size += get_mem4Dquant(*array5D, dim0 * dim1, dim2, dim3, dim4);

  for(i = 1; i < dim0; i++)
    (*array5D)[i] = (*array5D)[i - 1] + dim1;

  return mem_size;
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 2D short memory array -> short array2D[dim0][dim1]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem2Dshort(short ***array2D, int dim0, int dim1)
{
  int i;
  short *curr = NULL;

  if((  *array2D  = (short**)mem_malloc(dim0 *      sizeof(short*))) == NULL)
    no_mem_exit("get_mem2Dshort: array2D");
  if((*(*array2D) = (short* )mem_calloc(dim0 * dim1,sizeof(short ))) == NULL)
    no_mem_exit("get_mem2Dshort: array2D");

  curr = (*array2D)[0];
  for(i = 1; i < dim0; i++)
  {
    curr += dim1;
    (*array2D)[i] = curr;
  }

  return dim0 * (sizeof(short*) + dim1 * sizeof(short));
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 3D memory short array -> short array3D[dim0][dim1][dim2]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem3Dshort(short ****array3D,int dim0, int dim1, int dim2)
{
  int  i, mem_size = dim0 * sizeof(short**);
  short **curr = NULL;

  if(((*array3D) = (short***)mem_malloc(dim0 * sizeof(short**))) == NULL)
    no_mem_exit("get_mem3Dshort: array3D");

  mem_size += get_mem2Dshort(*array3D, dim0 * dim1, dim2);

  curr = (*array3D)[0];
  for(i = 1; i < dim0; i++)
  {
    curr += dim1;
    (*array3D)[i] = curr;
  }

  return mem_size;
}

