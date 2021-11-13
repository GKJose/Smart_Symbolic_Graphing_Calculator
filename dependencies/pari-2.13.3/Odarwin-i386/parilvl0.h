#ifndef ASMINLINE
#line 2 "../src/kernel/none/addll.h"
/* Copyright (C) 2003  The PARI group.

This file is part of the PARI/GP package.

PARI/GP is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version. It is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY WHATSOEVER.

Check the License for details. You should have received a copy of it, along
with the package; see the file 'COPYING'. If not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

/* This file originally adapted from gmp-3.1.1 (from T. Granlund), files
 * longlong.h and gmp-impl.h

  Copyright (C) 2000 Free Software Foundation, Inc. */

#undef LOCAL_OVERFLOW
#define LOCAL_OVERFLOW
extern ulong overflow;

#if !defined(INLINE)
extern long addll(ulong x, ulong y);
extern long addllx(ulong x, ulong y);
extern long subll(ulong x, ulong y);
extern long subllx(ulong x, ulong y);
#else

#if defined(__GNUC__) && !defined(DISABLE_INLINE)
#undef LOCAL_OVERFLOW
#define LOCAL_OVERFLOW register ulong overflow

#define addll(a, b)                                             \
__extension__ ({                                                \
   ulong __arg1 = (a), __arg2 = (b), __value = __arg1 + __arg2; \
   overflow = (__value < __arg1);                               \
   __value;                                                     \
})

#define addllx(a, b)                                          \
__extension__ ({                                              \
   ulong __arg1 = (a), __arg2 = (b), __value, __tmp = __arg1 + overflow;\
   overflow = (__tmp < __arg1);                               \
   __value = __tmp + __arg2;                                  \
   overflow |= (__value < __tmp);                             \
   __value;                                                   \
})

#define subll(a, b)                                           \
__extension__ ({                                              \
   ulong __arg1 = (a), __arg2 = (b);                          \
   overflow = (__arg2 > __arg1);                              \
   __arg1 - __arg2;                                           \
})

#define subllx(a, b)                                  \
__extension__ ({                                      \
   ulong __arg1 = (a), __arg2 = (b), __value, __tmp = __arg1 - overflow;\
   overflow = (__arg1 < overflow);                    \
   __value = __tmp - __arg2;                          \
   overflow |= (__arg2 > __tmp);                      \
   __value;                                           \
})

#else /* __GNUC__ */

INLINE long
addll(ulong x, ulong y)
{
  const ulong z = x+y;
  overflow=(z<x);
  return (long) z;
}

INLINE long
addllx(ulong x, ulong y)
{
  const ulong z = x+y+overflow;
  overflow = (z<x || (z==x && overflow));
  return (long) z;
}

INLINE long
subll(ulong x, ulong y)
{
  const ulong z = x-y;
  overflow = (z>x);
  return (long) z;
}

INLINE long
subllx(ulong x, ulong y)
{
  const ulong z = x-y-overflow;
  overflow = (z>x || (z==x && overflow));
  return (long) z;
}

#endif /* __GNUC__ */

#endif
#line 2 "../src/kernel/none/mulll.h"
/* Copyright (C) 2000  The PARI group.

This file is part of the PARI/GP package.

PARI/GP is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version. It is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY WHATSOEVER.

Check the License for details. You should have received a copy of it, along
with the package; see the file 'COPYING'. If not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

#undef  LOCAL_HIREMAINDER
#define LOCAL_HIREMAINDER
extern ulong hiremainder;

/* Version Peter Montgomery */
/*
 *      Assume (for presentation) that BITS_IN_LONG = 32.
 *      Then 0 <= xhi, xlo, yhi, ylo <= 2^16 - 1.  Hence
 *
 * -2^31 + 2^16 <= (xhi-2^15)*(ylo-2^15) + (xlo-2^15)*(yhi-2^15) <= 2^31.
 *
 *      If xhi*ylo + xlo*yhi = 2^32*overflow + xymid, then
 *
 * -2^32 + 2^16 <= 2^32*overflow + xymid - 2^15*(xhi + ylo + xlo + yhi) <= 0.
 *
 * 2^16*overflow <= (xhi+xlo+yhi+ylo)/2 - xymid/2^16 <= 2^16*overflow + 2^16-1
 *
 *       This inequality was derived using exact (rational) arithmetic;
 *       it remains valid when we truncate the two middle terms.
 */

