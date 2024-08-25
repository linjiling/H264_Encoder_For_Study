#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "typedefs.h"
#include "types.h"
#include "defines.h"

typedef struct block_pos BlockPos;

struct block_pos
{
  short x;
  short y;
};

typedef struct pix_pos
{
  int   available;
  int   mb_addr;
  short x;
  short y;
  short pos_x;
  short pos_y;
} PixelPos;

//! Bitstream
typedef struct bit_stream_enc
{
  int32     buffer_size;        //!< Buffer size
  int32     byte_pos;           //!< current position in bitstream;
  int32     bits_to_go;         //!< current bitcounter

  int32     stored_byte_pos;    //!< storage for position in bitstream;
  int32     stored_bits_to_go;  //!< storage for bitcounter
  int32     byte_pos_skip;      //!< storage for position in bitstream;
  int32     bits_to_go_skip;    //!< storage for bitcounter
  int32     write_flag;         //!< Bitstream contains data and needs to be written

  byte    byte_buf;           //!< current buffer for last written byte
  byte    stored_byte_buf;    //!< storage for buffer of last written byte
  byte    byte_buf_skip;      //!< current buffer for last written byte
  byte    *streamBuffer;      //!< actual buffer for written bytes

} Bitstream;

typedef struct syntaxelement_enc
{
  int                 type;           //!< type of syntax element for data part.
  int                 value1;         //!< numerical value of syntax element
  int                 value2;         //!< for blocked symbols, e.g. run/level
  int                 len;            //!< length of code
  int                 inf;            //!< info part of UVLC code
  unsigned int        bitpattern;     //!< UVLC bitpattern
  int                 context;        //!< CABAC context

#if TRACE
  #define             TRACESTRING_SIZE 100            //!< size of trace string
  char                tracestring[TRACESTRING_SIZE];  //!< trace string
#endif

  //!< for mapping of syntaxElement to UVLC
  void    (*mapping)(int value1, int value2, int* len_ptr, int* info_ptr);

} SyntaxElement;

typedef struct
{
    unsigned int profile_idc;  
} seq_parameter_set_rbsp_t;

typedef struct
{
    Boolean   constrained_intra_pred_flag;
} pic_parameter_set_rbsp_t;

typedef struct video_par
{
    int mb_size[MAX_PLANE][2];
    BlockPos *PicPos;
    pic_parameter_set_rbsp_t *active_pps;
    seq_parameter_set_rbsp_t *active_sps;
    int num_cdc_coeff;
    int ***nz_coeff;
    ColorFormat yuv_format;
    short   *intra_block;
    unsigned int FrameSizeInMbs;
    unsigned int PicWidthInMbs;
    int num_blk8x8_uv;
} VideoParameters;

typedef struct datapartition_enc
{
  Bitstream           *bitstream;
} DataPartition;

typedef struct slice
{
    struct video_par    *p_Vid;
    short               partition_mode;
    int     ****cofAC;
    int     ***cofDC;
    DataPartition       *partArr;
    short  idr_flag;
} Slice;

typedef struct bit_counter BitCounter;
struct bit_counter
{
  int mb_total;
  unsigned short mb_mode;
  unsigned short mb_inter;
  unsigned short mb_cbp;
  unsigned short mb_delta_quant;
  int mb_y_coeff;
  int mb_uv_coeff;
  int mb_cb_coeff;
  int mb_cr_coeff;
  int mb_stuffing;
};

typedef struct macroblock_enc
{
    struct video_par   *p_Vid;
    struct slice       *p_Slice;

    int                 mbAddrX;
    int                 mbAddrA, mbAddrB, mbAddrC, mbAddrD;
    byte                mbAvailA, mbAvailB, mbAvailC, mbAvailD;
    int                 DeblockCall;
    BitCounter          bits;
    short               mb_type;
} Macroblock;
#endif
