#ifndef _VLC_H_
#define _VLC_H_

#include "typedefs.h"

Boolean write_u_1  (char *tracestring, int value, Bitstream *bitstream);
int     write_se_v (char *tracestring, int value, Bitstream *bitstream);
int     write_ue_v (char *tracestring, int value, Bitstream *bitstream);
int     write_u_v  (int n, char *tracestring, int value, Bitstream *bitstream);

#endif
