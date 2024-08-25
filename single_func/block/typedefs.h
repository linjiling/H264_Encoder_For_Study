#ifndef _TYPEDEFS_H_
#define _TYPEDEFS_H_

typedef unsigned char  byte;     //!< byte type definition
typedef unsigned char  uint8;    //!< type definition for unsigned char (same as byte, 8 bits)
typedef unsigned short uint16;   //!< type definition for unsigned short (16 bits)
typedef unsigned int   uint32;   //!< type definition for unsigned int (32 bits)

typedef          char  int8;
typedef          short int16;
typedef          int   int32;

#if IMGTYPE == 0
typedef byte   imgpel;           //!< pixel type
typedef uint16 distpel;          //!< distortion type (for pixels)
typedef int32  distblk;          //!< distortion type (for Macroblock)
typedef int32  transpel;         //!< transformed coefficient type
#else
typedef uint16 imgpel;
typedef uint32 distpel;
typedef int64  distblk;
typedef int32  transpel;
#endif

//! Boolean Type
typedef enum {
  FALSE,
  TRUE
} Boolean;

#endif