#if !defined(INLINE)
extern long mulll(ulong x, ulong y);
extern long addmul(ulong x, ulong y);
#else

#if defined(__GNUC__) && !defined(DISABLE_INLINE)
#undef LOCAL_HIREMAINDER
#define LOCAL_HIREMAINDER register ulong hiremainder

#define mulll(x, y) \
__extension__ ({ \
  const ulong __x = (x), __y = (y);\
  const ulong __xlo = LOWWORD(__x), __xhi = HIGHWORD(__x); \
  const ulong __ylo = LOWWORD(__y), __yhi = HIGHWORD(__y); \
  ulong __xylo,__xymid,__xyhi,__xymidhi,__xymidlo; \
  ulong __xhl,__yhl; \
 \
  __xylo = __xlo*__ylo; __xyhi = __xhi*__yhi; \
  __xhl = __xhi+__xlo; __yhl = __yhi+__ylo; \
  __xymid = __xhl*__yhl - (__xyhi+__xylo); \
 \
  __xymidhi = HIGHWORD(__xymid); \
  __xymidlo = __xymid << BITS_IN_HALFULONG; \
 \
  __xylo += __xymidlo; \
  hiremainder = __xyhi + __xymidhi + (__xylo < __xymidlo) \
     + ((((__xhl + __yhl) >> 1) - __xymidhi) & HIGHMASK); \
 \
  __xylo; \
})

#define addmul(x, y) \
__extension__ ({                                           \
  const ulong __x = (x), __y = (y);\
  const ulong __xlo = LOWWORD(__x), __xhi = HIGHWORD(__x); \
  const ulong __ylo = LOWWORD(__y), __yhi = HIGHWORD(__y); \
  ulong __xylo,__xymid,__xyhi,__xymidhi,__xymidlo; \
  ulong __xhl,__yhl; \
 \
  __xylo = __xlo*__ylo; __xyhi = __xhi*__yhi; \
  __xhl = __xhi+__xlo; __yhl = __yhi+__ylo; \
  __xymid = __xhl*__yhl - (__xyhi+__xylo); \
 \
  __xylo += hiremainder; __xyhi += (__xylo < hiremainder); \
 \
  __xymidhi = HIGHWORD(__xymid); \
  __xymidlo = __xymid << BITS_IN_HALFULONG; \
 \
  __xylo += __xymidlo; \
  hiremainder = __xyhi + __xymidhi + (__xylo < __xymidlo) \
     + ((((__xhl + __yhl) >> 1) - __xymidhi) & HIGHMASK); \
 \
  __xylo; \
})

#else

INLINE long
mulll(ulong x, ulong y)
{
  const ulong xlo = LOWWORD(x), xhi = HIGHWORD(x);
  const ulong ylo = LOWWORD(y), yhi = HIGHWORD(y);
  ulong xylo,xymid,xyhi,xymidhi,xymidlo;
  ulong xhl,yhl;

  xylo = xlo*ylo; xyhi = xhi*yhi;
  xhl = xhi+xlo; yhl = yhi+ylo;
  xymid = xhl*yhl - (xyhi+xylo);

  xymidhi = HIGHWORD(xymid);
  xymidlo = xymid << BITS_IN_HALFULONG;

  xylo += xymidlo;
  hiremainder = xyhi + xymidhi + (xylo < xymidlo)
     + ((((xhl + yhl) >> 1) - xymidhi) & HIGHMASK);

  return xylo;
}

