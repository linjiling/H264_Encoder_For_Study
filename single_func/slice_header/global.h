#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "typedefs.h"
#include "parsetcommon.h"
#include "nalucommon.h"

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

//! Syntaxelement
typedef struct syntaxelement_dec
{
  int32           type;                  //!< type of syntax element for data part.
  int32           value1;                //!< numerical value of syntax element
  int32           value2;                //!< for blocked symbols, e.g. run/level
  int32           len;                   //!< length of code
  int32           inf;                   //!< info part of CAVLC code
  uint32          bitpattern;            //!< CAVLC bitpattern
  int32           context;               //!< CABAC context
  int32           k;                     //!< CABAC context for coeff_count,uv

} SyntaxElement;

typedef struct DecRefPicMarking_s
{
  int memory_management_control_operation;
  int difference_of_pic_nums_minus1;
  int long_term_pic_num;
  int long_term_frame_idx;
  int max_long_term_frame_idx_plus1;
  struct DecRefPicMarking_s *Next;
} DecRefPicMarking_t;

typedef struct video_par
{
    int current_mb_nr;
    int number;
    unsigned int log2_max_frame_num_minus4;
    unsigned int frame_num;
} VideoParameters;


typedef struct datapartition_enc
{
  struct slice        *p_Slice;
  struct video_par      *p_Vid;
  Bitstream           *bitstream;
  NALU_t              *nal_unit;
} DataPartition;

typedef struct slice
{
    struct video_par    *p_Vid;
    pic_parameter_set_rbsp_t *active_pps;
    seq_parameter_set_rbsp_t *active_sps;
    short               partition_mode;
    DataPartition       *partArr;
    short               slice_type;   //!< picture type
    short               idr_flag;
    int                 qp;
} Slice;


#endif
