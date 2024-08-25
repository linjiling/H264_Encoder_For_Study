#ifndef _IFUNCTIONS_H_
#define _IFUNCTIONS_H_

#include <math.h>
#include <limits.h>

static inline short sabs(short x)
{
  static const short SHORT_BITS = (sizeof(short) * CHAR_BIT) - 1;
  short y = (short) (x >> SHORT_BITS);
  return (short) ((x ^ y) - y);
}

static inline int iabs(int x)
{
  static const int INT_BITS = (sizeof(int) * CHAR_BIT) - 1;
  int y = x >> INT_BITS;
  return (x ^ y) - y;
}

static inline int imin(int a, int b)
{
  return ((a) < (b)) ? (a) : (b);
}
#endif