INLINE long
addmul(ulong x, ulong y)
{
  const ulong xlo = LOWWORD(x), xhi = HIGHWORD(x);
  const ulong ylo = LOWWORD(y), yhi = HIGHWORD(y);
  ulong xylo,xymid,xyhi,xymidhi,xymidlo;
  ulong xhl,yhl;

  xylo = xlo*ylo; xyhi = xhi*yhi;
  xhl = xhi+xlo; yhl = yhi+ylo;
  xymid = xhl*yhl - (xyhi+xylo);

  xylo += hiremainder; xyhi += (xylo < hiremainder);

  xymidhi = HIGHWORD(xymid);
  xymidlo = xymid << BITS_IN_HALFULONG;

  xylo += xymidlo;
  hiremainder = xyhi + xymidhi + (xylo < xymidlo)
     + ((((xhl + yhl) >> 1) - xymidhi) & HIGHMASK);

  return xylo;
}
#endif

#endif
#line 2 "../src/kernel/none/bfffo.h"
/* Copyright (C) 2000  The PARI group.

This file is part of the PARI/GP package.

PARI/GP is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version. It is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY WHATSOEVER.

Check the License for details. You should have received a copy of it, along
with the package; see the file 'COPYING'. If not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

#if !defined(INLINE)
extern int  bfffo(ulong x);
#else

#if defined(__GNUC__) && !defined(DISABLE_INLINE)

#ifdef LONG_IS_64BIT
#  define bfffo(x) \
__extension__ ({ \
  static int __bfffo_tabshi[16]={4,3,2,2,1,1,1,1,0,0,0,0,0,0,0,0};\
  int __value = BITS_IN_LONG - 4; \
  ulong __arg1=(x); \
  if (__arg1 & ~0xffffffffUL) {__value -= 32; __arg1 >>= 32;}\
  if (__arg1 & ~0xffffUL) {__value -= 16; __arg1 >>= 16;} \
  if (__arg1 & ~0x00ffUL) {__value -= 8; __arg1 >>= 8;} \
  if (__arg1 & ~0x000fUL) {__value -= 4; __arg1 >>= 4;} \
  __value + __bfffo_tabshi[__arg1]; \
})
#else
#  define bfffo(x) \
__extension__ ({ \
  static int __bfffo_tabshi[16]={4,3,2,2,1,1,1,1,0,0,0,0,0,0,0,0};\
  int __value = BITS_IN_LONG - 4; \
  ulong __arg1=(x); \
  if (__arg1 & ~0xffffUL) {__value -= 16; __arg1 >>= 16;} \
  if (__arg1 & ~0x00ffUL) {__value -= 8; __arg1 >>= 8;} \
  if (__arg1 & ~0x000fUL) {__value -= 4; __arg1 >>= 4;} \
  __value + __bfffo_tabshi[__arg1]; \
})
#endif

#else

INLINE int
bfffo(ulong x)
{
  static int tabshi[16]={4,3,2,2,1,1,1,1,0,0,0,0,0,0,0,0};
  int value = BITS_IN_LONG - 4;
  ulong arg1=x;
#ifdef LONG_IS_64BIT
  if (arg1 & ~0xffffffffUL) {value -= 32; arg1 >>= 32;}
#endif
  if (arg1 & ~0xffffUL) {value -= 16; arg1 >>= 16;}
  if (arg1 & ~0x00ffUL) {value -= 8; arg1 >>= 8;}
  if (arg1 & ~0x000fUL) {value -= 4; arg1 >>= 4;}
  return value + tabshi[arg1];
}
#endif

#endif
#line 2 "../src/kernel/none/divll.h"
/* Copyright (C) 2003  The PARI group.

This file is part of the PARI/GP package.

PARI/GP is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version. It is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY WHATSOEVER.

Check the License for details. You should have received a copy of it, along
with the package; see the file 'COPYING'. If not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

/* This file originally adapted from gmp-3.1.1 (from T. Granlund), files
 * longlong.h and gmp-impl.h

  Copyright (C) 2000 Free Software Foundation, Inc. */

#undef  LOCAL_HIREMAINDER
#define LOCAL_HIREMAINDER
extern ulong hiremainder;

#if !defined(INLINE)
extern long divll(ulong x, ulong y);
#else

#define __GLUE(hi, lo) (((hi) << BITS_IN_HALFULONG) | (lo))
#define __SPLIT(a, b, c) b = HIGHWORD(a); c = LOWWORD(a)
#define __LDIV(a, b, q, r) q = a / b; r = a - q*b
extern ulong hiremainder;

