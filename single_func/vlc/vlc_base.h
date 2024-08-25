#ifndef _VLC_H_
#define _VLC_H_

#include "global.h"
#include "defines.h"

extern Boolean write_u_1  (char *tracestring, int value, Bitstream *bitstream);
extern int     write_se_v (char *tracestring, int value, Bitstream *bitstream);
extern int     write_ue_v (char *tracestring, int value, Bitstream *bitstream);
extern int     write_u_v  (int n, char *tracestring, int value, Bitstream *bitstream);

int writeSyntaxElement_VLC(SyntaxElement *se, DataPartition *dp);
int writeSyntaxElement_TotalZeros(SyntaxElement *se, DataPartition *dp);
int writeSyntaxElement_TotalZerosChromaDC(VideoParameters *p_Vid, SyntaxElement *se, DataPartition *dp);
int writeSyntaxElement_NumCoeffTrailingOnesChromaDC(VideoParameters *p_Vid, SyntaxElement *se, DataPartition *dp);
int writeSyntaxElement_NumCoeffTrailingOnes(SyntaxElement *se, DataPartition *dp);

int writeSyntaxElement_Run(SyntaxElement *se, DataPartition *dp);
int writeSyntaxElement_Level_VLC1(SyntaxElement *se, DataPartition *dp, int profile_idc);
int writeSyntaxElement_Level_VLCN(SyntaxElement *se, int vlc, DataPartition *dp, int profile_idc);
#endif
