#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "quant_params.h"
#include "typedefs.h"

typedef struct video_par
{
    struct quant_params           *p_Quant;
} VideoParameters;


typedef struct slice
{
    int symbol_mode;
} Slice;

typedef struct macroblock_enc
{
    struct video_par   *p_Vid;
    struct slice       *p_Slice;
} Macroblock;

#endif