/* divide (hiremainder * 2^BITS_IN_LONG + n0) by d; assume hiremainder < d.
 * Return quotient, set hiremainder to remainder */

#if defined(__GNUC__) && !defined(DISABLE_INLINE)
#undef LOCAL_HIREMAINDER
#define LOCAL_HIREMAINDER register ulong hiremainder

#define divll(n0, d)                                                    \
__extension__ ({                                                        \
  ulong __d1, __d0, __q1, __q0, __r1, __r0, __m, __n1, __n0;            \
  ulong __k, __d;                                                       \
                                                                        \
  __n1 = hiremainder; __n0 = n0; __d = d;                               \
  if (__n1 == 0)                                                        \
  { /* Only one division needed */                                      \
    __LDIV(__n0, __d, __q1, hiremainder);                               \
  }                                                                     \
  else if (__d < LOWMASK)                                               \
  { /* Two half-word divisions  */                                      \
    __n1 = __GLUE(__n1, HIGHWORD(__n0));                                \
    __LDIV(__n1, __d, __q1, __r1);                                      \
    __n1 = __GLUE(__r1,  LOWWORD(__n0));                                \
    __LDIV(__n1, __d, __q0, hiremainder);                               \
    __q1 = __GLUE(__q1, __q0);                                          \
  }                                                                     \
  else                                                                  \
  { /* General case */                                                  \
    if (__d & HIGHBIT)                                                  \
    {                                                                   \
      __k = 0; __SPLIT(__d, __d1, __d0);                                \
    }                                                                   \
    else                                                                \
    {                                                                   \
      __k = bfffo(__d);                                                 \
      __n1 = (__n1 << __k) | (__n0 >> (BITS_IN_LONG - __k));            \
      __n0 <<= __k;                                                     \
      __d = __d << __k; __SPLIT(__d, __d1, __d0);                       \
    }                                                                   \
    __LDIV(__n1, __d1, __q1, __r1);                                     \
    __m =  __q1 * __d0;                                                 \
    __r1 = __GLUE(__r1, HIGHWORD(__n0));                                  \
    if (__r1 < __m)                                                        \
    {                                                                        \
      __q1--, __r1 += __d;                                                \
      if (__r1 >= __d) /* we didn't get carry when adding to __r1 */    \
        if (__r1 < __m)        __q1--, __r1 += __d;                                \
    }                                                                        \
    __r1 -= __m;                                                        \
    __LDIV(__r1, __d1, __q0, __r0);                                     \
    __m =  __q0 * __d0;                                                  \
    __r0 = __GLUE(__r0, LOWWORD(__n0));                                   \
    if (__r0 < __m)                                                        \
    {                                                                        \
      __q0--, __r0 += __d;                                                \
      if (__r0 >= __d)                                                        \
        if (__r0 < __m)        __q0--, __r0 += __d;                                \
    }                                                                        \
    hiremainder = (__r0 - __m) >> __k;                                        \
    __q1 = __GLUE(__q1, __q0);                                                 \
  }                                                                           \
  __q1;                                                                        \
})

#else /* __GNUC__ */

