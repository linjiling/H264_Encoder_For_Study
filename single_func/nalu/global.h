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

#endif
