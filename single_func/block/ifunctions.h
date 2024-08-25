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

static inline int isignab(int a, int b)
{
  return ((b) < 0) ? -iabs(a) : iabs(a);
}

static inline int rshift_rnd_sf(int x, int a)
{
  return ((x + (1 << (a-1) )) >> a);
}

static inline int imax(int a, int b)
{
  return ((a) > (b)) ? (a) : (b);
}

static inline int iClip1(int high, int x)
{
  x = imax(x, 0);
  x = imin(x, high);

  return x;
}

static inline distblk dist_scale(distblk blkdistCost)
{
#if JCOST_CALC_SCALEUP
  return ((blkdistCost)<<LAMBDA_ACCURACY_BITS);
#else
  return (blkdistCost);
#endif
}

static inline int dist_down(distblk blkdistCost)
{
#if JCOST_CALC_SCALEUP
  return ((int)((blkdistCost)>>LAMBDA_ACCURACY_BITS));
#else
  return ((int)blkdistCost);
#endif
}

static inline distblk weighted_cost(int factor, int bits)
{
#if JCOST_CALC_SCALEUP
  return (((distblk)(factor))*((distblk)(bits)));
#else
#if (USE_RND_COST)
  return (rshift_rnd_sf((lambda) * (bits), LAMBDA_ACCURACY_BITS));
#else
  return (((factor)*(bits))>>LAMBDA_ACCURACY_BITS);
#endif
#endif
}

#endif