INLINE long
divll(ulong n0, ulong d)
{
  ulong __d1, __d0, __q1, __q0, __r1, __r0, __m, __n1, __n0;
  ulong __k, __d;

  __n1 = hiremainder; __n0 = n0; __d = d;

  if (__n1 == 0)
  { /* Only one division needed */
    __LDIV(__n0, __d, __q1, hiremainder);
  }
  else if (__d < LOWMASK)
  { /* Two half-word divisions  */
    __n1 = __GLUE(__n1, HIGHWORD(__n0));
    __LDIV(__n1, __d, __q1, __r1);
    __n1 = __GLUE(__r1,  LOWWORD(__n0));
    __LDIV(__n1, __d, __q0, hiremainder);
    __q1 = __GLUE(__q1, __q0);
  }
  else
  { /* General case */
    if (__d & HIGHBIT)
    {
      __k = 0; __SPLIT(__d, __d1, __d0);
    }
    else
    {
      __k = bfffo(__d);
      __n1 = (__n1 << __k) | (__n0 >> (BITS_IN_LONG - __k));
      __n0 = __n0 << __k;
      __d = __d << __k; __SPLIT(__d, __d1, __d0);
    }
    __LDIV(__n1, __d1, __q1, __r1);
    __m =  __q1 * __d0;
    __r1 = __GLUE(__r1, HIGHWORD(__n0));
    if (__r1 < __m)
      {
        __q1--, __r1 += __d;
        if (__r1 >= __d) /* we didn't get carry when adding to __r1 */
          if (__r1 < __m) __q1--, __r1 += __d;
      }
    __r1 -= __m;
    __LDIV(__r1, __d1, __q0, __r0);
    __m =  __q0 * __d0;
    __r0 = __GLUE(__r0, LOWWORD(__n0));
    if (__r0 < __m)
      {
        __q0--, __r0 += __d;
        if (__r0 >= __d)
          if (__r0 < __m) __q0--, __r0 += __d;
      }
    hiremainder = (__r0 - __m) >> __k;
    __q1 = __GLUE(__q1, __q0);
  }
  return __q1;
}

#endif /* __GNUC__ */

#endif
#endif
#line 2 "../src/kernel/x86-64/asm0.h"
/* Copyright (C) 2004  The PARI group.

This file is part of the PARI/GP package.

PARI/GP is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version. It is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY WHATSOEVER.

Check the License for details. You should have received a copy of it, along
with the package; see the file 'COPYING'. If not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

/*
ASM addll mulll bfffo divll
*/
/* Written by Bill Allombert from the ix86 version by Bruno Haible. Basically
 * change insl to insq*/
#ifdef ASMINLINE
#define LOCAL_HIREMAINDER  register ulong hiremainder
#define LOCAL_OVERFLOW     register ulong overflow

#define addll(a,b) \
__extension__ ({ ulong __value, __arg1 = (a), __arg2 = (b); \
   __asm__ ("addq %3,%0 ; adcq %1,%1" \
        : "=r" (__value), "=r" (overflow) \
        : "0" (__arg1), "g" (__arg2), "1" ((ulong)0) \
        : "cc"); \
  __value; \
})

#define addllx(a,b) \
__extension__ ({ ulong __value, __arg1 = (a), __arg2 = (b), __temp; \
   __asm__ ("subq %5,%2 ; adcq %4,%0 ; adcq %1,%1" \
        : "=r" (__value), "=&r" (overflow), "=&r" (__temp) \
        : "0" (__arg1), "g" (__arg2), "g" (overflow), "1" ((ulong)0), "2" ((ulong)0) \
        : "cc"); \
  __value; \
})

#define addllx8(a,b,c,overflow) \
do { long *__arg1 = a, *__arg2 = b, *__out = c; \
     ulong __temp; \
   __asm__ ("subq %5, %0 \n\t" \
            "movq    (%2), %0 ; adcq    (%3),%0; movq %0,    (%4) \n\t" \
            "movq  -8(%2), %0 ; adcq  -8(%3),%0; movq %0,  -8(%4) \n\t" \
            "movq -16(%2), %0 ; adcq -16(%3),%0; movq %0, -16(%4) \n\t" \
            "movq -24(%2), %0 ; adcq -24(%3),%0; movq %0, -24(%4) \n\t" \
            "movq -32(%2), %0 ; adcq -32(%3),%0; movq %0, -32(%4) \n\t" \
            "movq -40(%2), %0 ; adcq -40(%3),%0; movq %0, -40(%4) \n\t" \
            "movq -48(%2), %0 ; adcq -48(%3),%0; movq %0, -48(%4) \n\t" \
            "movq -56(%2), %0 ; adcq -56(%3),%0; movq %0, -56(%4) \n\t" \
            "adcq  %1, %1" \
        : "=&r" (__temp), "=&r" (overflow) \
        : "r" (__arg1), "r" (__arg2), "r" (__out), "g" (overflow), "0" ((ulong)0), "1" ((ulong)0) \
        : "cc"); \
} while(0)

