#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "typedefs.h"

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
#endif