#define subll(a,b) \
__extension__ ({ ulong __value, __arg1 = (a), __arg2 = (b); \
   __asm__ ("subq %3,%0 ; adcq %1,%1" \
        : "=r" (__value), "=r" (overflow) \
        : "0" (__arg1), "g" (__arg2), "1" ((ulong)0) \
        : "cc"); \
  __value; \
})

#define subllx(a,b) \
__extension__ ({ ulong __value, __arg1 = (a), __arg2 = (b), __temp; \
   __asm__ ("subq %5,%2 ; sbbq %4,%0 ; adcq %1,%1" \
        : "=r" (__value), "=&r" (overflow), "=&r" (__temp) \
        : "0" (__arg1), "g" (__arg2), "g" (overflow), "1" ((ulong)0), "2" ((ulong)0) \
        : "cc"); \
  __value; \
})

#define subllx8(a,b,c,overflow) \
do { long *__arg1 = a, *__arg2 = b, *__out = c; \
     ulong __temp; \
   __asm__ ("subq %5, %0 \n\t" \
            "movq    (%2), %0 ; sbbq    (%3),%0; movq %0,    (%4) \n\t" \
            "movq  -8(%2), %0 ; sbbq  -8(%3),%0; movq %0,  -8(%4) \n\t" \
            "movq -16(%2), %0 ; sbbq -16(%3),%0; movq %0, -16(%4) \n\t" \
            "movq -24(%2), %0 ; sbbq -24(%3),%0; movq %0, -24(%4) \n\t" \
            "movq -32(%2), %0 ; sbbq -32(%3),%0; movq %0, -32(%4) \n\t" \
            "movq -40(%2), %0 ; sbbq -40(%3),%0; movq %0, -40(%4) \n\t" \
            "movq -48(%2), %0 ; sbbq -48(%3),%0; movq %0, -48(%4) \n\t" \
            "movq -56(%2), %0 ; sbbq -56(%3),%0; movq %0, -56(%4) \n\t" \
            "adcq  %1, %1" \
        : "=&r" (__temp), "=&r" (overflow) \
        : "r" (__arg1), "r" (__arg2), "r" (__out), "g" (overflow), "0" ((ulong)0), "1" ((ulong)0) \
        : "cc"); \
} while(0)

#define mulll(a,b) \
__extension__ ({ ulong __valuelo, __arg1 = (a), __arg2 = (b); \
   __asm__ ("mulq %3" \
        : "=a" /* %eax */ (__valuelo), "=d" /* %edx */ (hiremainder) \
        : "0" (__arg1), "rm" (__arg2)); \
   __valuelo; \
})

#define addmul(a,b) \
__extension__ ({ ulong __valuelo, __arg1 = (a), __arg2 = (b), __temp; \
   __asm__ ("mulq %4 ; addq %5,%0 ; adcq %6,%1" \
        : "=a" /* %eax */ (__valuelo), "=&d" /* %edx */ (hiremainder), "=r" (__temp) \
        : "0" (__arg1), "rm" (__arg2), "g" (hiremainder), "2" ((ulong)0)); \
   __valuelo; \
})

#define divll(a,b) \
__extension__ ({ ulong __value, __arg1 = (a), __arg2 = (b); \
   __asm__ ("divq %4" \
        : "=a" /* %eax */ (__value), "=d" /* %edx */ (hiremainder) \
        : "0" /* %eax */ (__arg1), "1" /* %edx */ (hiremainder), "mr" (__arg2)); \
   __value; \
})

#define bfffo(x) \
__extension__ ({ ulong __arg = (x); \
   long leading_one_position; \
  __asm__ ("bsrq %1,%0" : "=r" (leading_one_position) : "rm" (__arg)); \
  63 - leading_one_position; \
})
#endif
#line 2 "../src/kernel/none/divll_pre.h"
/* Copyright (C) 2014  The PARI group.

This file is part of the PARI/GP package.

PARI/GP is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version. It is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY WHATSOEVER.

Check the License for details. You should have received a copy of it, along
with the package; see the file 'COPYING'. If not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

#undef  LOCAL_HIREMAINDER
extern ulong hiremainder;
#if defined(INLINE) && defined(__GNUC__) && !defined(DISABLE_INLINE)
#define LOCAL_HIREMAINDER register ulong hiremainder
#else
#define LOCAL_HIREMAINDER
#endif

#if defined(INLINE) && defined(__GNUC__) && !defined(DISABLE_INLINE)
INLINE ulong /* precompute inverse of n */
get_Fl_red(ulong n)
{
  LOCAL_HIREMAINDER;
  n <<= bfffo(n);
  hiremainder = ~n;
  return divll(~0UL, n);
}
#else
INLINE ulong /* precompute inverse of n */
get_Fl_red(ulong n)
{
  ulong q, oldhi = hiremainder;
  n <<= bfffo(n);
  hiremainder = ~n;
  q = divll(~0UL, n);
  hiremainder = oldhi;
  return q;
}
#endif

INLINE ulong /* requires u1 <= n, n normalised */
divll_pre_normalized(ulong u1, ulong u0, ulong n, ulong ninv, ulong *pt_r)
{
  ulong q0, q1, r;
  LOCAL_HIREMAINDER;
  LOCAL_OVERFLOW;
  q0 = mulll(ninv, u1); q1 = hiremainder;
  q0 = addll(q0, u0);
  q1 = addllx(q1+1, u1);
  r = u0 - q1 * n;
  if (r > q0)
  {
    r += n; q1--;
  }
  if (r >= n)
  {
    r -= n; q1++;
  }
  *pt_r = r; return q1;
}

INLINE ulong /* requires u1 <= n, n normalised */
remll_pre_normalized(ulong u1, ulong u0, ulong n, ulong ninv)
{
  ulong q0, q1, r;
  LOCAL_HIREMAINDER;
  LOCAL_OVERFLOW;
  q0 = mulll(ninv, u1); q1 = hiremainder;
  q0 = addll(q0, u0);
  q1 = addllx(q1, u1);
  r = u0 - (q1 + 1) * n;
  if (r >= q0)
    r += n;
  return r < n ? r : r - n;
}

INLINE ulong /* reduce <a_hi, a_lo> mod n */
remll_pre(ulong a_hi, ulong a_lo, ulong n, ulong ninv)
{
  int norm = bfffo(n);
  int bits = BITS_IN_LONG - norm;
  ulong sn = n << norm;
  if (a_hi >= n) /* reduce a_hi first */
  {
    const ulong u1 = norm ? a_hi >> bits : 0;
    const ulong u0 = a_hi << norm;
    a_hi = remll_pre_normalized(u1, u0, sn, ninv) >> norm;
  }
  /* now reduce <a_hi, a_lo> */
  {
    const ulong u1 = ((a_hi << norm) | (norm ? a_lo >> bits: 0));
    const ulong u0 =   a_lo << norm;
    return remll_pre_normalized(u1, u0, sn, ninv) >> norm;
  }
}

#if !defined(INLINE)
extern ulong divll_pre(ulong a_lo, ulong n, ulong ninv);
#else

#if defined(__GNUC__) && !defined(DISABLE_INLINE)
#define divll_pre(a, n, ninv)                                           \
__extension__ ({                                                        \
  ulong __a = (a);                                                      \
  ulong __n = (n);                                                      \
  int norm = bfffo(__n);                                                \
  int bits = BITS_IN_LONG - norm;                                       \
  ulong r, sn = __n << norm;                                            \
  const ulong u1 = ((hiremainder << norm) | (norm ? __a >> bits: 0));   \
  const ulong u0 = __a << norm;                                         \
  const ulong q = divll_pre_normalized(u1, u0, sn, ninv, &r);           \
  hiremainder = r>>norm; q;                                             \
              })

#else /* __GNUC__ */
INLINE ulong
divll_pre(ulong a_lo, ulong n, ulong ninv)
{
  int norm = bfffo(n);
  int bits = BITS_IN_LONG - norm;
  ulong r, sn = n << norm;
  const ulong u1 = ((hiremainder << norm) | (norm ? a_lo >> bits: 0));
  const ulong u0 = a_lo << norm;
  const ulong q  = divll_pre_normalized(u1, u0, sn, ninv, &r);
  hiremainder = r>>norm; return q;
}
#endif /* __GNUC__ */

#endif
