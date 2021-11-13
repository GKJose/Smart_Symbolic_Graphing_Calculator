#line 2 "../src/kernel/gmp/mp.c"
/* Copyright (C) 2002-2003  The PARI group.

This file is part of the PARI/GP package.

PARI/GP is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version. It is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY WHATSOEVER.

Check the License for details. You should have received a copy of it, along
with the package; see the file 'COPYING'. If not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

/***********************************************************************/
/**                                                                   **/
/**                               GMP KERNEL                          **/
/** BA2002Sep24                                                       **/
/***********************************************************************/
/* GMP t_INT as just like normal t_INT, just the mantissa is the other way
 * round
 *
 *   `How would you like to live in Looking-glass House, Kitty?  I
 *   wonder if they'd give you milk in there?  Perhaps Looking-glass
 *   milk isn't good to drink--But oh, Kitty! now we come to the
 *   passage.  You can just see a little PEEP of the passage in
 *   Looking-glass House, if you leave the door of our drawing-room
 *   wide open:  and it's very like our passage as far as you can see,
 *   only you know it may be quite different on beyond.  Oh, Kitty!
 *   how nice it would be if we could only get through into Looking-
 *   glass House!  I'm sure it's got, oh! such beautiful things in it!
 *
 *  Through the Looking Glass,  Lewis Carrol
 *
 *  (pityful attempt to beat GN code/comments rate)
 *  */

#include <gmp.h>
#include "pari.h"
#include "paripriv.h"
#include "../src/kernel/none/tune-gen.h"

/*We need PARI invmod renamed to invmod_pari*/
#define INVMOD_PARI

static void *pari_gmp_realloc(void *ptr, size_t old_size, size_t new_size) {
  (void)old_size; return (void *) pari_realloc(ptr,new_size);
}

static void pari_gmp_free(void *ptr, size_t old_size){
  (void)old_size; pari_free(ptr);
}

static void *(*old_gmp_malloc)(size_t new_size);
static void *(*old_gmp_realloc)(void *ptr, size_t old_size, size_t new_size);
static void (*old_gmp_free)(void *ptr, size_t old_size);

void
pari_kernel_init(void)
{
  mp_get_memory_functions (&old_gmp_malloc, &old_gmp_realloc, &old_gmp_free);
  mp_set_memory_functions((void *(*)(size_t)) pari_malloc, pari_gmp_realloc, pari_gmp_free);
}

const char *
pari_kernel_version(void)
{
#ifdef gmp_version
  return gmp_version;
#else
  return "";
#endif
}

void
pari_kernel_close(void)
{
  void *(*new_gmp_malloc)(size_t new_size);
  void *(*new_gmp_realloc)(void *ptr, size_t old_size, size_t new_size);
  void (*new_gmp_free)(void *ptr, size_t old_size);
  mp_get_memory_functions (&new_gmp_malloc, &new_gmp_realloc, &new_gmp_free);
  if (new_gmp_malloc==pari_malloc) new_gmp_malloc = old_gmp_malloc;
  if (new_gmp_realloc==pari_gmp_realloc) new_gmp_realloc = old_gmp_realloc;
  if (new_gmp_free==pari_gmp_free) new_gmp_free = old_gmp_free;
  mp_set_memory_functions(new_gmp_malloc, new_gmp_realloc, new_gmp_free);
}

#define LIMBS(x)  ((mp_limb_t *)((x)+2))
#define NLIMBS(x) (lgefint(x)-2)
/*This one is for t_REALs to emphasize they are not t_INTs*/
#define RLIMBS(x)  ((mp_limb_t *)((x)+2))
#define RNLIMBS(x) (lg(x)-2)

INLINE void
xmpn_copy(mp_limb_t *x, mp_limb_t *y, long n)
{
  while (--n >= 0) x[n]=y[n];
}

INLINE void
xmpn_mirror(mp_limb_t *x, long n)
{
  long i;
  for(i=0;i<(n>>1);i++)
  {
    ulong m=x[i];
    x[i]=x[n-1-i];
    x[n-1-i]=m;
  }
}

INLINE void
xmpn_mirrorcopy(mp_limb_t *z, mp_limb_t *x, long n)
{
  long i;
  for(i=0;i<n;i++)
    z[i]=x[n-1-i];
}

INLINE void
xmpn_zero(mp_ptr x, mp_size_t n)
{
  while (--n >= 0) x[n]=0;
}

INLINE GEN
icopy_ef(GEN x, long l)
{
  register long lx = lgefint(x);
  const GEN y = cgeti(l);

  while (--lx > 0) y[lx]=x[lx];
  return y;
}

/* NOTE: arguments of "spec" routines (muliispec, addiispec, etc.) aren't
 * GENs but pairs (long *a, long na) representing a list of digits (in basis
 * BITS_IN_LONG) : a[0], ..., a[na-1]. [ In ordre to facilitate splitting: no
 * need to reintroduce codewords ]
 * Use speci(a,na) to visualize the corresponding GEN.
 */

/***********************************************************************/
/**                                                                   **/
/**                     ADDITION / SUBTRACTION                        **/
/**                                                                   **/
/***********************************************************************/

GEN
setloop(GEN a)
{
  pari_sp av = avma - 2 * sizeof(long);
  (void)cgetg(lgefint(a) + 3, t_VECSMALL);
  return icopy_avma(a, av); /* two cells of extra space after a */
}

/* we had a = setloop(?), then some incloops. Reset a to b */
GEN
resetloop(GEN a, GEN b) {
  a[0] = evaltyp(t_INT) | evallg(lgefint(b));
  affii(b, a); return a;
}

/* assume a > 0, initialized by setloop. Do a++ */
static GEN
incpos(GEN a)
{
  long i, l = lgefint(a);
  for (i=2; i<l; i++)
    if (++uel(a,i)) return a;
  a[l] = 1; l++;
  a[0]=evaltyp(t_INT) | _evallg(l);
  a[1]=evalsigne(1) | evallgefint(l);
  return a;
}

/* assume a < 0, initialized by setloop. Do a++ */
static GEN
incneg(GEN a)
{
  long i, l = lgefint(a)-1;
  if (uel(a,2)--)
  {
    if (!a[l]) /* implies l = 2 */
    {
      a[0] = evaltyp(t_INT) | _evallg(2);
      a[1] = evalsigne(0) | evallgefint(2);
    }
    return a;
  }
  for (i=3; i<=l; i++)
    if (uel(a,i)--) break;
  if (!a[l])
  {
    a[0] = evaltyp(t_INT) | _evallg(l);
    a[1] = evalsigne(-1) | evallgefint(l);
  }
  return a;
}

/* assume a initialized by setloop. Do a++ */
GEN
incloop(GEN a)
{
  switch(signe(a))
  {
    case 0:
      a[0]=evaltyp(t_INT) | _evallg(3);
      a[1]=evalsigne(1) | evallgefint(3);
      a[2]=1; return a;
    case -1: return incneg(a);
    default: return incpos(a);
  }
}

INLINE GEN
adduispec(ulong s, GEN x, long nx)
{
  GEN  zd;
  long lz;

  if (nx == 1) return adduu(uel(x,0), s);
  lz = nx+3; zd = cgeti(lz);
  if (mpn_add_1(LIMBS(zd),(mp_limb_t *)x,nx,s))
    zd[lz-1]=1;
  else
    lz--;
  zd[1] = evalsigne(1) | evallgefint(lz);
  return zd;
}

GEN
adduispec_offset(ulong s, GEN x, long offset, long nx)
{
  GEN xd=x+2+offset;
  while (nx && *(xd+nx-1)==0) nx--;
  if (!nx) return utoi(s);
  return adduispec(s,xd,nx);
}

INLINE GEN
addiispec(GEN x, GEN y, long nx, long ny)
{
  GEN zd;
  long lz;

  if (nx < ny) swapspec(x,y, nx,ny);
  if (ny == 1) return adduispec(*y,x,nx);
  lz = nx+3; zd = cgeti(lz);

  if (mpn_add(LIMBS(zd),(mp_limb_t *)x,nx,(mp_limb_t *)y,ny))
    zd[lz-1]=1;
  else
    lz--;

  zd[1] = evalsigne(1) | evallgefint(lz);
  return zd;
}

/* assume x >= y */
INLINE GEN
subiuspec(GEN x, ulong s, long nx)
{
  GEN zd;
  long lz;

  if (nx == 1) return utoi(x[0] - s);

  lz = nx + 2; zd = cgeti(lz);
  mpn_sub_1 (LIMBS(zd), (mp_limb_t *)x, nx, s);
  if (! zd[lz - 1]) { --lz; }

  zd[1] = evalsigne(1) | evallgefint(lz);
  return zd;
}

/* assume x > y */
INLINE GEN
subiispec(GEN x, GEN y, long nx, long ny)
{
  GEN zd;
  long lz;
  if (ny==1) return subiuspec(x,*y,nx);
  lz = nx+2; zd = cgeti(lz);

  mpn_sub (LIMBS(zd), (mp_limb_t *)x, nx, (mp_limb_t *)y, ny);
  while (lz >= 3 && zd[lz - 1] == 0) { lz--; }

  zd[1] = evalsigne(1) | evallgefint(lz);
  return zd;
}

static void
roundr_up_ip(GEN x, long l)
{
  long i = l;
  for(;;)
  {
    if (++((ulong*)x)[--i]) break;
    if (i == 2) { x[2] = HIGHBIT; shiftr_inplace(x, 1); break; }
  }
}

void
affir(GEN x, GEN y)
{
  const long s = signe(x), ly = lg(y);
  long lx, sh, i;

  if (!s)
  {
    y[1] = evalexpo(-bit_accuracy(ly));
    return;
  }
  lx = lgefint(x); sh = bfffo(*int_MSW(x));
  y[1] = evalsigne(s) | evalexpo(bit_accuracy(lx)-sh-1);
  if (sh) {
    if (lx <= ly)
    {
      for (i=lx; i<ly; i++) y[i]=0;
      mpn_lshift(LIMBS(y),LIMBS(x),lx-2,sh);
      xmpn_mirror(LIMBS(y),lx-2);
      return;
    }
    mpn_lshift(LIMBS(y),LIMBS(x)+lx-ly,ly-2,sh);
    uel(y,2) |= uel(x,lx-ly+1) >> (BITS_IN_LONG-sh);
    xmpn_mirror(LIMBS(y),ly-2);
    /* lx > ly: round properly */
    if ((uel(x,lx-ly+1)<<sh) & HIGHBIT) roundr_up_ip(y, ly);
  }
  else {
    GEN xd=int_MSW(x);
    if (lx <= ly)
    {
      for (i=2; i<lx; i++,xd=int_precW(xd)) y[i]=*xd;
      for (   ; i<ly; i++) y[i]=0;
      return;
    }
    for (i=2; i<ly; i++,xd=int_precW(xd)) y[i]=*xd;
    /* lx > ly: round properly */
    if (uel(x,lx-ly+1) & HIGHBIT) roundr_up_ip(y, ly);
  }
}

INLINE GEN
shiftispec(GEN x, long nx, long n)
{
  long ny,m;
  GEN yd, y;

  if (!n) return icopyspec(x, nx);

  if (n > 0)
  {
    long d = dvmdsBIL(n, &m);
    long i;

    ny = nx + d + (m!=0);
    y = cgeti(ny + 2); yd = y + 2;
    for (i=0; i<d; i++) yd[i] = 0;

    if (!m) xmpn_copy((mp_limb_t *) (yd + d), (mp_limb_t *) x, nx);
    else
    {
      ulong carryd = mpn_lshift((mp_limb_t *) (yd + d), (mp_limb_t *) x, nx, m);
      if (carryd) yd[ny - 1] = carryd;
      else ny--;
    }
  }
  else
  {
    long d = dvmdsBIL(-n, &m);

    ny = nx - d;
    if (ny < 1) return gen_0;
    y = cgeti(ny + 2); yd = y + 2;

    if (!m) xmpn_copy((mp_limb_t *) yd, (mp_limb_t *) (x + d), nx - d);
    else
    {
      mpn_rshift((mp_limb_t *) yd, (mp_limb_t *) (x + d), nx - d, m);
      if (yd[ny - 1] == 0)
      {
        if (ny == 1) return gc_const((pari_sp)(yd + 1), gen_0);
        ny--;
      }
    }
  }
  y[1] = evalsigne(1)|evallgefint(ny + 2);
  return y;
}

GEN
mantissa2nr(GEN x, long n)
{
  long ly, i, m, s = signe(x), lx = lg(x);
  GEN y;
  if (!s) return gen_0;
  if (!n)
  {
    y = cgeti(lx);
    y[1] = evalsigne(s) | evallgefint(lx);
    xmpn_mirrorcopy(LIMBS(y),RLIMBS(x),lx-2);
    return y;
  }
  if (n > 0)
  {
    GEN z = (GEN)avma;
    long d = dvmdsBIL(n, &m);

    ly = lx+d; y = new_chunk(ly);
    for ( ; d; d--) *--z = 0;
    if (!m) for (i=2; i<lx; i++) y[i]=x[i];
    else
    {
      register const ulong sh = BITS_IN_LONG - m;
      shift_left(y,x, 2,lx-1, 0,m);
      i = uel(x,2) >> sh;
      /* Extend y on the left? */
      if (i) { ly++; y = new_chunk(1); y[2] = i; }
    }
  }
  else
  {
    ly = lx - dvmdsBIL(-n, &m);
    if (ly<3) return gen_0;
    y = new_chunk(ly);
    if (m) {
      shift_right(y,x, 2,ly, 0,m);
      if (y[2] == 0)
      {
        if (ly==3) return gc_const((pari_sp)(y+3), gen_0);
        ly--; set_avma((pari_sp)(++y));
      }
    } else {
      for (i=2; i<ly; i++) y[i]=x[i];
    }
  }
  xmpn_mirror(LIMBS(y),ly-2);
  y[1] = evalsigne(s)|evallgefint(ly);
  y[0] = evaltyp(t_INT)|evallg(ly); return y;
}

GEN
truncr(GEN x)
{
  long s, e, d, m, i;
  GEN y;
  if ((s=signe(x)) == 0 || (e=expo(x)) < 0) return gen_0;
  d = nbits2lg(e+1); m = remsBIL(e);
  if (d > lg(x)) pari_err_PREC( "truncr (precision loss in truncation)");

  y=cgeti(d); y[1] = evalsigne(s) | evallgefint(d);
  if (++m == BITS_IN_LONG)
    for (i=2; i<d; i++) y[d-i+1]=x[i];
  else
  {
    GEN z=cgeti(d);
    for (i=2; i<d; i++) z[d-i+1]=x[i];
    mpn_rshift(LIMBS(y),LIMBS(z),d-2,BITS_IN_LONG-m);
    set_avma((pari_sp)y);
  }
  return y;
}

/* integral part */
GEN
floorr(GEN x)
{
  long e, d, m, i, lx;
  GEN y;
  if (signe(x) >= 0) return truncr(x);
  if ((e=expo(x)) < 0) return gen_m1;
  d = nbits2lg(e+1); m = remsBIL(e);
  lx=lg(x); if (d>lx) pari_err_PREC( "floorr (precision loss in truncation)");
  y = cgeti(d+1);
  if (++m == BITS_IN_LONG)
  {
    for (i=2; i<d; i++) y[d-i+1]=x[i];
    i=d; while (i<lx && !x[i]) i++;
    if (i==lx) goto END;
  }
  else
  {
    GEN z=cgeti(d);
    for (i=2; i<d; i++) z[d-i+1]=x[i];
    mpn_rshift(LIMBS(y),LIMBS(z),d-2,BITS_IN_LONG-m);
    if (uel(x,d-1)<<m == 0)
    {
      i=d; while (i<lx && !x[i]) i++;
      if (i==lx) goto END;
    }
  }
  if (mpn_add_1(LIMBS(y),LIMBS(y),d-2,1))
    y[d++]=1;
END:
  y[1] = evalsigne(-1) | evallgefint(d);
  return y;
}

INLINE int
cmpiispec(GEN x, GEN y, long lx, long ly)
{
  if (lx < ly) return -1;
  if (lx > ly) return  1;
  return mpn_cmp((mp_limb_t*)x,(mp_limb_t*)y, lx);
}

INLINE int
equaliispec(GEN x, GEN y, long lx, long ly)
{
  if (lx != ly) return 0;
  return !mpn_cmp((mp_limb_t*)x,(mp_limb_t*)y, lx);
}

/***********************************************************************/
/**                                                                   **/
/**                          MULTIPLICATION                           **/
/**                                                                   **/
/***********************************************************************/
/* assume ny > 0 */
INLINE GEN
muluispec(ulong x, GEN y, long ny)
{
  if (ny == 1)
    return muluu(x, *y);
  else
  {
    long lz = ny+3;
    GEN z = cgeti(lz);
    ulong hi = mpn_mul_1 (LIMBS(z), (mp_limb_t *)y, ny, x);
    if (hi) { z[lz - 1] = hi; } else lz--;
    z[1] = evalsigne(1) | evallgefint(lz);
    return z;
  }
}

/* a + b*|y| */
GEN
addumului(ulong a, ulong b, GEN y)
{
  GEN z;
  long i, lz;
  ulong hi;
  if (!b || !signe(y)) return utoi(a);
  lz = lgefint(y)+1;
  z = cgeti(lz);
  z[2]=a;
  for(i=3;i<lz;i++) z[i]=0;
  hi=mpn_addmul_1(LIMBS(z), LIMBS(y), NLIMBS(y), b);
  if (hi) z[lz-1]=hi; else lz--;
  z[1] = evalsigne(1) | evallgefint(lz);
  return gc_const((pari_sp)z, z);
}

/***********************************************************************/
/**                                                                   **/
/**                          DIVISION                                 **/
/**                                                                   **/
/***********************************************************************/

ulong
umodiu(GEN y, ulong x)
{
  long sy=signe(y);
  ulong hi;
  if (!x) pari_err_INV("umodiu",gen_0);
  if (!sy) return 0;
  hi = mpn_mod_1(LIMBS(y),NLIMBS(y),x);
  if (!hi) return 0;
  return (sy > 0)? hi: x - hi;
}

/* return |y| \/ x */
GEN
absdiviu_rem(GEN y, ulong x, ulong *rem)
{
  long ly;
  GEN z;

  if (!x) pari_err_INV("absdiviu_rem",gen_0);
  if (!signe(y)) { *rem = 0; return gen_0; }

  ly = lgefint(y);
  if (ly == 3 && (ulong)x > uel(y,2)) { *rem = uel(y,2); return gen_0; }

  z = cgeti(ly);
  *rem = mpn_divrem_1(LIMBS(z), 0, LIMBS(y), NLIMBS(y), x);
  if (z [ly - 1] == 0) ly--;
  z[1] = evallgefint(ly) | evalsigne(1);
  return z;
}

GEN
divis_rem(GEN y, long x, long *rem)
{
  long sy=signe(y),ly,s;
  GEN z;

  if (!x) pari_err_INV("divis_rem",gen_0);
  if (!sy) { *rem = 0; return gen_0; }
  if (x<0) { s = -sy; x = -x; } else s = sy;

  ly = lgefint(y);
  if (ly == 3 && (ulong)x > uel(y,2)) { *rem = itos(y); return gen_0; }

  z = cgeti(ly);
  *rem = mpn_divrem_1(LIMBS(z), 0, LIMBS(y), NLIMBS(y), x);
  if (sy<0) *rem = -  *rem;
  if (z[ly - 1] == 0) ly--;
  z[1] = evallgefint(ly) | evalsigne(s);
  return z;
}

GEN
divis(GEN y, long x)
{
  long sy=signe(y),ly,s;
  GEN z;

  if (!x) pari_err_INV("divis",gen_0);
  if (!sy) return gen_0;
  if (x<0) { s = -sy; x = -x; } else s=sy;

  ly = lgefint(y);
  if (ly == 3 && (ulong)x > uel(y,2)) return gen_0;

  z = cgeti(ly);
  (void)mpn_divrem_1(LIMBS(z), 0, LIMBS(y), NLIMBS(y), x);
  if (z[ly - 1] == 0) ly--;
  z[1] = evallgefint(ly) | evalsigne(s);
  return z;
}

/* We keep llx bits of x and lly bits of y*/
static GEN
divrr_with_gmp(GEN x, GEN y)
{
  long lx=RNLIMBS(x),ly=RNLIMBS(y);
  long lw=minss(lx,ly);
  long lly=minss(lw+1,ly);
  GEN  w=cgetr(lw+2);
  long lu=lw+lly;
  long llx=minss(lu,lx);
  mp_limb_t *u=(mp_limb_t *)new_chunk(lu);
  mp_limb_t *z=(mp_limb_t *)new_chunk(lly);
  mp_limb_t *q,*r;
  long e=expo(x)-expo(y);
  long sx=signe(x),sy=signe(y);
  xmpn_mirrorcopy(z,RLIMBS(y),lly);
  xmpn_mirrorcopy(u+lu-llx,RLIMBS(x),llx);
  xmpn_zero(u,lu-llx);
  q = (mp_limb_t *)new_chunk(lw+1);
  r = (mp_limb_t *)new_chunk(lly);

  mpn_tdiv_qr(q,r,0,u,lu,z,lly);

  /*Round up: This is not exactly correct we should test 2*r>z*/
  if (uel(r,lly-1) > (uel(z,lly-1)>>1))
    mpn_add_1(q,q,lw+1,1);

  xmpn_mirrorcopy(RLIMBS(w),q,lw);

  if (q[lw] == 0) e--;
  else if (q[lw] == 1) { shift_right(w,w, 2,lw+2, 1,1); }
  else { w[2] = HIGHBIT; e++; }
  if (sy < 0) sx = -sx;
  w[1] = evalsigne(sx) | evalexpo(e);
  return gc_const((pari_sp)w, w);
}

/* We keep llx bits of x and lly bits of y*/
static GEN
divri_with_gmp(GEN x, GEN y)
{
  long llx=RNLIMBS(x),ly=NLIMBS(y);
  long lly=minss(llx+1,ly);
  GEN  w=cgetr(llx+2);
  long lu=llx+lly, ld=ly-lly;
  mp_limb_t *u=(mp_limb_t *)new_chunk(lu);
  mp_limb_t *z=(mp_limb_t *)new_chunk(lly);
  mp_limb_t *q,*r;
  long sh=bfffo(y[ly+1]);
  long e=expo(x)-expi(y);
  long sx=signe(x),sy=signe(y);
  if (sh) mpn_lshift(z,LIMBS(y)+ld,lly,sh);
  else xmpn_copy(z,LIMBS(y)+ld,lly);
  xmpn_mirrorcopy(u+lu-llx,RLIMBS(x),llx);
  xmpn_zero(u,lu-llx);
  q = (mp_limb_t *)new_chunk(llx+1);
  r = (mp_limb_t *)new_chunk(lly);

  mpn_tdiv_qr(q,r,0,u,lu,z,lly);

  /*Round up: This is not exactly correct we should test 2*r>z*/
  if (uel(r,lly-1) > (uel(z,lly-1)>>1))
    mpn_add_1(q,q,llx+1,1);

  xmpn_mirrorcopy(RLIMBS(w),q,llx);

  if (q[llx] == 0) e--;
  else if (q[llx] == 1) { shift_right(w,w, 2,llx+2, 1,1); }
  else { w[2] = HIGHBIT; e++; }
  if (sy < 0) sx = -sx;
  w[1] = evalsigne(sx) | evalexpo(e);
  return gc_const((pari_sp)w, w);
}

GEN
divri(GEN x, GEN y)
{
  long  s = signe(y);

  if (!s) pari_err_INV("divri",gen_0);
  if (!signe(x)) return real_0_bit(expo(x) - expi(y));
  if (!is_bigint(y)) {
    GEN z = divru(x, y[2]);
    if (s < 0) togglesign(z);
    return z;
  }
  return divri_with_gmp(x,y);
}

GEN
divrr(GEN x, GEN y)
{
  long sx=signe(x), sy=signe(y), lx,ly,lr,e,i,j;
  ulong y0,y1;
  GEN r, r1;

  if (!sy) pari_err_INV("divrr",y);
  e = expo(x) - expo(y);
  if (!sx) return real_0_bit(e);
  if (sy<0) sx = -sx;

  lx=lg(x); ly=lg(y);
  if (ly==3)
  {
    ulong k = x[2], l = (lx>3)? x[3]: 0;
    LOCAL_HIREMAINDER;
    if (k < uel(y,2)) e--;
    else
    {
      l >>= 1; if (k&1) l |= HIGHBIT;
      k >>= 1;
    }
    hiremainder = k; k = divll(l,y[2]);
    if (hiremainder > (uel(y,2) >> 1) && !++k) { k = HIGHBIT; e++; }
    r = cgetr(3);
    r[1] = evalsigne(sx) | evalexpo(e);
    r[2] = k; return r;
  }

  if (ly>=DIVRR_GMP_LIMIT)
    return divrr_with_gmp(x,y);

  lr = minss(lx,ly); r = new_chunk(lr);
  r1 = r-1;
  r1[1] = 0; for (i=2; i<lr; i++) r1[i]=x[i];
  r1[lr] = (lx>ly)? x[lr]: 0;
  y0 = y[2]; y1 = y[3];
  for (i=0; i<lr-1; i++)
  { /* r1 = r + (i-1), OK up to r1[2] (accesses at most r[lr]) */
    ulong k, qp;
    LOCAL_HIREMAINDER;
    LOCAL_OVERFLOW;

    if (uel(r1,1) == y0) { qp = ULONG_MAX; k = addll(y0,r1[2]); }
    else
    {
      if (uel(r1,1) > y0) /* can't happen if i=0 */
      {
        GEN y1 = y+1;
        j = lr-i; r1[j] = subll(r1[j],y1[j]);
        for (j--; j>0; j--) r1[j] = subllx(r1[j],y1[j]);
        j=i; do uel(r,--j)++; while (j && !r[j]);
      }
      hiremainder = r1[1]; overflow = 0;
      qp = divll(r1[2],y0); k = hiremainder;
    }
    j = lr-i+1;
    if (!overflow)
    {
      long k3, k4;
      k3 = mulll(qp,y1);
      if (j == 3) /* i = lr - 2 maximal, r1[3] undefined -> 0 */
        k4 = subll(hiremainder,k);
      else
      {
        k3 = subll(k3, r1[3]);
        k4 = subllx(hiremainder,k);
      }
      while (!overflow && k4) { qp--; k3=subll(k3,y1); k4=subllx(k4,y0); }
    }
    if (j<ly) (void)mulll(qp,y[j]); else { hiremainder = 0 ; j = ly; }
    for (j--; j>1; j--)
    {
      r1[j] = subll(r1[j], addmul(qp,y[j]));
      hiremainder += overflow;
    }
    if (uel(r1,1) != hiremainder)
    {
      if (uel(r1,1) < hiremainder)
      {
        qp--;
        j = lr-i-(lr-i>=ly); r1[j] = addll(r1[j], y[j]);
        for (j--; j>1; j--) r1[j] = addllx(r1[j], y[j]);
      }
      else
      {
        uel(r1,1) -= hiremainder;
        while (r1[1])
        {
          qp++; if (!qp) { j=i; do uel(r,--j)++; while (j && !r[j]); }
          j = lr-i-(lr-i>=ly); r1[j] = subll(r1[j],y[j]);
          for (j--; j>1; j--) r1[j] = subllx(r1[j],y[j]);
          uel(r1,1) -= overflow;
        }
      }
    }
    *++r1 = qp;
  }
  /* i = lr-1 */
  /* round correctly */
  if (uel(r1,1) > (y0>>1))
  {
    j=i; do uel(r,--j)++; while (j && !r[j]);
  }
  r1 = r-1; for (j=i; j>=2; j--) r[j]=r1[j];
  if (r[0] == 0) e--;
  else if (r[0] == 1) { shift_right(r,r, 2,lr, 1,1); }
  else { /* possible only when rounding up to 0x2 0x0 ... */
    r[2] = (long)HIGHBIT; e++;
  }
  r[0] = evaltyp(t_REAL)|evallg(lr);
  r[1] = evalsigne(sx) | evalexpo(e);
  return r;
}

/* Integer division x / y: such that sign(r) = sign(x)
 *   if z = ONLY_REM return remainder, otherwise return quotient
 *   if z != NULL set *z to remainder
 *   *z is the last object on stack (and thus can be disposed of with cgiv
 *   instead of gerepile)
 * If *z is zero, we put gen_0 here and no copy.
 * space needed: lx + ly
 */
GEN
dvmdii(GEN x, GEN y, GEN *z)
{
  long sx=signe(x),sy=signe(y);
  long lx, ly, lq;
  pari_sp av;
  GEN r,q;

  if (!sy) pari_err_INV("dvmdii",y);
  if (!sx)
  {
    if (!z || z == ONLY_REM) return gen_0;
    *z=gen_0; return gen_0;
  }
  lx=lgefint(x);
  ly=lgefint(y); lq=lx-ly;
  if (lq <= 0)
  {
    if (lq == 0)
    {
      long s=mpn_cmp(LIMBS(x),LIMBS(y),NLIMBS(x));
      if (s>0) goto DIVIDE;
      if (s==0)
      {
        if (z == ONLY_REM) return gen_0;
        if (z) *z = gen_0;
        if (sx < 0) sy = -sy;
        return stoi(sy);
      }
    }
    if (z == ONLY_REM) return icopy(x);
    if (z) *z = icopy(x);
    return gen_0;
  }
DIVIDE: /* quotient is nonzero */
  av=avma; if (sx<0) sy = -sy;
  if (ly==3)
  {
    ulong lq = lx;
    ulong si;
    q  = cgeti(lq);
    si = mpn_divrem_1(LIMBS(q), 0, LIMBS(x), NLIMBS(x), y[2]);
    if (q[lq - 1] == 0) lq--;
    if (z == ONLY_REM)
    {
      if (!si) return gc_const(av, gen_0);
      set_avma(av); r = cgeti(3);
      r[1] = evalsigne(sx) | evallgefint(3);
      r[2] = si; return r;
    }
    q[1] = evalsigne(sy) | evallgefint(lq);
    if (!z) return q;
    if (!si) { *z=gen_0; return q; }
    r=cgeti(3);
    r[1] = evalsigne(sx) | evallgefint(3);
    r[2] = si; *z=r; return q;
  }
  if (z == ONLY_REM)
  {
    ulong lr = lgefint(y);
    ulong lq = lgefint(x)-lgefint(y)+3;
    GEN r = cgeti(lr);
    GEN q = cgeti(lq);
    mpn_tdiv_qr(LIMBS(q), LIMBS(r),0, LIMBS(x), NLIMBS(x), LIMBS(y), NLIMBS(y));
    if (!r[lr - 1])
    {
      while(lr>2 && !r[lr - 1]) lr--;
      if (lr == 2) return gc_const(av, gen_0); /* exact division */
    }
    r[1] = evalsigne(sx) | evallgefint(lr);
    return gc_const((pari_sp)r, r);
  }
  else
  {
    ulong lq = lgefint(x)-lgefint(y)+3;
    ulong lr = lgefint(y);
    GEN q = cgeti(lq);
    GEN r = cgeti(lr);
    mpn_tdiv_qr(LIMBS(q), LIMBS(r),0, LIMBS(x), NLIMBS(x), LIMBS(y), NLIMBS(y));
    if (q[lq - 1] == 0) lq--;
    q[1] = evalsigne(sy) | evallgefint(lq);
    if (!z) return gc_const((pari_sp)q, q);
    if (!r[lr - 1])
    {
      while(lr>2 && !r[lr - 1]) lr--;
      if (lr == 2) { *z = gen_0; return gc_const((pari_sp)q, q); } /* exact */
    }
    r[1] = evalsigne(sx) | evallgefint(lr);
    *z = gc_const((pari_sp)r, r); return q;
  }
}

/* Montgomery reduction.
 * N has k words, assume T >= 0 has less than 2k.
 * Return res := T / B^k mod N, where B = 2^BIL
 * such that 0 <= res < T/B^k + N  and  res has less than k words */
GEN
red_montgomery(GEN T, GEN N, ulong inv)
{
  pari_sp av;
  GEN Te, Td, Ne, Nd, scratch;
  ulong i, j, m, t, d, k = NLIMBS(N);
  int carry;
  LOCAL_HIREMAINDER;
  LOCAL_OVERFLOW;

  if (k == 0) return gen_0;
  d = NLIMBS(T); /* <= 2*k */
  if (d == 0) return gen_0;
#ifdef DEBUG
  if (d > 2*k) pari_err_BUG("red_montgomery");
#endif
  if (k == 1)
  { /* as below, special cased for efficiency */
    ulong n = uel(N,2);
    if (d == 1) {
      hiremainder = uel(T,2);
      m = hiremainder * inv;
      (void)addmul(m, n); /* t + m*n = 0 */
      return utoi(hiremainder);
    } else { /* d = 2 */
      hiremainder = uel(T,2);
      m = hiremainder * inv;
      (void)addmul(m, n); /* t + m*n = 0 */
      t = addll(hiremainder, uel(T,3));
      if (overflow) t -= n; /* t > n doesn't fit in 1 word */
      return utoi(t);
    }
  }
  /* assume k >= 2 */
  av = avma; scratch = new_chunk(k<<1); /* >= k + 2: result fits */

  /* copy T to scratch space (pad with zeroes to 2k words) */
  Td = scratch;
  Te = T + 2;
  for (i=0; i < d     ; i++) *Td++ = *Te++;
  for (   ; i < (k<<1); i++) *Td++ = 0;

  Te = scratch - 1; /* 1 beyond end of current T mantissa (in scratch) */
  Ne = N + 1;       /* 1 beyond end of N mantissa */

  carry = 0;
  for (i=0; i<k; i++) /* set T := T/B nod N, k times */
  {
    Td = Te; /* one beyond end of (new) T mantissa */
    Nd = Ne;
    hiremainder = *++Td;
    m = hiremainder * inv; /* solve T + m N = O(B) */

    /* set T := (T + mN) / B */
    Te = Td;
    (void)addmul(m, *++Nd); /* = 0 */
    for (j=1; j<k; j++)
    {
      t = addll(addmul(m, *++Nd), *++Td);
      *Td = t;
      hiremainder += overflow;
    }
    t = addll(hiremainder, *++Td); *Td = t + carry;
    carry = (overflow || (carry && *Td == 0));
  }
  if (carry)
  { /* Td > N overflows (k+1 words), set Td := Td - N */
    GEN NE = N + k+1;
    Td = Te;
    Nd = Ne;
    t = subll(*++Td, *++Nd); *Td = t;
    while (Nd < NE) { t = subllx(*++Td, *++Nd); *Td = t; }
  }

  /* copy result */
  Td = (GEN)av - 1; /* *Td = high word of final result */
  while (*Td == 0 && Te < Td) Td--; /* strip leading 0s */
  k = Td - Te; if (!k) return gc_const(av, gen_0);
  Td = (GEN)av - k; /* will write mantissa there */
  (void)memmove(Td, Te+1, k*sizeof(long));
  Td -= 2;
  Td[0] = evaltyp(t_INT) | evallg(k+2);
  Td[1] = evalsigne(1) | evallgefint(k+2);
#ifdef DEBUG
{
  long l = NLIMBS(N), s = BITS_IN_LONG*l;
  GEN R = int2n(s);
  GEN res = remii(mulii(T, Fp_inv(R, N)), N);
  if (k > lgefint(N)
    || !equalii(remii(Td,N),res)
    || cmpii(Td, addii(shifti(T, -s), N)) >= 0) pari_err_BUG("red_montgomery");
}
#endif
  return gc_const((pari_sp)Td, Td);
}

/* EXACT INTEGER DIVISION */

/* use undocumented GMP interface */
static void
GEN2mpz(mpz_t X, GEN x)
{
  long l = lgefint(x)-2;
  X->_mp_alloc = l;
  X->_mp_size = signe(x) > 0? l: -l;
  X->_mp_d = LIMBS(x);
}
static void
mpz2GEN(GEN z, mpz_t Z)
{
  long l = Z->_mp_size;
  z[1] = evalsigne(l > 0? 1: -1) | evallgefint(labs(l)+2);
}

#ifdef mpn_divexact_1
static GEN
diviuexact_i(GEN x, ulong y)
{
  long l = lgefint(x);
  GEN z = cgeti(l);
  mpn_divexact_1(LIMBS(z), LIMBS(x), NLIMBS(x), y);
  if (z[l-1] == 0) l--;
  z[1] = evallgefint(l) | evalsigne(signe(x));
  return z;
}
#elif 1 && !defined(_WIN64) /* mpz_divexact_ui is not LLP64 friendly   */
                            /* assume y != 0 and the division is exact */
static GEN
diviuexact_i(GEN x, ulong y)
{
  long l = lgefint(x);
  GEN z = cgeti(l);
  mpz_t X, Z;
  GEN2mpz(X, x);
  Z->_mp_alloc = l-2;
  Z->_mp_size  = l-2;
  Z->_mp_d = LIMBS(z);
  mpz_divexact_ui(Z, X, y);
  mpz2GEN(z, Z); return z;
}
#else
/* assume y != 0 and the division is exact */
static GEN
diviuexact_i(GEN x, ulong y)
{
  /*TODO: implement true exact division.*/
  return divii(x,utoi(y));
}
#endif

GEN
diviuexact(GEN x, ulong y)
{
  GEN z;
  if (!signe(x)) return gen_0;
  z = diviuexact_i(x, y);
  if (lgefint(z) == 2) pari_err_OP("exact division", x, utoi(y));
  return z;
}

/* Find z such that x=y*z, knowing that y | x (unchecked) */
GEN
diviiexact(GEN x, GEN y)
{
  GEN z;
  if (!signe(y)) pari_err_INV("diviiexact",y);
  if (!signe(x)) return gen_0;
  if (lgefint(y) == 3)
  {
    z = diviuexact_i(x, y[2]);
    if (signe(y) < 0) togglesign(z);
  }
  else
  {
    long l = lgefint(x);
    mpz_t X, Y, Z;
    z = cgeti(l);
    GEN2mpz(X, x);
    GEN2mpz(Y, y);
    Z->_mp_alloc = l-2;
    Z->_mp_size  = l-2;
    Z->_mp_d = LIMBS(z);
    mpz_divexact(Z, X, Y);
    mpz2GEN(z, Z);
  }
  if (lgefint(z) == 2) pari_err_OP("exact division", x, y);
  return z;
}

/* assume yz != and yz | x */
GEN
diviuuexact(GEN x, ulong y, ulong z)
{
  long tmp[4];
  ulong t;
  LOCAL_HIREMAINDER;
  t = mulll(y, z);
  if (!hiremainder) return diviuexact(x, t);
  tmp[0] = evaltyp(t_INT)|_evallg(4);
  tmp[1] = evalsigne(1)|evallgefint(4);
  tmp[2] = t;
  tmp[3] = hiremainder;
  return diviiexact(x, tmp);
}

/********************************************************************/
/**                                                                **/
/**               INTEGER MULTIPLICATION                           **/
/**                                                                **/
/********************************************************************/

/* nx >= ny = num. of digits of x, y (not GEN, see mulii) */
GEN
muliispec(GEN x, GEN y, long nx, long ny)
{
  GEN zd;
  long lz;
  ulong hi;

  if (nx < ny) swapspec(x,y, nx,ny);
  if (!ny) return gen_0;
  if (ny == 1) return muluispec((ulong)*y, x, nx);

  lz = nx+ny+2;
  zd = cgeti(lz);
  hi = mpn_mul(LIMBS(zd), (mp_limb_t *)x, nx, (mp_limb_t *)y, ny);
  if (!hi) lz--;
  /*else zd[lz-1]=hi; GH tell me it is not necessary.*/

  zd[1] = evalsigne(1) | evallgefint(lz);
  return zd;
}
GEN
muluui(ulong x, ulong y, GEN z)
{
  long t, s = signe(z);
  GEN r;
  LOCAL_HIREMAINDER;

  if (!x || !y || !signe(z)) return gen_0;
  t = mulll(x,y);
  if (!hiremainder)
    r = muluispec(t, z+2, lgefint(z)-2);
  else
  {
    long tmp[2];
    tmp[1] = hiremainder;
    tmp[0] = t;
    r = muliispec(z+2,tmp, lgefint(z)-2, 2);
  }
  setsigne(r,s); return r;
}

GEN
sqrispec(GEN x, long nx)
{
  GEN zd;
  long lz;

  if (!nx) return gen_0;
  if (nx==1) return sqru(*x);

  lz = (nx<<1)+2;
  zd = cgeti(lz);
#ifdef mpn_sqr
  mpn_sqr(LIMBS(zd), (mp_limb_t *)x, nx);
#else
  mpn_mul_n(LIMBS(zd), (mp_limb_t *)x, (mp_limb_t *)x, nx);
#endif
  if (zd[lz-1]==0) lz--;

  zd[1] = evalsigne(1) | evallgefint(lz);
  return zd;
}

INLINE GEN
sqrispec_mirror(GEN x, long nx)
{
  GEN cx=new_chunk(nx);
  GEN z;
  xmpn_mirrorcopy((mp_limb_t *)cx,(mp_limb_t *)x,nx);
  z=sqrispec(cx, nx);
  xmpn_mirror(LIMBS(z), NLIMBS(z));
  return z;
}

/* leaves garbage on the stack. */
INLINE GEN
muliispec_mirror(GEN x, GEN y, long nx, long ny)
{
  GEN cx, cy, z;
  long s = 0;
  while (nx && x[nx-1]==0) { nx--; s++; }
  while (ny && y[ny-1]==0) { ny--; s++; }
  cx=new_chunk(nx); cy=new_chunk(ny);
  xmpn_mirrorcopy((mp_limb_t *)cx,(mp_limb_t *)x,nx);
  xmpn_mirrorcopy((mp_limb_t *)cy,(mp_limb_t *)y,ny);
  z =  nx>=ny ? muliispec(cx, cy, nx, ny): muliispec(cy, cx, ny, nx);
  if (s)
  {
    long i, lz = lgefint(z) + s;
    (void)new_chunk(s);
    z -= s;
    for (i=0; i<s; i++) z[2+i]=0;
    z[1] = evalsigne(1) | evallgefint(lz);
    z[0] = evaltyp(t_INT) | evallg(lz);
  }
  xmpn_mirror(LIMBS(z), NLIMBS(z));
  return z;
}

/* x % (2^n), assuming n >= 0 */
GEN
remi2n(GEN x, long n)
{
  ulong hi;
  long l, k, lx, ly, sx = signe(x);
  GEN z, xd, zd;

  if (!sx || !n) return gen_0;

  k = dvmdsBIL(n, &l);
  lx = lgefint(x);
  if (lx < k+3) return icopy(x);

  xd = x + (2 + k);
  /* x = |k|...|1|#|... : copy the last l bits of # and the first k words
   *              ^--- initial xd  */
  hi = ((ulong)*xd) & ((1UL<<l)-1); /* last l bits of # = top bits of result */
  if (!hi)
  { /* strip leading zeroes from result */
    xd--; while (k && !*xd) { k--; xd--; }
    if (!k) return gen_0;
    ly = k+2;
  }
  else
    ly = k+3;

  zd = z = cgeti(ly);
  *++zd = evalsigne(sx) | evallgefint(ly);
  xd = x+1; for ( ;k; k--) *++zd = *++xd;
  if (hi) *++zd = hi;
  return z;
}

/********************************************************************/
/**                                                                **/
/**                      INTEGER SQUARE ROOT                       **/
/**                                                                **/
/********************************************************************/

/* Return S (and set R) s.t S^2 + R = N, 0 <= R <= 2S.
 * As for dvmdii, R is last on stack and guaranteed to be gen_0 in case the
 * remainder is 0. R = NULL is allowed. */
GEN
sqrtremi(GEN a, GEN *r)
{
  long l, na = NLIMBS(a);
  mp_size_t nr;
  GEN S;
  if (!na) {
    if (r) *r = gen_0;
    return gen_0;
  }
  l = (na + 5) >> 1; /* 2 + ceil(na/2) */
  S = cgetipos(l);
  if (r) {
    GEN R = cgeti(2 + na);
    nr = mpn_sqrtrem(LIMBS(S), LIMBS(R), LIMBS(a), na);
    if (nr) R[1] = evalsigne(1) | evallgefint(nr+2);
    else    { set_avma((pari_sp)S); R = gen_0; }
    *r = R;
  }
  else
    (void)mpn_sqrtrem(LIMBS(S), NULL, LIMBS(a), na);
  return S;
}

/* compute sqrt(|a|), assuming a != 0 */
GEN
sqrtr_abs(GEN a)
{
  GEN res;
  mp_limb_t *b, *c;
  long l = RNLIMBS(a), e = expo(a), er = e>>1;
  long n;
  res = cgetr(2 + l);
  res[1] = evalsigne(1) | evalexpo(er);
  if (e&1)
  {
    b = (mp_limb_t *) new_chunk(l<<1);
    xmpn_zero(b,l);
    xmpn_mirrorcopy(b+l, RLIMBS(a), l);
    c = (mp_limb_t *) new_chunk(l);
    n = mpn_sqrtrem(c,b,b,l<<1); /* c <- sqrt; b <- rem */
    if (n>l || (n==l && mpn_cmp(b,c,l) > 0)) mpn_add_1(c,c,l,1);
  }
  else
  {
    ulong u;
    b = (mp_limb_t *) mantissa2nr(a,-1);
    b[1] = uel(a,l+1)<<(BITS_IN_LONG-1);
    b = (mp_limb_t *) new_chunk(l);
    xmpn_zero(b,l+1); /* overwrites the former b[0] */
    c = (mp_limb_t *) new_chunk(l + 1);
    n = mpn_sqrtrem(c,b,b,(l<<1)+2); /* c <- sqrt; b <- rem */
    u = (ulong)*c++;
    if ( u&HIGHBIT || (u == ~HIGHBIT &&
             (n>l || (n==l && mpn_cmp(b,c,l)>0))))
      mpn_add_1(c,c,l,1);
  }
  xmpn_mirrorcopy(RLIMBS(res),c,l);
  return gc_const((pari_sp)res, res);
}

/* Normalize a nonnegative integer */
GEN
int_normalize(GEN x, long known_zero_words)
{
  long i =  lgefint(x) - 1 - known_zero_words;
  for ( ; i > 1; i--)
    if (x[i]) { setlgefint(x, i+1); return x; }
  x[1] = evalsigne(0) | evallgefint(2); return x;
}

/********************************************************************
 **                                                                **
 **                           Base Conversion                      **
 **                                                                **
 ********************************************************************/

ulong *
convi(GEN x, long *l)
{
  long n = nchar2nlong(2 + (long)(NLIMBS(x) * (BITS_IN_LONG * LOG10_2)));
  GEN str = cgetg(n+1, t_VECSMALL);
  unsigned char *res = (unsigned char*) GSTR(str);
  long llz = mpn_get_str(res, 10, LIMBS(icopy(x)), NLIMBS(x));
  long lz;
  ulong *z;
  long i, j;
  unsigned char *t;
  while (!*res) {res++; llz--;} /*Strip leading zeros*/
  lz  = (8+llz)/9;
  z = (ulong*)new_chunk(1+lz);
  t=res+llz+9;
  for(i=0;i<llz-8;i+=9)
  {
    ulong s;
    t-=18;
    s=*t++;
    for (j=1; j<9;j++)
      s=10*s+*t++;
    *z++=s;
  }
  if (i<llz)
  {
    unsigned char *t = res;
    ulong s=*t++;
    for (j=i+1; j<llz;j++)
      s=10*s+*t++;
    *z++=s;
  }
  *l = lz;
  return z;
}
#line 2 "../src/kernel/none/cmp.c"
/* Copyright (C) 2002-2003  The PARI group.

This file is part of the PARI/GP package.

PARI/GP is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version. It is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY WHATSOEVER.

Check the License for details. You should have received a copy of it, along
with the package; see the file 'COPYING'. If not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */


/********************************************************************/
/**                                                                **/
/**                      Comparison routines                       **/
/**                                                                **/
/********************************************************************/

/*They depend on cmpiispec and equaliispec in mp.c*/

int
equalii(GEN x, GEN y)
{
  if ((x[1] & (LGBITS|SIGNBITS)) != (y[1] & (LGBITS|SIGNBITS))) return 0;
  return equaliispec(x+2, y+2, lgefint(x)-2, lgefint(y)-2);
}

int
cmpii(GEN x, GEN y)
{
  const long sx = signe(x), sy = signe(y);
  if (sx<sy) return -1;
  if (sx>sy) return 1;
  if (!sx) return 0;
  if (sx>0)
    return cmpiispec(x+2, y+2, lgefint(x)-2, lgefint(y)-2);
  else
    return -cmpiispec(x+2, y+2, lgefint(x)-2, lgefint(y)-2);
}

int
equalrr(GEN x, GEN y)
{
  long lx, ly, i;

  if (!signe(x)) {
    if (!signe(y)) return 1; /* all zeroes are equal */
    return expo(x) >= expo(y);
  }
  if (!signe(y))
    return expo(y) >= expo(x);

  if (x[1] != y[1]) return 0;

  lx = lg(x);
  ly = lg(y);
  if (lx < ly)
  {
    i=2; while (i<lx && x[i]==y[i]) i++;
    if (i<lx) return 0;
    for (; i < ly; i++) if (y[i]) return 0;
  }
  else
  {
    i=2; while (i<ly && x[i]==y[i]) i++;
    if (i<ly) return 0;
    for (; i < lx; i++) if (x[i]) return 0;
  }
  return 1;
}

int
cmprr(GEN x, GEN y)
{
  const long sx = signe(x), sy = signe(y);
  long ex,ey,lx,ly,lz,i;

  if (!sx) {
    if (!sy || expo(x) >= expo(y)) return 0;
    return sy > 0? -1: 1;
  }
  if (!sy) {
    if (expo(y) >= expo(x)) return 0;
    return sx > 0? 1: -1;
  }
  if (sx<sy) return -1;
  if (sx>sy) return 1;

  ex=expo(x); ey=expo(y);
  if (ex>ey) return sx;
  if (ex<ey) return -sx;

  lx=lg(x); ly=lg(y); lz = (lx<ly)?lx:ly;
  i=2; while (i<lz && x[i]==y[i]) i++;
  if (i<lz) return ((ulong)x[i] > (ulong)y[i]) ? sx : -sx;
  if (lx>=ly)
  {
    while (i<lx && !x[i]) i++;
    return (i==lx) ? 0 : sx;
  }
  while (i<ly && !y[i]) i++;
  return (i==ly) ? 0 : -sx;
}

/* x and y are integers. Return 1 if |x| == |y|, 0 otherwise */
int
absequalii(GEN x, GEN y)
{
  if (!signe(x)) return !signe(y);
  if (!signe(y)) return 0;
  return equaliispec(x+2, y+2, lgefint(x)-2, lgefint(y)-2);
}

/* x and y are integers. Return sign(|x| - |y|) */
int
abscmpii(GEN x, GEN y)
{
  if (!signe(x)) return signe(y)? -1: 0;
  if (!signe(y)) return 1;
  return cmpiispec(x+2, y+2, lgefint(x)-2, lgefint(y)-2);
}

/* x and y are reals. Return sign(|x| - |y|) */
int
abscmprr(GEN x, GEN y)
{
  long ex,ey,lx,ly,lz,i;

  if (!signe(x)) return signe(y)? -1: 0;
  if (!signe(y)) return 1;

  ex=expo(x); ey=expo(y);
  if (ex>ey) return  1;
  if (ex<ey) return -1;

  lx=lg(x); ly=lg(y); lz = (lx<ly)?lx:ly;
  i=2; while (i<lz && x[i]==y[i]) i++;
  if (i<lz) return ((ulong)x[i] > (ulong)y[i])? 1: -1;
  if (lx>=ly)
  {
    while (i<lx && !x[i]) i++;
    return (i==lx)? 0: 1;
  }
  while (i<ly && !y[i]) i++;
  return (i==ly)? 0: -1;
}

#line 2 "../src/kernel/none/gcdll.c"
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

/***********************************************************************/
/**                                                                   **/
/**                          GCD                                      **/
/**                                                                   **/
/***********************************************************************/
/* Fast ulong gcd.  Called with y odd; x can be arbitrary (but will most of
 * the time be smaller than y). */

/* Gotos are Harmful, and Programming is a Science.  E.W.Dijkstra. */
INLINE ulong
gcduodd(ulong x, ulong y)         /* assume y&1==1, y > 1 */
{
  if (!x) return y;
  /* fix up x */
  while (!(x&1)) x>>=1;
  if (x==1) return 1;
  if (x==y) return y;
  else if (x>y) goto xislarger;/* will be rare, given how we'll use this */
  /* loop invariants: x,y odd and distinct. */
 yislarger:
  if ((x^y)&2)                 /* ...01, ...11 or vice versa */
    y=(x>>2)+(y>>2)+1;         /* ==(x+y)>>2 except it can't overflow */
  else                         /* ...01,...01 or ...11,...11 */
    y=(y-x)>>2;                /* now y!=0 in either case */
  while (!(y&1)) y>>=1;        /* kill any windfall-gained powers of 2 */
  if (y==1) return 1;          /* comparand == return value... */
  if (x==y) return y;          /* this and the next is just one comparison */
  else if (x<y) goto yislarger;/* else fall through to xislarger */

 xislarger:                    /* same as above, seen through a mirror */
  if ((x^y)&2)
    x=(x>>2)+(y>>2)+1;
  else
    x=(x-y)>>2;                /* x!=0 */
  while (!(x&1)) x>>=1;
  if (x==1) return 1;
  if (x==y) return y;
  else if (x>y) goto xislarger;

  goto yislarger;
}
/* Gotos are useful, and Programming is an Art.  D.E.Knuth. */
/* PS: Of course written with Dijkstra's lessons firmly in mind... --GN */

/* at least one of a or b is odd, return gcd(a,b) */
INLINE ulong
mygcduodd(ulong a, ulong b)
{
  ulong c;
  if (b&1)
  {
    if (a==1 || b==1)
      c = 1;
    else
     c = gcduodd(a, b);
  }
  else
  {
    if (a==1)
      c = 1;
    else
      c = gcduodd(b, a);
  }
  return c;
}

/* modified right shift binary algorithm with at most one division */
ulong
ugcd(ulong a,ulong b)
{
  long v;
  if (!b) return a;
  if (!a) return b;
  if (a>b) { a %= b; if (!a) return b; }
  else     { b %= a; if (!b) return a; }
  v = vals(a|b);
  return mygcduodd(a>>v, b>>v) << v;
}
long
cgcd(long a,long b) { return (long)ugcd(labs(a), labs(b)); }

/* For gcdii(): assume a>b>0, return gcd(a,b) as a GEN */
static GEN
igcduu(ulong a, ulong b)
{
  long v;
  a %= b; if (!a) return utoipos(b);
  v = vals(a|b);
  return utoipos( mygcduodd(a>>v, b>>v) << v );
}

/*Warning: overflows silently if lcm does not fit*/
ulong
ulcm(ulong a, ulong b)
{
  ulong d = ugcd(a,b);
  if (!d) return 0;
  return d == 1? a*b: a*(b/d);
}
long
clcm(long a,long b) { return ulcm(labs(a), labs(b)); }

/********************************************************************/
/**                                                                **/
/**               INTEGER EXTENDED GCD  (AND INVMOD)               **/
/**                                                                **/
/********************************************************************/
/* Two basic ideas - (1) avoid many integer divisions, especially when the
 * quotient is 1 which happens ~ 40% of the time.  (2) Use Lehmer's trick as
 * modified by Jebelean of extracting a couple of words' worth of leading bits
 * from both operands, and compute partial quotients from them as long as we
 * can be sure of their values.  Jebelean's modifications consist in
 * inequalities from which we can quickly decide whether to carry on or to
 * return to the outer loop, and in re-shifting after the first word's worth of
 * bits has been used up.  All of this is described in R. Lercier's thesis
 * [pp148-153 & 163f.], except his outer loop isn't quite right: the catch-up
 * divisions needed when one partial quotient is larger than a word are missing.
 *
 * The API consists of invmod() and bezout() below; the single-word routines
 * xgcduu and xxgcduu may be called directly if desired; lgcdii() probably
 * doesn't make much sense out of context.
 *
 * The whole lot is a factor 6 .. 8 faster on word-sized operands, and asym-
 * ptotically about a factor 2.5 .. 3, depending on processor architecture,
 * than the naive continued-division code.  Unfortunately, thanks to the
 * unrolled loops and all, the code is lengthy. */

/*==================================
 * xgcduu(d,d1,f,v,v1,s)
 * xxgcduu(d,d1,f,u,u1,v,v1,s)
 * rgcduu(d,d1,vmax,u,u1,v,v1,s)
 *==================================*/
/*
 * Fast `final' extended gcd algorithm, acting on two ulongs.  Ideally this
 * should be replaced with assembler versions wherever possible.  The present
 * code essentially does `subtract, compare, and possibly divide' at each step,
 * which is reasonable when hardware division (a) exists, (b) is a bit slowish
 * and (c) does not depend a lot on the operand values (as on i486).  When
 * wordsize division is in fact an assembler routine based on subtraction,
 * this strategy may not be the most efficient one.
 *
 * xxgcduu() should be called with  d > d1 > 0, returns gcd(d,d1), and assigns
 * the usual signless cont.frac. recurrence matrix to [u, u1; v, v1]  (i.e.,
 * the product of all the [0, 1; 1 q_j] where the leftmost factor arises from
 * the quotient of the first division step),  and the information about the
 * implied signs to s  (-1 when an odd number of divisions has been done,
 * 1 otherwise).  xgcduu() is exactly the same except that u,u1 are not com-
 * puted (and not returned, of course).
 *
 * The input flag f should be set to 1 if we know in advance that gcd(d,d1)==1
 * (so we can stop the chain division one step early:  as soon as the remainder
 * equals 1).  Use this when you intend to use only what would be v, or only
 * what would be u and v, after that final division step, but not u1 and v1.
 * With the flag in force and thus without that final step, the interesting
 * quantity/ies will still sit in [u1 and] v1, of course.
 *
 * For computing the inverse of a single-word INTMOD known to exist, pass f=1
 * to xgcduu(), and obtain the result from s and v1.  (The routine does the
 * right thing when d1==1 already.)  For finishing a multiword modinv known
 * to exist, pass f=1 to xxgcduu(), and multiply the returned matrix  (with
 * properly adjusted signs)  onto the values v' and v1' previously obtained
 * from the multiword division steps.  Actually, just take the scalar product
 * of [v',v1'] with [u1,-v1], and change the sign if s==-1.  (If the final
 * step had been carried out, it would be [-u,v], and s would also change.)
 * For reducing a rational number to lowest terms, pass f=0 to xgcduu().
 * Finally, f=0 with xxgcduu() is useful for Bezout computations.
 * (It is safe for invmod() to call xgcduu() with f=1, because f&1 doesn't
 * make a difference when gcd(d,d1)>1.  The speedup is negligible.)
 *
 * In principle, when gcd(d,d1) is known to be 1, it is straightforward to
 * recover the final u,u1 given only v,v1 and s.  However, it probably isn't
 * worthwhile, as it trades a few multiplications for a division.
 *
 * rgcduu() is a variant of xxgcduu() which does not have f  (the effect is
 * that of f=0),  but instead has a ulong vmax parameter, for use in rational
 * reconstruction below.  It returns when v1 exceeds vmax;  v will never
 * exceed vmax.  (vmax=0 is taken as a synonym of ULONG_MAX i.e. unlimited,
 * in which case rgcduu behaves exactly like xxgcduu with f=0.)  The return
 * value of rgcduu() is typically meaningless;  the interesting part is the
 * matrix. */

ulong
xgcduu(ulong d, ulong d1, int f, ulong* v, ulong* v1, long *s)
{
  ulong xv,xv1, xs, q,res;
  LOCAL_HIREMAINDER;

  /* The above blurb contained a lie.  The main loop always stops when d1
   * has become equal to 1.  If (d1 == 1 && !(f&1)) after the loop, we do
   * the final `division' of d by 1 `by hand' as it were.
   *
   * The loop has already been unrolled once.  Aggressive optimization could
   * well lead to a totally unrolled assembler version.
   *
   * On modern x86 architectures, this loop is a pig anyway.  The division
   * instruction always puts its result into the same pair of registers, and
   * we always want to use one of them straight away, so pipeline performance
   * will suck big time.  An assembler version should probably do a first loop
   * computing and storing all the quotients -- their number is bounded in
   * advance -- and then assembling the matrix in a second pass.  On other
   * architectures where we can cycle through four or so groups of registers
   * and exploit a fast ALU result-to-operand feedback path, this is much less
   * of an issue. */
  xs = res = 0;
  xv = 0UL; xv1 = 1UL;
  while (d1 > 1UL)
  {
    d -= d1; /* no need to use subll */
    if (d >= d1)
    {
      hiremainder = 0; q = 1 + divll(d,d1); d = hiremainder;
      xv += q * xv1;
    }
    else
      xv += xv1;
    if (d <= 1UL) { xs=1; break; } /* possible loop exit */
    /* repeat with inverted roles */
    d1 -= d;
    if (d1 >= d)
    {
      hiremainder = 0; q = 1 + divll(d1,d); d1 = hiremainder;
      xv1 += q * xv;
    }
    else
      xv1 += xv;
  }

  if (!(f&1))
  { /* division by 1 postprocessing if needed */
    if (xs && d==1)
    { xv1 += d1 * xv; xs = 0; res = 1UL; }
    else if (!xs && d1==1)
    { xv += d * xv1; xs = 1; res = 1UL; }
  }

  if (xs)
  {
    *s = -1; *v = xv1; *v1 = xv;
    return (res ? res : (d==1 ? 1UL : d1));
  }
  else
  {
    *s = 1; *v = xv; *v1 = xv1;
    return (res ? res : (d1==1 ? 1UL : d));
  }
}

ulong
xxgcduu(ulong d, ulong d1, int f,
        ulong* u, ulong* u1, ulong* v, ulong* v1, long *s)
{
  ulong xu,xu1, xv,xv1, xs, q,res;
  LOCAL_HIREMAINDER;

  xs = res = 0;
  xu = xv1 = 1UL;
  xu1 = xv = 0UL;
  while (d1 > 1UL)
  {
    /* no need to use subll */
    d -= d1;
    if (d >= d1)
    {
      hiremainder = 0; q = 1 + divll(d,d1); d = hiremainder;
      xv += q * xv1;
      xu += q * xu1;
    }
    else
    { xv += xv1; xu += xu1; }
    if (d <= 1UL) { xs=1; break; } /* possible loop exit */
    /* repeat with inverted roles */
    d1 -= d;
    if (d1 >= d)
    {
      hiremainder = 0; q = 1 + divll(d1,d); d1 = hiremainder;
      xv1 += q * xv;
      xu1 += q * xu;
    }
    else
    { xv1 += xv; xu1 += xu; }
  }

  if (!(f&1))
  { /* division by 1 postprocessing if needed */
    if (xs && d==1)
    {
      xv1 += d1 * xv;
      xu1 += d1 * xu;
      xs = 0; res = 1UL;
    }
    else if (!xs && d1==1)
    {
      xv += d * xv1;
      xu += d * xu1;
      xs = 1; res = 1UL;
    }
  }

  if (xs)
  {
    *s = -1; *u = xu1; *u1 = xu; *v = xv1; *v1 = xv;
    return (res ? res : (d==1 ? 1UL : d1));
  }
  else
  {
    *s = 1; *u = xu; *u1 = xu1; *v = xv; *v1 = xv1;
    return (res ? res : (d1==1 ? 1UL : d));
  }
}

ulong
rgcduu(ulong d, ulong d1, ulong vmax,
       ulong* u, ulong* u1, ulong* v, ulong* v1, long *s)
{
  ulong xu,xu1, xv,xv1, xs, q, res=0;
  int f = 0;
  LOCAL_HIREMAINDER;

  if (vmax == 0) vmax = ULONG_MAX;
  xs = res = 0;
  xu = xv1 = 1UL;
  xu1 = xv = 0UL;
  while (d1 > 1UL)
  {
    d -= d1; /* no need to use subll */
    if (d >= d1)
    {
      hiremainder = 0; q = 1 + divll(d,d1); d = hiremainder;
      xv += q * xv1;
      xu += q * xu1;
    }
    else
    { xv += xv1; xu += xu1; }
    /* possible loop exit */
    if (xv > vmax) { f=xs=1; break; }
    if (d <= 1UL) { xs=1; break; }
    /* repeat with inverted roles */
    d1 -= d;
    if (d1 >= d)
    {
      hiremainder = 0; q = 1 + divll(d1,d); d1 = hiremainder;
      xv1 += q * xv;
      xu1 += q * xu;
    }
    else
    { xv1 += xv; xu1 += xu; }
    /* possible loop exit */
    if (xv1 > vmax) { f=1; break; }
  }

  if (!(f&1))
  { /* division by 1 postprocessing if needed */
    if (xs && d==1)
    {
      xv1 += d1 * xv;
      xu1 += d1 * xu;
      xs = 0; res = 1UL;
    }
    else if (!xs && d1==1)
    {
      xv += d * xv1;
      xu += d * xu1;
      xs = 1; res = 1UL;
    }
  }

  if (xs)
  {
    *s = -1; *u = xu1; *u1 = xu; *v = xv1; *v1 = xv;
    return (res ? res : (d==1 ? 1UL : d1));
  }
  else
  {
    *s = 1; *u = xu; *u1 = xu1; *v = xv; *v1 = xv1;
    return (res ? res : (d1==1 ? 1UL : d));
  }
}

/*==================================
 * cbezout(a,b,uu,vv)
 *==================================
 * Same as bezout() but for C longs.
 *    Return g = gcd(a,b) >= 0, and assign longs u,v through pointers uu,vv
 *    such that g = u*a + v*b.
 * Special cases:
 *    a = b = 0 ==> pick u=1, v=0 (and return 1, surprisingly)
 *    a != 0 = b ==> keep v=0
 *    a = 0 != b ==> keep u=0
 *    |a| = |b| != 0 ==> keep u=0, set v=+-1
 * Assignments through uu,vv happen unconditionally. */
long
cbezout(long a,long b,long *uu,long *vv)
{
  long s,*t;
  ulong d = labs(a), d1 = labs(b);
  ulong r,u,u1,v,v1;

  if (!b)
  {
    *vv=0L;
    if (!a) { *uu=1L; return 0L; }
    *uu = a < 0 ? -1L : 1L;
    return (long)d;
  }
  else if (!a || (d == d1))
  {
    *uu = 0L; *vv = b < 0 ? -1L : 1L;
    return (long)d1;
  }
  else if (d == 1) /* frequently used by nfinit */
  {
    *uu = a; *vv = 0L;
    return 1L;
  }
  else if (d < d1)
  {
/* bug in gcc-2.95.3:
 * s = a; a = b; b = s; produces wrong result a = b. This is OK:  */
    { long _x = a; a = b; b = _x; } /* in order to keep the right signs */
    r = d; d = d1; d1 = r;
    t = uu; uu = vv; vv = t;
  }
  /* d > d1 > 0 */
  r = xxgcduu(d, d1, 0, &u, &u1, &v, &v1, &s);
  if (s < 0)
  {
    *uu = a < 0 ? (long)u : -(long)u;
    *vv = b < 0 ? -(long)v : (long)v;
  }
  else
  {
    *uu = a < 0 ? -(long)u : (long)u;
    *vv = b < 0 ? (long)v : -(long)v;
  }
  return (long)r;
}

/*==================================
 * lgcdii(d,d1,u,u1,v,v1,vmax)
 *==================================*/
/* Lehmer's partial extended gcd algorithm, acting on two t_INT GENs.
 *
 * Tries to determine, using the leading 2*BITS_IN_LONG significant bits of d
 * and a quantity of bits from d1 obtained by a shift of the same displacement,
 * as many partial quotients of d/d1 as possible, and assigns to [u,u1;v,v1]
 * the product of all the [0,1; 1,qj] thus obtained, where the leftmost
 * factor arises from the quotient of the first division step.
 *
 * For use in rational reconstruction, vmax can be given a nonzero value.
 * In this case, we will return early as soon as v1 > vmax (i.e. v <= vmax)
 *
 * MUST be called with  d > d1 > 0, and with  d occupying more than one
 * significant word.  Returns the number of reduction/swap steps carried out,
 * possibly zero, or under certain conditions minus that number.  When the
 * return value is nonzero, the caller should use the returned recurrence
 * matrix to update its own copies of d,d1.  When the return value is
 * nonpositive, and the latest remainder after updating turns out to be
 * nonzero, the caller should at once attempt a full division, rather than
 * trying lgcdii() again -- this typically happens when we are about to
 * encounter a quotient larger than half a word. (This is not detected
 * infallibly -- after a positive return value, it is possible that the next
 * stage will end up needing a full division.  After a negative return value,
 * however, this is certain, and should be acted upon.)
 *
 * The sign information, for which xgcduu() has its return argument s, is now
 * implicit in the LSB of our return value, and the caller may take advantage
 * of the fact that a return value of +-1 implies u==0,u1==v==1  [only v1 pro-
 * vides interesting information in this case].  One might also use the fact
 * that if the return value is +-2, then u==1, but this is rather marginal.
 *
 * If it was not possible to determine even the first quotient, either because
 * we're too close to an integer quotient or because the quotient would be
 * larger than one word  (if the `leading digit' of d1 after shifting is all
 * zeros), we return 0 and do not assign anything to the last four args.
 *
 * The division chain might even run to completion. It is up to the caller to
 * detect this case. This routine does not change d or d1; this is also up to
 * the caller */
int
lgcdii(ulong* d, ulong* d1, ulong* u, ulong* u1, ulong* v, ulong* v1,
       ulong vmax)
{
  /* Strategy:  (1) Extract/shift most significant bits.  We assume that d
   * has at least two significant words, but we can cope with a one-word d1.
   * Let dd,dd1 be the most significant dividend word and matching part of the
   * divisor.
   * (2) Check for overflow on the first division.  For our purposes, this
   * happens when the upper half of dd1 is zero.  (Actually this is detected
   * during extraction.)
   * (3) Get a fix on the first quotient.  We compute q = floor(dd/dd1), which
   * is an upper bound for floor(d/d1), and which gives the true value of the
   * latter if (and-almost-only-if) the remainder dd' = dd-q*dd1 is >= q.
   * (If it isn't, we give up.  This is annoying because the subsequent full
   * division will repeat some work already done, but it happens infrequently.
   * Doing the extra-bit-fetch in this case would be awkward.)
   * (4) Finish initializations.
   *
   * The remainder of the action is comparatively boring... The main loop has
   * been unrolled once (so we don't swap things and we can apply Jebelean's
   * termination conditions which alternatingly take two different forms during
   * successive iterations).  When we first run out of sufficient bits to form
   * a quotient, and have an extra word of each operand, we pull out two whole
   * word's worth of dividend bits, and divisor bits of matching significance;
   * to these we apply our partial matrix (disregarding overflow because the
   * result mod 2^(2*BITS_IN_LONG) will in fact give the correct values), and
   * re-extract one word's worth of the current dividend and a matching amount
   * of divisor bits.  The affair will normally terminate with matrix entries
   * just short of a whole word.  (We terminate the inner loop before these can
   * possibly overflow.) */
  ulong dd,dd1,ddlo,dd1lo, sh,shc; /* `digits', shift count */
  ulong xu,xu1, xv,xv1, q,res; /* recurrences, partial quotient, count */
  ulong tmp0,tmp1,tmp2,tmpd,tmpu,tmpv; /* temps */
  ulong dm1, d1m1;
  long ld, ld1, lz;
  int skip = 0;
  LOCAL_OVERFLOW;
  LOCAL_HIREMAINDER;

  /* following is just for convenience: vmax==0 means no bound */
  if (vmax == 0) vmax = ULONG_MAX;
  ld = lgefint(d); ld1 = lgefint(d1); lz = ld - ld1; /* >= 0 */
  if (lz > 1) return 0; /* rare */

  d = int_MSW(d);  dm1  = *int_precW(d);
  d1 = int_MSW(d1);d1m1 = *int_precW(d1);
  dd1lo = 0; /* unless we find something better */
  sh = bfffo(*d);

  if (sh)
  { /* do the shifting */
    shc = BITS_IN_LONG - sh;
    if (lz)
    { /* dividend longer than divisor */
      dd1 = (*d1 >> shc);
      if (!(HIGHMASK & dd1)) return 0; /* overflow detected */
      if (ld1 > 3)
        dd1lo = (*d1 << sh) + (d1m1 >> shc);
      else
        dd1lo = (*d1 << sh);
    }
    else
    { /* dividend and divisor have the same length */
      dd1 = (*d1 << sh);
      if (!(HIGHMASK & dd1)) return 0;
      if (ld1 > 3)
      {
        dd1 += (d1m1 >> shc);
        if (ld1 > 4)
          dd1lo = (d1m1 << sh) + (*int_precW(int_precW(d1)) >> shc);
        else
          dd1lo = (d1m1 << sh);
      }
    }
    /* following lines assume d to have 2 or more significant words */
    dd = (*d << sh) + (dm1 >> shc);
    if (ld > 4)
      ddlo = (dm1 << sh) + (*int_precW(int_precW(d)) >> shc);
    else
      ddlo = (dm1 << sh);
  }
  else
  { /* no shift needed */
    if (lz) return 0; /* dividend longer than divisor: overflow */
    dd1 = *d1;
    if (!(HIGHMASK & dd1)) return 0;
    if(ld1 > 3) dd1lo = d1m1;
    /* assume again that d has another significant word */
    dd = *d; ddlo = dm1;
  }
  /* First subtraction/division stage.  (If a subtraction initially suffices,
   * we don't divide at all.)  If a Jebelean condition is violated, and we
   * can't fix it even by looking at the low-order bits in ddlo,dd1lo, we
   * give up and ask for a full division.  Otherwise we commit the result,
   * possibly deciding to re-shift immediately afterwards. */
  dd -= dd1;
  if (dd < dd1)
  { /* first quotient known to be == 1 */
    xv1 = 1UL;
    if (!dd) /* !(Jebelean condition), extraspecial case */
    { /* This actually happens. Now q==1 is known, but we underflow already.
       * OTOH we've just shortened d by a whole word. Thus we are happy and
       * return. */
      *u = 0; *v = *u1 = *v1 = 1UL;
      return -1; /* Next step will be a full division. */
    }
  }
  else
  { /* division indicated */
    hiremainder = 0;
    xv1 = 1 + divll(dd, dd1); /* xv1: alternative spelling of `q', here ;) */
    dd = hiremainder;
    if (dd < xv1) /* !(Jebelean cond'), nonextra special case */
    { /* Attempt to complete the division using the less significant bits,
       * before skipping right past the 1st loop to the reshift stage. */
      ddlo = subll(ddlo, mulll(xv1, dd1lo));
      dd = subllx(dd, hiremainder);

      /* If we now have an overflow, q was too large. Thanks to our decision
       * not to get here unless the original dd1 had bits set in the upper half
       * of the word, we now know that the correct quotient is in fact q-1. */
      if (overflow)
      {
        xv1--;
        ddlo = addll(ddlo,dd1lo);
        dd = addllx(dd,dd1); /* overflows again which cancels the borrow */
        /* ...and fall through to skip=1 below */
      }
      else
      /* Test Jebelean condition anew, at this point using _all_ the extracted
       * bits we have.  This is clutching at straws; we have a more or less
       * even chance of succeeding this time.  Note that if we fail, we really
       * do not know whether the correct quotient would have been q or some
       * smaller value. */
        if (!dd && ddlo < xv1) return 0;

      /* Otherwise, we now know that q is correct, but we cannot go into the
       * 1st loop.  Raise a flag so we'll remember to skip past the loop.
       * Get here also after the q-1 adjustment case. */
      skip = 1;
    } /* if !(Jebelean) then */
  }
  res = 1;
  if (xv1 > vmax)
  { /* gone past the bound already */
    *u = 0UL; *u1 = 1UL; *v = 1UL; *v1 = xv1;
    return res;
  }
  xu = 0UL; xv = xu1 = 1UL;

  /* Some invariants from here across the first loop:
   *
   * At this point, and again after we are finished with the first loop and
   * subsequent conditional, a division and the attached update of the
   * recurrence matrix have just been carried out completely.  The matrix
   * xu,xu1;xv,xv1 has been initialized (or updated, possibly with permuted
   * columns), and the current remainder == next divisor (dd at the moment)
   * is nonzero (it might be zero here, but then skip will have been set).
   *
   * After the first loop, or when skip is set already, it will also be the
   * case that there aren't sufficiently many bits to continue without re-
   * shifting.  If the divisor after reshifting is zero, or indeed if it
   * doesn't have more than half a word's worth of bits, we will have to
   * return at that point.  Otherwise, we proceed into the second loop.
   *
   * Furthermore, when we reach the re-shift stage, dd:ddlo and dd1:dd1lo will
   * already reflect the result of applying the current matrix to the old
   * ddorig:ddlo and dd1orig:dd1lo.  (For the first iteration above, this
   * was easy to achieve, and we didn't even need to peek into the (now
   * no longer existent!) saved words.  After the loop, we'll stop for a
   * moment to merge in the ddlo,dd1lo contributions.)
   *
   * Note that after the 1st division, even an a priori quotient of 1 cannot be
   * trusted until we've checked Jebelean's condition: it might be too small */
  if (!skip)
  {
    for(;;)
    { /* First half of loop divides dd into dd1, and leaves the recurrence
       * matrix xu,...,xv1 groomed the wrong way round (xu,xv will be the newer
       * entries) when successful. */
      tmpd = dd1 - dd;
      if (tmpd < dd)
      { /* quotient suspected to be 1 */
        tmpu = xu + xu1; /* cannot overflow -- everything bounded by
                          * the original dd during first loop */
        tmpv = xv + xv1;
      }
      else
      { /* division indicated */
        hiremainder = 0;
        q = 1 + divll(tmpd, dd);
        tmpd = hiremainder;
        tmpu = xu + q*xu1; /* can't overflow, but may need to be undone */
        tmpv = xv + q*xv1;
      }

      tmp0 = addll(tmpv, xv1);
      if ((tmpd < tmpu) || overflow ||
          (dd - tmpd < tmp0)) /* !(Jebelean cond.) */
        break; /* skip ahead to reshift stage */
      else
      { /* commit dd1, xu, xv */
        res++;
        dd1 = tmpd; xu = tmpu; xv = tmpv;
        if (xv > vmax) { *u = xu1; *u1 = xu; *v = xv1; *v1 = xv; return res; }
      }

      /* Second half of loop divides dd1 into dd, and the matrix returns to its
       * normal arrangement. */
      tmpd = dd - dd1;
      if (tmpd < dd1)
      { /* quotient suspected to be 1 */
        tmpu = xu1 + xu; /* cannot overflow */
        tmpv = xv1 + xv;
      }
      else
      { /* division indicated */
        hiremainder = 0;
        q = 1 + divll(tmpd, dd1);
        tmpd = hiremainder;
        tmpu = xu1 + q*xu;
        tmpv = xv1 + q*xv;
      }

      tmp0 = addll(tmpu, xu);
      if ((tmpd < tmpv) || overflow ||
          (dd1 - tmpd < tmp0)) /* !(Jebelean cond.) */
        break; /* skip ahead to reshift stage */
      else
      { /* commit dd, xu1, xv1 */
        res++;
        dd = tmpd; xu1 = tmpu; xv1 = tmpv;
        if (xv1 > vmax) { *u = xu; *u1 = xu1; *v = xv; *v1 = xv1; return res; }
      }
    } /* end of first loop */

    /* Intermezzo: update dd:ddlo, dd1:dd1lo.  (But not if skip is set.) */
    if (res&1)
    { /* after failed division in 1st half of loop:
       * [dd1:dd1lo,dd:ddlo] = [ddorig:ddlo,dd1orig:dd1lo]
       *                       * [ -xu, xu1 ; xv, -xv1 ]
       * Actually, we only multiply [ddlo,dd1lo] onto the matrix and add the
       * high-order remainders + overflows onto [dd1,dd] */
      tmp1 = mulll(ddlo, xu); tmp0 = hiremainder;
      tmp1 = subll(mulll(dd1lo,xv), tmp1);
      dd1 += subllx(hiremainder, tmp0);
      tmp2 = mulll(ddlo, xu1); tmp0 = hiremainder;
      ddlo = subll(tmp2, mulll(dd1lo,xv1));
      dd += subllx(tmp0, hiremainder);
      dd1lo = tmp1;
    }
    else
    { /* after failed division in 2nd half of loop:
       * [dd:ddlo,dd1:dd1lo] = [ddorig:ddlo,dd1orig:dd1lo]
       *                       * [ xu1, -xu ; -xv1, xv ]
       * Actually, we only multiply [ddlo,dd1lo] onto the matrix and add the
       * high-order remainders + overflows onto [dd,dd1] */
      tmp1 = mulll(ddlo, xu1); tmp0 = hiremainder;
      tmp1 = subll(tmp1, mulll(dd1lo,xv1));
      dd += subllx(tmp0, hiremainder);
      tmp2 = mulll(ddlo, xu); tmp0 = hiremainder;
      dd1lo = subll(mulll(dd1lo,xv), tmp2);
      dd1 += subllx(hiremainder, tmp0);
      ddlo = tmp1;
    }
  } /* end of skip-pable section:  get here also, with res==1, when there
     * was a problem immediately after the very first division. */

  /* Re-shift.  Note: the shift count _can_ be zero, viz. under the following
   * precise conditions:  The original dd1 had its topmost bit set, so the 1st
   * q was 1, and after subtraction, dd had its topmost bit unset.  If now dd=0,
   * we'd have taken the return exit already, so we couldn't have got here.
   * If not, then it must have been the second division which has gone amiss
   * (because dd1 was very close to an exact multiple of the remainder dd value,
   * so this will be very rare).  At this point, we'd have a fairly slim chance
   * of fixing things by re-examining dd1:dd1lo vs. dd:ddlo, but this is not
   * guaranteed to work. Instead of trying, we return at once and let caller
   * see to it that the initial subtraction is re-done usingall the bits of
   * both operands, which already helps, and the next round will either be a
   * full division  (if dd occupied a halfword or less), or another llgcdii()
   * first step.  In the latter case, since we try a little harder during our
   * first step, we may actually be able to fix the problem, and get here again
   * with improved low-order bits and with another step under our belt.
   * Otherwise we'll have given up above and forced a full division.
   *
   * If res is even, the shift count _cannot_ be zero.  (The first step forces
   * a zero into the remainder's MSB, and all subsequent remainders will have
   * inherited it.)
   *
   * The re-shift stage exits if the next divisor has at most half a word's
   * worth of bits.
   *
   * For didactic reasons, the second loop will be arranged in the same way
   * as the first -- beginning with the division of dd into dd1, as if res
   * was odd.  To cater for this, if res is actually even, we swap things
   * around during reshifting.  (During the second loop, the parity of res
   * does not matter; we know in which half of the loop we are when we decide
   * to return.) */
  if (res&1)
  { /* after odd number of division(s) */
    if (dd1 && (sh = bfffo(dd1)))
    {
      shc = BITS_IN_LONG - sh;
      dd = (ddlo >> shc) + (dd << sh);
      if (!(HIGHMASK & dd))
      {
        *u = xu; *u1 = xu1; *v = xv; *v1 = xv1;
        return -res; /* full division asked for */
      }
      dd1 = (dd1lo >> shc) + (dd1 << sh);
    }
    else
    { /* time to return: <= 1 word left, or sh==0 */
      *u = xu; *u1 = xu1; *v = xv; *v1 = xv1;
      return res;
    }
  }
  else
  { /* after even number of divisions */
    if (dd)
    {
      sh = bfffo(dd); /* > 0 */
      shc = BITS_IN_LONG - sh;
      /* dd:ddlo will become the new dd1, and v.v. */
      tmpd = (ddlo >> shc) + (dd << sh);
      dd = (dd1lo >> shc) + (dd1 << sh);
      dd1 = tmpd;
      /* This completes the swap; now dd is again the current divisor */
      if (HIGHMASK & dd)
      { /* recurrence matrix is the wrong way round; swap it */
        tmp0 = xu; xu = xu1; xu1 = tmp0;
        tmp0 = xv; xv = xv1; xv1 = tmp0;
      }
      else
      { /* recurrence matrix is the wrong way round; fix this */
        *u = xu1; *u1 = xu; *v = xv1; *v1 = xv;
        return -res;                /* full division asked for */
      }
    }
    else
    { /* time to return: <= 1 word left */
      *u = xu1; *u1 = xu; *v = xv1; *v1 = xv;
      return res;
    }
  } /* end reshift */

  /* The Second Loop.  Rip-off of the first, but we now check for overflow
   * in the recurrences.  Returns instead of breaking when we cannot fix the
   * quotient any longer. */
  for(;;)
  { /* First half of loop divides dd into dd1, and leaves the recurrence
     * matrix xu,...,xv1 groomed the wrong way round (xu,xv will be the newer
     * entries) when successful */
    tmpd = dd1 - dd;
    if (tmpd < dd)
    { /* quotient suspected to be 1 */
      tmpu = xu + xu1;
      tmpv = addll(xv, xv1); /* xv,xv1 will overflow first */
      tmp1 = overflow;
    }
    else
    { /* division indicated */
      hiremainder = 0;
      q = 1 + divll(tmpd, dd);
      tmpd = hiremainder;
      tmpu = xu + q*xu1;
      tmpv = addll(xv, mulll(q,xv1));
      tmp1 = overflow | hiremainder;
    }

    tmp0 = addll(tmpv, xv1);
    if ((tmpd < tmpu) || overflow || tmp1 ||
        (dd - tmpd < tmp0)) /* !(Jebelean cond.) */
    { /* The recurrence matrix has not yet been warped... */
      *u = xu; *u1 = xu1; *v = xv; *v1 = xv1;
      break;
    }
    /* commit dd1, xu, xv */
    res++;
    dd1 = tmpd; xu = tmpu; xv = tmpv;
    if (xv > vmax) { *u = xu1; *u1 = xu; *v = xv1; *v1 = xv; return res; }

    /* Second half of loop divides dd1 into dd, and the matrix returns to its
     * normal arrangement */
    tmpd = dd - dd1;
    if (tmpd < dd1)
    { /* quotient suspected to be 1 */
      tmpu = xu1 + xu;
      tmpv = addll(xv1, xv);
      tmp1 = overflow;
    }
    else
    { /* division indicated */
      hiremainder = 0;
      q = 1 + divll(tmpd, dd1);
      tmpd = hiremainder;
      tmpu = xu1 + q*xu;
      tmpv = addll(xv1, mulll(q, xv));
      tmp1 = overflow | hiremainder;
    }

    tmp0 = addll(tmpu, xu);
    if ((tmpd < tmpv) || overflow || tmp1 ||
        (dd1 - tmpd < tmp0)) /* !(Jebelean cond.) */
    { /* The recurrence matrix has not yet been unwarped, so it is
       * the wrong way round;  fix this. */
      *u = xu1; *u1 = xu; *v = xv1; *v1 = xv;
      break;
    }

    res++; /* commit dd, xu1, xv1 */
    dd = tmpd; xu1 = tmpu; xv1 = tmpv;
    if (xv1 > vmax) { *u = xu; *u1 = xu1; *v = xv; *v1 = xv1; return res; }
  } /* end of second loop */

  return res;
}

/* 1 / Mod(x,p). Assume x < p */
ulong
Fl_invsafe(ulong x, ulong p)
{
  long s;
  ulong xv, xv1, g = xgcduu(p, x, 1, &xv, &xv1, &s);
  if (g != 1UL) return 0UL;
  xv = xv1 % p; if (s < 0) xv = p - xv;
  return xv;
}

/* 1 / Mod(x,p). Assume x < p */
ulong
Fl_inv(ulong x, ulong p)
{
  ulong xv  = Fl_invsafe(x, p);
  if (!xv && p!=1UL) pari_err_INV("Fl_inv", mkintmod(utoi(x), utoi(p)));
  return xv;
}
#line 2 "../src/kernel/none/halfgcd.c"
/* Copyright (C) 2019  The PARI group.

This file is part of the PARI/GP package.

PARI/GP is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version. It is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY WHATSOEVER.

Check the License for details. You should have received a copy of it, along
with the package; see the file 'COPYING'. If not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

static GEN
ZM_mul2(GEN A, GEN B)
{
  GEN A11=gcoeff(A,1,1),A12=gcoeff(A,1,2), B11=gcoeff(B,1,1),B12=gcoeff(B,1,2);
  GEN A21=gcoeff(A,2,1),A22=gcoeff(A,2,2), B21=gcoeff(B,2,1),B22=gcoeff(B,2,2);
  GEN M1 = mulii(addii(A11,A22), addii(B11,B22));
  GEN M2 = mulii(addii(A21,A22), B11);
  GEN M3 = mulii(A11, subii(B12,B22));
  GEN M4 = mulii(A22, subii(B21,B11));
  GEN M5 = mulii(addii(A11,A12), B22);
  GEN M6 = mulii(subii(A21,A11), addii(B11,B12));
  GEN M7 = mulii(subii(A12,A22), addii(B21,B22));
  GEN T1 = addii(M1,M4), T2 = subii(M7,M5);
  GEN T3 = subii(M1,M2), T4 = addii(M3,M6);
  retmkmat2(mkcol2(addii(T1,T2), addii(M2,M4)),
            mkcol2(addii(M3,M5), addii(T3,T4)));
}

static GEN
matid2(void)
{
    retmkmat2(mkcol2(gen_1,gen_0),
              mkcol2(gen_0,gen_1));
}

/* Return M*[q,1;1,0] */
static GEN
mulq(GEN M, GEN q)
{
  GEN u, v, res = cgetg(3, t_MAT);
  u = addii(mulii(gcoeff(M,1,1), q), gcoeff(M,1,2));
  v = addii(mulii(gcoeff(M,2,1), q), gcoeff(M,2,2));
  gel(res,1) = mkcol2(u, v);
  gel(res,2) = gel(M,1);
  return res;
}
static GEN
mulqab(GEN M, GEN q, GEN *ap, GEN *bp)
{
  GEN b = subii(*ap, mulii(*bp, q));
  *ap = *bp; *bp = b;
  return mulq(M,q);
}

/* Return M*[q,1;1,0]^-1 */

static GEN
mulqi(GEN M, GEN q, GEN *ap, GEN *bp)
{
  GEN u, v, res, a;
  a = addii(mulii(*ap, q), *bp);
  *bp = *ap; *ap = a;
  res = cgetg(3, t_MAT);
  u = subii(gcoeff(M,1,1),mulii(gcoeff(M,1,2), q));
  v = subii(gcoeff(M,2,1),mulii(gcoeff(M,2,2), q));
  gel(res,1) = gel(M,2);
  gel(res,2) = mkcol2(u,v);
  return res;
}

static long
uexpi(GEN a)
{ return expi(subiu(shifti(a,1),1)); }

static GEN
FIXUP0(GEN M, GEN *a, GEN *b, long m)
{
  long cnt=0;
  while (expi(*b) >= m)
  {
    GEN r, q = dvmdii(*a, *b, &r);
    *a = *b; *b = r;
    M = mulq(M, q);
    cnt++;
  };
  if (cnt>6) pari_err_BUG("FIXUP0");
  return M;
}

static long
signdet(GEN Q)
{
  long a = Mod4(gcoeff(Q,1,1)), b = Mod4(gcoeff(Q,1,2));
  long c = Mod4(gcoeff(Q,2,1)), d = Mod4(gcoeff(Q,2,2));
  return ((a*d-b*c)&3)==1 ? 1 : -1;
}

static GEN
ZM_inv2(GEN M)
{
  long e = signdet(M);
  if (e==1) return mkmat22(gcoeff(M,2,2),negi(gcoeff(M,1,2)),
                          negi(gcoeff(M,2,1)),gcoeff(M,1,1));
  else      return mkmat22(negi(gcoeff(M,2,2)),gcoeff(M,1,2),
                           gcoeff(M,2,1),negi(gcoeff(M,1,1)));
}

static GEN
lastq(GEN Q)
{
  GEN p = gcoeff(Q,1,1), q = gcoeff(Q,1,2), s = gcoeff(Q,2,2);
  if (signe(q)==0) pari_err_BUG("halfgcd");
  if (signe(s)==0) return p;
  if (equali1(q))  return subiu(p,1);
  return divii(p, q);
}

static GEN
mulT(GEN Q, GEN *ap, GEN *bp)
{
  *ap = addii(*ap, *bp);
  *bp = negi(*bp);
  return mkmat2(gel(Q,1),
           mkcol2(subii(gcoeff(Q,1,1), gcoeff(Q,1,2))
                , subii(gcoeff(Q,2,1), gcoeff(Q,2,2))));
}

static GEN
FIXUP1(GEN M, GEN a, GEN b, long m, long t, GEN *ap, GEN *bp)
{
  GEN Q = gel(M,1), a0 = gel(M,2), b0 = gel(M,3);
  GEN q, am = remi2n(a, m), bm = remi2n(b, m);
  if (signdet(Q)==-1)
  {
    *ap = subii(mulii(bm, gcoeff(Q,1,2)),mulii(am, gcoeff(Q,2,2)));
    *bp = subii(mulii(am, gcoeff(Q,2,1)),mulii(bm, gcoeff(Q,1,1)));
    *ap = addii(*ap, shifti(addii(a0, gcoeff(Q,2,2)), m));
    *bp = addii(*bp, shifti(subii(b0, gcoeff(Q,2,1)), m));
    if (signe(*bp) >= 0)
      return Q;
    if (expi(addii(*ap,*bp)) >= m+t)
      return mulT(Q, ap ,bp);
    q = lastq(Q);
    Q = mulqi(Q, q, ap, bp);
    if (cmpiu(q, 2)>=0)
      return mulqab(Q, subiu(q,1), ap, bp);
    else
      return mulqi(Q, lastq(Q), ap, bp);
  }
  else
  {
    *ap = subii(mulii(am, gcoeff(Q,2,2)),mulii(bm, gcoeff(Q,1,2)));
    *bp = subii(mulii(bm, gcoeff(Q,1,1)),mulii(am, gcoeff(Q,2,1)));
    *ap = addii(*ap, shifti(subii(a0, gcoeff(Q,2,2)), m));
    *bp = addii(*bp, shifti(addii(b0, gcoeff(Q,2,1)), m));
    if (expi(*ap) >= m+t)
      return FIXUP0(Q, ap, bp, m+t);
    else
      return signe(gcoeff(Q,1,2))==0? Q: mulqi(Q, lastq(Q), ap, bp);
  }
}

static long
magic_threshold(GEN a)
{ return (3+uexpi(a))>>1; }

static GEN
HGCD_basecase(GEN a, GEN b)
{
  pari_sp av=avma;
  long m = magic_threshold(a);
  GEN u,u1,v,v1;
  u1 = v = gen_0;
  u = v1 = gen_1;
  while (expi(b) >= m)
  {
    GEN r, q = dvmdii(a,b, &r);
    a = b; b = r; swap(u,u1); swap(v,v1);
    u = addii(mulii(u1, q), u);
    v = addii(mulii(v1, q), v);
   if (gc_needed(av,2))
    {
      if (DEBUGMEM>1) pari_warn(warnmem,"halfgcd (d = %ld)",lgefint(b));
      gerepileall(av,6, &a,&b,&u1,&v1,&u,&v);
    }
  }
  return gerepilecopy(av, mkvec3(mkmat22(u,u1,v,v1), a, b));
}

static GEN HGCD(GEN x, GEN y);

/*
Based on
Klaus Thull and Chee K. Yap,
A unified approach to HGCD algorithms for polynomials andintegers,
1990, Manuscript.
URL: http://cs.nyu.edu/cs/faculty/yap/papers.
*/

static GEN
HGCD_split(GEN a, GEN b)
{
  pari_sp av = avma;
  long m = magic_threshold(a), t, l, k, tp;
  GEN a0, b0, ap, bp, c, d, c0, d0, cp, dp, R, S, T, q, r;
  if (signe(b) < 0  || cmpii(a,b)<0) pari_err_BUG("HGCD_split");
  if (expi(b) < m)
    return gerepilecopy(av, mkvec3(matid2(), a, b));
  a0 = addiu(shifti(a, -m), 1);
  if (cmpiu(a0,7) <= 0)
  {
    R = FIXUP0(matid2(), &a, &b, m);
    return gerepilecopy(av, mkvec3(R, a, b));
  }
  b0 = shifti(b,-m);
  t = magic_threshold(a0);
  R = FIXUP1(HGCD(a0,b0),a, b, m, t, &ap, &bp);
  if (expi(bp) < m)
    return gerepilecopy(av, mkvec3(R, ap, bp));
  q = dvmdii(ap, bp, &r);
  c = bp; d = r;
  if (cmpiu(shifti(c,-m),6) <= 0)
  {
    R = FIXUP0(mulq(R, q), &c, &d, m);
    return gerepilecopy(av, mkvec3(R, c, d));
  }
  l = uexpi(c);
  k = 2*m-l-1; if (k<0) pari_err_BUG("halfgcd");
  c0 = addiu(shifti(c, -k), 1); if (cmpiu(c0,8)<0) pari_err_BUG("halfgcd");
  d0 = shifti(d, -k);
  tp = magic_threshold(c0);
  S = FIXUP1(HGCD(c0,d0), c, d, k, tp, &cp, &dp);
  if (!(expi(cp)>=m+1 && m+1 > expi(dp))) pari_err_BUG("halfgcd");
  T = FIXUP0(ZM_mul2(mulq(R, q), S), &cp, &dp, m);
  return gerepilecopy(av, mkvec3(T, cp, dp));
}

static GEN
HGCD(GEN x, GEN y)
{
  if (lgefint(y)-2 < HALFGCD_LIMIT)
    return HGCD_basecase(x, y);
  else
    return HGCD_split(x, y);
}

static GEN
HGCD0(GEN x, GEN y)
{
  if (signe(y) >= 0 && cmpii(x, y) >= 0)
    return HGCD(x, y);
  if (cmpii(x, y) < 0)
  {
    GEN M = HGCD0(y, x), Q = gel(M,1);
    return mkvec3(mkmat22(gcoeff(Q,2,1),gcoeff(Q,2,2),gcoeff(Q,1,1),gcoeff(Q,1,2)),
        gel(M,2),gel(M,3));
  } /* Now y <= x*/
  if (signe(x) <= 0)
  { /* y <= x <=0 */
    GEN M = HGCD(negi(y), negi(x)), Q = gel(M,1);
    return mkvec3(mkmat22(negi(gcoeff(Q,2,1)),negi(gcoeff(Q,2,2)),
                          negi(gcoeff(Q,1,1)),negi(gcoeff(Q,1,2))),
        gel(M,2),gel(M,3));
  }
  else /* y <= 0 <=x */
  {
    GEN M = HGCD0(x, negi(y)), Q = gel(M,1);
    return mkvec3(mkmat22(gcoeff(Q,1,1),gcoeff(Q,1,2),negi(gcoeff(Q,2,1)),negi(gcoeff(Q,2,2))),
        gel(M,2),gel(M,3));
  }
}

GEN
halfgcdii(GEN A, GEN B)
{
  pari_sp av = avma;
  GEN M, Q, a, b, m = absi_shallow(abscmpii(A, B)>0 ? A: B);
  M = HGCD0(A,B); Q = gel(M,1); a = gel(M,2); b = gel(M,3);
  while (signe(b) && cmpii(sqri(b), m) >= 0)
  {
    GEN r, q = dvmdii(a, b, &r);
    a = b; b = r;
    Q = mulq(Q, q);
  }
  return gerepilecopy(av, mkvec2(ZM_inv2(Q),mkcol2(a,b)));
}
#line 2 "../src/kernel/none/ratlift.c"
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

/*==========================================================
 * Fp_ratlift(GEN x, GEN m, GEN *a, GEN *b, GEN amax, GEN bmax)
 *==========================================================
 * Reconstruct rational number from its residue x mod m
 *    Given t_INT x, m, amax>=0, bmax>0 such that
 *         0 <= x < m;  2*amax*bmax < m
 *    attempts to find t_INT a, b such that
 *         (1) a = b*x (mod m)
 *         (2) |a| <= amax, 0 < b <= bmax
 *         (3) gcd(m, b) = gcd(a, b)
 *    If unsuccessful, it will return 0 and leave a,b unchanged  (and
 *    caller may deduce no such a,b exist).  If successful, sets a,b
 *    and returns 1.  If there exist a,b satisfying (1), (2), and
 *         (3') gcd(m, b) = 1
 *    then they are uniquely determined subject to (1),(2) and
 *         (3'') gcd(a, b) = 1,
 *    and will be returned by the routine.  (The caller may wish to
 *    check gcd(a,b)==1, either directly or based on known prime
 *    divisors of m, depending on the application.)
 * Reference:
 @article {MR97c:11116,
     AUTHOR = {Collins, George E. and Encarnaci{\'o}n, Mark J.},
      TITLE = {Efficient rational number reconstruction},
    JOURNAL = {J. Symbolic Comput.},
     VOLUME = {20},
       YEAR = {1995},
     NUMBER = {3},
      PAGES = {287--297},
 }
 * Preprint available from:
 * ftp://ftp.risc.uni-linz.ac.at/pub/techreports/1994/94-64.ps.gz */
static ulong
get_vmax(GEN r, long lb, long lbb)
{
  long lr = lb - lgefint(r);
  ulong vmax;
  if (lr > 1)        /* still more than a word's worth to go */
    vmax = ULONG_MAX;        /* (cannot in fact happen) */
  else
  { /* take difference of bit lengths */
    long lbr = bfffo(*int_MSW(r));
    lr = lr*BITS_IN_LONG - lbb + lbr;
    if ((ulong)lr > BITS_IN_LONG)
      vmax = ULONG_MAX;
    else if (lr == 0)
      vmax = 1UL;
    else
      vmax = 1UL << (lr-1); /* pessimistic but faster than a division */
  }
  return vmax;
}

/* Assume x,m,amax >= 0,bmax > 0 are t_INTs, 0 <= x < m, 2 amax * bmax < m */
int
Fp_ratlift(GEN x, GEN m, GEN amax, GEN bmax, GEN *a, GEN *b)
{
  GEN d, d1, v, v1, q, r;
  pari_sp av = avma, av1;
  long lb, lbb, s, s0;
  ulong vmax;
  ulong xu, xu1, xv, xv1; /* Lehmer stage recurrence matrix */
  int lhmres;             /* Lehmer stage return value */

  /* special cases x=0 and/or amax=0 */
  if (!signe(x)) { *a = gen_0; *b = gen_1; return 1; }
  if (!signe(amax)) return 0;
  /* assert: m > x > 0, amax > 0 */

  /* check whether a=x, b=1 is a solution */
  if (cmpii(x,amax) <= 0) { *a = icopy(x); *b = gen_1; return 1; }

  /* There is no special case for single-word numbers since this is
   * mainly meant to be used with large moduli. */
  (void)new_chunk(lgefint(bmax) + lgefint(amax)); /* room for a,b */
  d = m; d1 = x;
  v = gen_0; v1 = gen_1;
  /* assert d1 > amax, v1 <= bmax here */
  lb = lgefint(bmax);
  lbb = bfffo(*int_MSW(bmax));
  s = 1;
  av1 = avma;

  /* General case: Euclidean division chain starting with m div x, and
   * with bounds on the sequence of convergents' denoms v_j.
   * Just to be different from what invmod and bezout are doing, we work
   * here with the all-nonnegative matrices [u,u1;v,v1]=prod_j([0,1;1,q_j]).
   * Loop invariants:
   * (a) (sign)*[-v,v1]*x = [d,d1] (mod m)  (componentwise)
   * (sign initially +1, changes with each Euclidean step)
   * so [a,b] will be obtained in the form [-+d,v] or [+-d1,v1];
   * this congruence is a consequence of
   *
   * (b) [x,m]~ = [u,u1;v,v1]*[d1,d]~,
   * where u,u1 is the usual numerator sequence starting with 1,0
   * instead of 0,1  (just multiply the eqn on the left by the inverse
   * matrix, which is det*[v1,-u1;-v,u], where "det" is the same as the
   * "(sign)" in (a)).  From m = v*d1 + v1*d and
   *
   * (c) d > d1 >= 0, 0 <= v < v1,
   * we have d >= m/(2*v1), so while v1 remains smaller than m/(2*amax),
   * the pair [-(sign)*d,v] satisfies (1) but violates (2) (d > amax).
   * Conversely, v1 > bmax indicates that no further solutions will be
   * forthcoming;  [-(sign)*d,v] will be the last, and first, candidate.
   * Thus there's at most one point in the chain division where a solution
   * can live:  v < bmax, v1 >= m/(2*amax) > bmax,  and this is acceptable
   * iff in fact d <= amax  (e.g. m=221, x=34 or 35, amax=bmax=10 fail on
   * this count while x=32,33,36,37 succeed).  However, a division may leave
   * a zero residue before we ever reach this point  (consider m=210, x=35,
   * amax=bmax=10),  and our caller may find that gcd(d,v) > 1  (Examples:
   * keep m=210 and consider any of x=29,31,32,33,34,36,37,38,39,40,41).
   * Furthermore, at the start of the loop body we have in fact
   *
   * (c') 0 <= v < v1 <= bmax, d > d1 > amax >= 0,
   * (and are never done already).
   *
   * Main loop is similar to those of invmod() and bezout(), except for
   * having to determine appropriate vmax bounds, and checking termination
   * conditions.  The signe(d1) condition is only for paranoia */
  while (lgefint(d) > 3 && signe(d1))
  {
    /* determine vmax for lgcdii so as to ensure v won't overshoot.
     * If v+v1 > bmax, the next step would take v1 beyond the limit, so
     * since [+-d1,v1] is not a solution, we give up.  Otherwise if v+v1
     * is way shorter than bmax, use vmax=MAXULUNG.  Otherwise, set vmax
     * to a crude lower approximation of bmax/(v+v1), or to 1, which will
     * allow the inner loop to do one step */
    r = addii(v,v1);
    if (cmpii(r,bmax) > 0) return gc_long(av, 0); /* done, not found */
    vmax = get_vmax(r, lb, lbb);
    /* do a Lehmer-Jebelean round */
    lhmres = lgcdii((ulong *)d, (ulong *)d1, &xu, &xu1, &xv, &xv1, vmax);
    if (lhmres) /* check progress */
    { /* apply matrix */
      if (lhmres == 1 || lhmres == -1)
      {
        s = -s;
        if (xv1 == 1)
        { /* re-use v+v1 computed above */
          v = v1; v1 = r;
          r = subii(d,d1); d = d1; d1 = r;
        }
        else
        {
          r = subii(d, mului(xv1,d1)); d = d1; d1 = r;
          r = addii(v, mului(xv1,v1)); v = v1; v1 = r;
        }
      }
      else
      {
        r  = subii(muliu(d,xu),  muliu(d1,xv));
        d1 = subii(muliu(d,xu1), muliu(d1,xv1)); d = r;
        r  = addii(muliu(v,xu),  muliu(v1,xv));
        v1 = addii(muliu(v,xu1), muliu(v1,xv1)); v = r;
        if (lhmres&1) { togglesign(d); s = -s; } else togglesign(d1);
      }
      /* check whether we're done.  Assert v <= bmax here.  Examine v1:
       * if v1 > bmax, check d and return 0 or 1 depending on the outcome;
       * if v1 <= bmax, check d1 and return 1 if d1 <= amax, otherwise proceed*/
      if (cmpii(v1,bmax) > 0)
      {
        set_avma(av);
        if (cmpii(d,amax) > 0) return 0; /* done, not found */
        /* done, found */
        *a = icopy(d); setsigne(*a,-s);
        *b = icopy(v); return 1;
      }
      if (cmpii(d1,amax) <= 0)
      { /* done, found */
        set_avma(av);
        if (signe(d1)) { *a = icopy(d1); setsigne(*a,s); } else *a = gen_0;
        *b = icopy(v1); return 1;
      }
    } /* lhmres != 0 */

    if (lhmres <= 0 && signe(d1))
    {
      q = dvmdii(d,d1,&r);
      d = d1; d1 = r;
      r = addii(v, mulii(q,v1));
      v = v1; v1 = r;
      s = -s;
      /* check whether we are done now.  Since we weren't before the div, it
       * suffices to examine v1 and d1 -- the new d (former d1) cannot cut it */
      if (cmpii(v1,bmax) > 0) return gc_long(av, 0); /* done, not found */
      if (cmpii(d1,amax) <= 0) /* done, found */
      {
        set_avma(av);
        if (signe(d1)) { *a = icopy(d1); setsigne(*a,s); } else *a = gen_0;
        *b = icopy(v1); return 1;
      }
    }

    if (gc_needed(av,1))
    {
      if(DEBUGMEM>1) pari_warn(warnmem,"ratlift");
      gerepileall(av1, 4, &d, &d1, &v, &v1);
    }
  } /* end while */

  /* Postprocessing - final sprint.  Since we usually underestimate vmax,
   * this function needs a loop here instead of a simple conditional.
   * Note we can only get here when amax fits into one word  (which will
   * typically not be the case!).  The condition is bogus -- d1 is never
   * zero at the start of the loop.  There will be at most a few iterations,
   * so we don't bother collecting garbage */
  while (signe(d1))
  {
    /* Assertions: lgefint(d)==lgefint(d1)==3.
     * Moreover, we aren't done already, or we would have returned by now.
     * Recompute vmax */
    r = addii(v,v1);
    if (cmpii(r,bmax) > 0) return gc_long(av, 0); /* done, not found */
    vmax = get_vmax(r, lb, lbb);
    /* single-word "Lehmer", discarding the gcd or whatever it returns */
    (void)rgcduu((ulong)*int_MSW(d), (ulong)*int_MSW(d1), vmax, &xu, &xu1, &xv, &xv1, &s0);
    if (xv1 == 1) /* avoid multiplications */
    { /* re-use r = v+v1 computed above */
      v = v1; v1 = r;
      r = subii(d,d1); d = d1; d1 = r;
      s = -s;
    }
    else if (xu == 0) /* and xv==1, xu1==1, xv1 > 1 */
    {
      r = subii(d, mului(xv1,d1)); d = d1; d1 = r;
      r = addii(v, mului(xv1,v1)); v = v1; v1 = r;
      s = -s;
    }
    else
    {
      r  = subii(muliu(d,xu),  muliu(d1,xv));
      d1 = subii(muliu(d,xu1), muliu(d1,xv1)); d = r;
      r  = addii(muliu(v,xu),  muliu(v1,xv));
      v1 = addii(muliu(v,xu1), muliu(v1,xv1)); v = r;
      if (s0 < 0) { togglesign(d); s = -s; } else togglesign(d1);
    }
    /* check whether we're done, as above.  Assert v <= bmax.
     * if v1 > bmax, check d and return 0 or 1 depending on the outcome;
     * if v1 <= bmax, check d1 and return 1 if d1 <= amax, otherwise proceed.
     */
    if (cmpii(v1,bmax) > 0)
    {
      set_avma(av);
      if (cmpii(d,amax) > 0) return 0; /* done, not found */
      /* done, found */
      *a = icopy(d); setsigne(*a,-s);
      *b = icopy(v); return 1;
    }
    if (cmpii(d1,amax) <= 0)
    { /* done, found */
      set_avma(av);
      if (signe(d1)) { *a = icopy(d1); setsigne(*a,s); } else *a = gen_0;
      *b = icopy(v1); return 1;
    }
  } /* while */

  /* We have run into d1 == 0 before returning. This cannot happen */
  pari_err_BUG("ratlift failed to catch d1 == 0");
  return 0; /* LCOV_EXCL_LINE */
}
#line 2 "../src/kernel/none/invmod.c"
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

/*==================================
 * invmod(a,b,res)
 *==================================
 *    If a is invertible, return 1, and set res  = a^{ -1 }
 *    Otherwise, return 0, and set res = gcd(a,b)
 *
 * This is sufficiently different from bezout() to be implemented separately
 * instead of having a bunch of extra conditionals in a single function body
 * to meet both purposes.
 */

#ifdef INVMOD_PARI
INLINE int
invmod_pari(GEN a, GEN b, GEN *res)
#else
int
invmod(GEN a, GEN b, GEN *res)
#endif
{
  GEN v,v1,d,d1,q,r;
  pari_sp av, av1;
  long s;
  ulong g;
  ulong xu,xu1,xv,xv1; /* Lehmer stage recurrence matrix */
  int lhmres; /* Lehmer stage return value */

  if (!signe(b)) { *res=absi(a); return 0; }
  av = avma;
  if (lgefint(b) == 3) /* single-word affair */
  {
    ulong d1 = umodiu(a, uel(b,2));
    if (d1 == 0)
    {
      if (b[2] == 1L)
        { *res = gen_0; return 1; }
      else
        { *res = absi(b); return 0; }
    }
    g = xgcduu(uel(b,2), d1, 1, &xv, &xv1, &s);
    set_avma(av);
    if (g != 1UL) { *res = utoipos(g); return 0; }
    xv = xv1 % uel(b,2); if (s < 0) xv = uel(b,2) - xv;
    *res = utoipos(xv); return 1;
  }

  (void)new_chunk(lgefint(b));
  d = absi_shallow(b); d1 = modii(a,d);

  v=gen_0; v1=gen_1;        /* general case */
  av1 = avma;

  while (lgefint(d) > 3 && signe(d1))
  {
    lhmres = lgcdii((ulong*)d, (ulong*)d1, &xu, &xu1, &xv, &xv1, ULONG_MAX);
    if (lhmres != 0)                /* check progress */
    {                                /* apply matrix */
      if (lhmres == 1 || lhmres == -1)
      {
        if (xv1 == 1)
        {
          r = subii(d,d1); d=d1; d1=r;
          a = subii(v,v1); v=v1; v1=a;
        }
        else
        {
          r = subii(d, mului(xv1,d1)); d=d1; d1=r;
          a = subii(v, mului(xv1,v1)); v=v1; v1=a;
        }
      }
      else
      {
        r  = subii(muliu(d,xu),  muliu(d1,xv));
        a  = subii(muliu(v,xu),  muliu(v1,xv));
        d1 = subii(muliu(d,xu1), muliu(d1,xv1)); d = r;
        v1 = subii(muliu(v,xu1), muliu(v1,xv1)); v = a;
        if (lhmres&1) { togglesign(d);  togglesign(v); }
        else          { togglesign(d1); togglesign(v1); }
      }
    }

    if (lhmres <= 0 && signe(d1))
    {
      q = dvmdii(d,d1,&r);
      a = subii(v,mulii(q,v1));
      v=v1; v1=a;
      d=d1; d1=r;
    }
    if (gc_needed(av,1))
    {
      if(DEBUGMEM>1) pari_warn(warnmem,"invmod");
      gerepileall(av1, 4, &d,&d1,&v,&v1);
    }
  } /* end while */

  /* Postprocessing - final sprint */
  if (signe(d1))
  {
    /* Assertions: lgefint(d)==lgefint(d1)==3, and
     * gcd(d,d1) is nonzero and fits into one word
     */
    g = xxgcduu(uel(d,2), uel(d1,2), 1, &xu, &xu1, &xv, &xv1, &s);
    if (g != 1UL) { set_avma(av); *res = utoipos(g); return 0; }
    /* (From the xgcduu() blurb:)
     * For finishing the multiword modinv, we now have to multiply the
     * returned matrix  (with properly adjusted signs)  onto the values
     * v' and v1' previously obtained from the multiword division steps.
     * Actually, it is sufficient to take the scalar product of [v',v1']
     * with [u1,-v1], and change the sign if s==1.
     */
    v = subii(muliu(v,xu1),muliu(v1,xv1));
    if (s > 0) setsigne(v,-signe(v));
    set_avma(av); *res = modii(v,b); return 1;
  }
  /* get here when the final sprint was skipped (d1 was zero already) */
  set_avma(av);
  if (!equalii(d,gen_1)) { *res = icopy(d); return 0; }
  *res = modii(v,b); return 1;
}
#line 2 "../src/kernel/gmp/gcd.c"
/* Copyright (C) 2000-2003  The PARI group.

This file is part of the PARI/GP package.

PARI/GP is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version. It is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY WHATSOEVER.

Check the License for details. You should have received a copy of it, along
with the package; see the file 'COPYING'. If not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

/* assume y > x > 0. return y mod x */
static ulong
resiu(GEN y, ulong x)
{
  return mpn_mod_1(LIMBS(y), NLIMBS(y), x);
}

GEN
gcdii(GEN a, GEN b)
{
  long v, w;
  pari_sp av;
  GEN t;

  switch (abscmpii(a,b))
  {
    case 0: return absi(a);
    case -1: swap(a,b);
  }
  if (!signe(b)) return absi(a);
  /* here |a|>|b|>0. Try single precision first */
  if (lgefint(a)==3)
    return igcduu((ulong)a[2], (ulong)b[2]);
  if (lgefint(b)==3)
  {
    ulong u = resiu(a,(ulong)b[2]);
    if (!u) return absi(b);
    return igcduu((ulong)b[2], u);
  }
  /* larger than gcd: "set_avma(av)" gerepile (erasing t) is valid */
  av = avma; (void)new_chunk(lgefint(b)+1); /* HACK */
  t = remii(a,b);
  if (!signe(t)) { set_avma(av); return absi(b); }

  a = b; b = t;
  v = vali(a); a = shifti(a,-v); setabssign(a);
  w = vali(b); b = shifti(b,-w); setabssign(b);
  if (w < v) v = w;
  switch(abscmpii(a,b))
  {
    case  0: set_avma(av); a=shifti(a,v); return a;
    case -1: swap(a,b);
  }
  if (is_pm1(b)) { set_avma(av); return int2n(v); }
 {
  /* general case */
  /*This serve two purposes: 1) mpn_gcd destroy its input and need an extra
   * limb 2) this allows us to use icopy instead of gerepile later.  NOTE: we
   * must put u before d else the final icopy could fail.
   */
  GEN res= cgeti(lgefint(a)+1);
  GEN ca = icopy_ef(a,lgefint(a)+1);
  GEN cb = icopy_ef(b,lgefint(b)+1);
  long l = mpn_gcd(LIMBS(res), LIMBS(ca), NLIMBS(ca), LIMBS(cb), NLIMBS(cb));
  res[1] = evalsigne(1)|evallgefint(l+2);
  set_avma(av);
  return shifti(res,v);
  }
}
#line 2 "../src/kernel/gmp/gcdext.c"
/* Copyright (C) 2000-2003  The PARI group.

This file is part of the PARI/GP package.

PARI/GP is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version. It is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY WHATSOEVER.

Check the License for details. You should have received a copy of it, along
with the package; see the file 'COPYING'. If not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

/*==================================
 * invmod(a,b,res)
 *==================================
 *    If a is invertible, return 1, and set res  = a^{ -1 }
 *    Otherwise, return 0, and set res = gcd(a,b)
 */
int
invmod(GEN a, GEN b, GEN *res)
{
  if (!signe(b)) { *res=absi(a); return 0; }
  if (NLIMBS(b) < INVMOD_GMP_LIMIT)
    return invmod_pari(a,b,res);
  { /* General case: use gcdext(a+b, b) since mpn_gcdext require S1>=S2 */
    pari_sp av = avma;
    GEN  ca, cb, u, d;
    long l, su, sa = signe(a), lb,lna;
    mp_size_t lu;
    GEN na;
    if (!sa) { set_avma(av); *res = absi(b); return 0; }
    if (signe(b) < 0) b = negi(b);
    if (abscmpii(a, b) < 0)
      na = sa > 0? addii(a, b): subii(a, b);
    else
      na = a;
    /* Copy serves two purposes:
     * 1) mpn_gcdext destroys its input and needs an extra limb
     * 2) allows us to use icopy instead of gerepile later. */
    lb = lgefint(b); lna = lgefint(na);
    ca = icopy_ef(na,lna+1);
    cb = icopy_ef( b,lb+1);
    /* must create u first else final icopy could fail. */
    u = cgeti(lna+1);
    d = cgeti(lna+1);
    /* na >= b */
    l = mpn_gcdext(LIMBS(d), LIMBS(u), &lu, LIMBS(ca), NLIMBS(ca), LIMBS(cb), NLIMBS(cb));
    d[1] = evalsigne(1)|evallgefint(l+2);
    if (!is_pm1(d)) {set_avma(av); *res=icopy(d); return 0;}
    su = lu?((sa ^ lu) < 0)? -1: 1: 0;
    u[1] = evalsigne(su) | evallgefint(labs(lu)+2);
    if (su < 0) u = addii(u, b);
    set_avma(av); *res=icopy(u); return 1;
  }
}

/*==================================
 * bezout(a,b,pu,pv)
 *==================================
 *    Return g = gcd(a,b) >= 0, and assign GENs u,v through pointers pu,pv
 *    such that g = u*a + v*b.
 * Special cases:
 *    a == b == 0 ==> pick u=1, v=0
 *    a != 0 == b ==> keep v=0
 *    a == 0 != b ==> keep u=0
 *    |a| == |b| != 0 ==> keep u=0, set v=+-1
 * Assignments through pu,pv will be suppressed when the corresponding
 * pointer is NULL  (but the computations will happen nonetheless).
 */

GEN
bezout(GEN a, GEN b, GEN *pu, GEN *pv)
{
  long s, sa, sb;
  ulong g;
  ulong xu,xu1,xv,xv1;                /* Lehmer stage recurrence matrix */

  s = abscmpii(a,b);
  if (s < 0) { swap(a,b); pswap(pu,pv); }
  /* now |a| >= |b| */

  sa = signe(a); sb = signe(b);
  if (!sb)
  {
    if (pv) *pv = gen_0;
    switch(sa)
    {
    case  0: if (pu) *pu = gen_0;  return gen_0;
    case  1: if (pu) *pu = gen_1;  return icopy(a);
    case -1: if (pu) *pu = gen_m1; return negi(a);
    }
  }
  if (s == 0)                        /* |a| == |b| != 0 */
  {
    if (pu) *pu = gen_0;
    if (sb > 0)
    { if (pv) *pv = gen_1;  return icopy(b); }
    else
    { if (pv) *pv = gen_m1; return negi(b); }
  }
  /* now |a| > |b| > 0 */

  if (lgefint(a) == 3)                /* single-word affair */
  {
    g = xxgcduu((ulong)a[2], (ulong)b[2], 0, &xu, &xu1, &xv, &xv1, &s);
    sa = s > 0 ? sa : -sa;
    sb = s > 0 ? -sb : sb;
    if (pu)
    {
      if (xu == 0) *pu = gen_0; /* can happen when b divides a */
      else if (xu == 1) *pu = sa < 0 ? gen_m1 : gen_1;
      else if (xu == 2) *pu = sa < 0 ? gen_m2 : gen_2;
      else
      {
        *pu = cgeti(3);
        (*pu)[1] = evalsigne(sa)|evallgefint(3);
        (*pu)[2] = xu;
      }
    }
    if (pv)
    {
      if (xv == 1) *pv = sb < 0 ? gen_m1 : gen_1;
      else if (xv == 2) *pv = sb < 0 ? gen_m2 : gen_2;
      else
      {
        *pv = cgeti(3);
        (*pv)[1] = evalsigne(sb)|evallgefint(3);
        (*pv)[2] = xv;
      }
    }
    if (g == 1) return gen_1;
    else if (g == 2) return gen_2;
    else return utoipos(g);
  }
  else
  { /* general case */
    pari_sp av = avma;
    /*Copy serves two purposes:
     * 1) mpn_gcdext destroys its input and needs an extra limb
     * 2) allows us to use icopy instead of gerepile later.
     * NOTE: we must put u before d else the final icopy could fail. */
    GEN ca = icopy_ef(a,lgefint(a)+1);
    GEN cb = icopy_ef(b,lgefint(b)+1);
    GEN u = cgeti(lgefint(a)+1), v = NULL;
    GEN d = cgeti(lgefint(a)+1);
    long su,l;
    mp_size_t lu;
    l = mpn_gcdext(LIMBS(d), LIMBS(u), &lu, LIMBS(ca), NLIMBS(ca), LIMBS(cb), NLIMBS(cb));
    if (lu<=0)
    {
      if (lu==0) su=0;
      else {su=-1;lu=-lu;}
    }
    else
      su=1;
    if (sa<0) su=-su;
    d[1] = evalsigne(1)|evallgefint(l+2);
    u[1] = evalsigne(su)|evallgefint(lu+2);
    if (pv) v=diviiexact(subii(d,mulii(u,a)),b);
    set_avma(av);
    if (pu) *pu=icopy(u);
    if (pv) *pv=icopy(v);
    return icopy(d);
  }
}
#line 2 "../src/kernel/none/mp_indep.c"
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

/* Find c such that 1=c*b mod 2^BITS_IN_LONG, assuming b odd (unchecked) */
ulong
invmod2BIL(ulong b)
{
  static int tab[] = { 0, 0, 0, 8, 0, 8, 0, 0 };
  ulong x = b + tab[b & 7]; /* b^(-1) mod 2^4 */

  /* Newton applied to 1/x - b = 0 */
#ifdef LONG_IS_64BIT
  x = x*(2-b*x); /* one more pass necessary */
#endif
  x = x*(2-b*x);
  x = x*(2-b*x); return x*(2-b*x);
}

void
affrr(GEN x, GEN y)
{
  long i, lx, ly = lg(y);
  if (!signe(x))
  {
    y[1] = evalexpo(minss(expo(x), -bit_accuracy(ly)));
    return;
  }
  y[1] = x[1]; lx = lg(x);
  if (lx <= ly)
  {
    for (i=2; i<lx; i++) y[i]=x[i];
    for (   ; i<ly; i++) y[i]=0;
    return;
  }
  for (i=2; i<ly; i++) y[i]=x[i];
  /* lx > ly: round properly */
  if (x[ly] & HIGHBIT) roundr_up_ip(y, ly);
}

GEN
trunc2nr(GEN x, long n)
{
  long ex;
  if (!signe(x)) return gen_0;
  ex = expo(x) + n; if (ex < 0) return gen_0;
  return mantissa2nr(x, ex - bit_prec(x) + 1);
}

/* x a t_REAL, x = i/2^e, i a t_INT */
GEN
mantissa_real(GEN x, long *e)
{
  *e = bit_prec(x)-1-expo(x);
  return mantissa2nr(x, 0);
}

GEN
mului(ulong x, GEN y)
{
  long s = signe(y);
  GEN z;

  if (!s || !x) return gen_0;
  z = muluispec(x, y+2, lgefint(y)-2);
  setsigne(z,s); return z;
}

GEN
mulsi(long x, GEN y)
{
  long s = signe(y);
  GEN z;

  if (!s || !x) return gen_0;
  if (x<0) { s = -s; x = -x; }
  z = muluispec((ulong)x, y+2, lgefint(y)-2);
  setsigne(z,s); return z;
}

GEN
mulss(long x, long y)
{
  long p1;
  LOCAL_HIREMAINDER;

  if (!x || !y) return gen_0;
  if (x<0) {
    x = -x;
    if (y<0) { y = -y; p1 = mulll(x,y); return uutoi(hiremainder, p1); }
    p1 = mulll(x,y); return uutoineg(hiremainder, p1);
  } else {
    if (y<0) { y = -y; p1 = mulll(x,y); return uutoineg(hiremainder, p1); }
    p1 = mulll(x,y); return uutoi(hiremainder, p1);
  }
}
GEN
sqrs(long x)
{
  long p1;
  LOCAL_HIREMAINDER;

  if (!x) return gen_0;
  if (x<0) x = -x;
  p1 = mulll(x,x); return uutoi(hiremainder, p1);
}
GEN
muluu(ulong x, ulong y)
{
  long p1;
  LOCAL_HIREMAINDER;

  if (!x || !y) return gen_0;
  p1 = mulll(x,y); return uutoi(hiremainder, p1);
}
GEN
sqru(ulong x)
{
  long p1;
  LOCAL_HIREMAINDER;

  if (!x) return gen_0;
  p1 = mulll(x,x); return uutoi(hiremainder, p1);
}

/* assume x > 1, y != 0. Return u * y with sign s */
static GEN
mulur_2(ulong x, GEN y, long s)
{
  long m, sh, i, lx = lg(y), e = expo(y);
  GEN z = cgetr(lx);
  ulong garde;
  LOCAL_HIREMAINDER;

  y--; garde = mulll(x,y[lx]);
  for (i=lx-1; i>=3; i--) z[i]=addmul(x,y[i]);
  z[2]=hiremainder; /* != 0 since y normalized and |x| > 1 */
  sh = bfffo(hiremainder); m = BITS_IN_LONG-sh;
  if (sh) shift_left(z,z, 2,lx-1, garde,sh);
  z[1] = evalsigne(s) | evalexpo(m+e);
  if ((garde << sh) & HIGHBIT) roundr_up_ip(z, lx);
  return z;
}

INLINE GEN
mul0r(GEN x)
{
  long l = lg(x), e = expo(x);
  e = (l > 2)? -prec2nbits(l) + e: (e < 0? 2*e: 0);
  return real_0_bit(e);
}
/* lg(x) > 2 */
INLINE GEN
div0r(GEN x) {
  long l = lg(x), e = expo(x);
  return real_0_bit(-prec2nbits(l) - e);
}

GEN
mulsr(long x, GEN y)
{
  long s;

  if (!x) return mul0r(y);
  s = signe(y);
  if (!s)
  {
    if (x < 0) x = -x;
    return real_0_bit( expo(y) + expu(x) );
  }
  if (x==1)  return rcopy(y);
  if (x==-1) return negr(y);
  if (x < 0)
    return mulur_2((ulong)-x, y, -s);
  else
    return mulur_2((ulong)x, y, s);
}

GEN
mulur(ulong x, GEN y)
{
  long s;

  if (!x) return mul0r(y);
  s = signe(y);
  if (!s) return real_0_bit( expo(y) + expu(x) );
  if (x==1) return rcopy(y);
  return mulur_2(x, y, s);
}

INLINE void
mulrrz_end(GEN z, GEN hi, long lz, long sz, long ez, ulong garde)
{
  long i;
  if (hi[2] < 0)
  {
    if (z != hi)
      for (i=2; i<lz ; i++) z[i] = hi[i];
    ez++;
  }
  else
  {
    shift_left(z,hi,2,lz-1, garde, 1);
    garde <<= 1;
  }
  if (garde & HIGHBIT)
  { /* round to nearest */
    i = lz; do ((ulong*)z)[--i]++; while (i>1 && z[i]==0);
    if (i == 1) { z[2] = (long)HIGHBIT; ez++; }
  }
  z[1] = evalsigne(sz)|evalexpo(ez);
}
/* mulrrz_end for lz = 3, minor simplifications. z[2]=hiremainder from mulll */
INLINE void
mulrrz_3end(GEN z, long sz, long ez, ulong garde)
{
  if (z[2] < 0)
  { /* z2 < (2^BIL-1)^2 / 2^BIL, hence z2+1 != 0 */
    if (garde & HIGHBIT) z[2]++; /* round properly */
    ez++;
  }
  else
  {
    uel(z,2) = (uel(z,2)<<1) | (garde>>(BITS_IN_LONG-1));
    if (garde & (1UL<<(BITS_IN_LONG-2)))
    {
      uel(z,2)++; /* round properly, z2+1 can overflow */
      if (!uel(z,2)) { uel(z,2) = HIGHBIT; ez++; }
    }
  }
  z[1] = evalsigne(sz)|evalexpo(ez);
}

/* set z <-- x^2 != 0, floating point multiplication.
 * lz = lg(z) = lg(x) */
INLINE void
sqrz_i(GEN z, GEN x, long lz)
{
  long ez = 2*expo(x);
  long i, j, lzz, p1;
  ulong garde;
  GEN x1;
  LOCAL_HIREMAINDER;
  LOCAL_OVERFLOW;

  if (lz > SQRR_SQRI_LIMIT)
  {
    pari_sp av = avma;
    GEN hi = sqrispec_mirror(x+2, lz-2);
    mulrrz_end(z, hi, lz, 1, ez, hi[lz]);
    set_avma(av); return;
  }
  if (lz == 3)
  {
    garde = mulll(x[2],x[2]);
    z[2] = hiremainder;
    mulrrz_3end(z, 1, ez, garde);
    return;
  }

  lzz = lz-1; p1 = x[lzz];
  if (p1)
  {
    (void)mulll(p1,x[3]);
    garde = addmul(p1,x[2]);
    z[lzz] = hiremainder;
  }
  else
  {
    garde = 0;
    z[lzz] = 0;
  }
  for (j=lz-2, x1=x-j; j>=3; j--)
  {
    p1 = x[j]; x1++;
    if (p1)
    {
      (void)mulll(p1,x1[lz+1]);
      garde = addll(addmul(p1,x1[lz]), garde);
      for (i=lzz; i>j; i--)
      {
        hiremainder += overflow;
        z[i] = addll(addmul(p1,x1[i]), z[i]);
      }
      z[j] = hiremainder+overflow;
    }
    else z[j]=0;
  }
  p1 = x[2]; x1++;
  garde = addll(mulll(p1,x1[lz]), garde);
  for (i=lzz; i>2; i--)
  {
    hiremainder += overflow;
    z[i] = addll(addmul(p1,x1[i]), z[i]);
  }
  z[2] = hiremainder+overflow;
  mulrrz_end(z, z, lz, 1, ez, garde);
}

/* lz "large" = lg(y) = lg(z), lg(x) > lz if flag = 1 and >= if flag = 0 */
INLINE void
mulrrz_int(GEN z, GEN x, GEN y, long lz, long flag, long sz)
{
  pari_sp av = avma;
  GEN hi = muliispec_mirror(y+2, x+2, lz+flag-2, lz-2);
  mulrrz_end(z, hi, lz, sz, expo(x)+expo(y), hi[lz]);
  set_avma(av);
}

/* lz = 3 */
INLINE void
mulrrz_3(GEN z, GEN x, GEN y, long flag, long sz)
{
  ulong garde;
  LOCAL_HIREMAINDER;
  if (flag)
  {
    (void)mulll(x[2],y[3]);
    garde = addmul(x[2],y[2]);
  }
  else
    garde = mulll(x[2],y[2]);
  z[2] = hiremainder;
  mulrrz_3end(z, sz, expo(x)+expo(y), garde);
}

/* set z <-- x*y, floating point multiplication. Trailing 0s for x are
 * treated efficiently (important application: mulir).
 * lz = lg(z) = lg(x) <= ly <= lg(y), sz = signe(z). flag = lg(x) < lg(y) */
INLINE void
mulrrz_i(GEN z, GEN x, GEN y, long lz, long flag, long sz)
{
  long ez, i, j, lzz, p1;
  ulong garde;
  GEN y1;
  LOCAL_HIREMAINDER;
  LOCAL_OVERFLOW;

  if (x == y) { sqrz_i(z,x,lz); return; }
  if (lz > MULRR_MULII_LIMIT) { mulrrz_int(z,x,y,lz,flag,sz); return; }
  if (lz == 3) { mulrrz_3(z,x,y,flag,sz); return; }
  ez = expo(x) + expo(y);
  if (flag) { (void)mulll(x[2],y[lz]); garde = hiremainder; } else garde = 0;
  lzz=lz-1; p1=x[lzz];
  if (p1)
  {
    (void)mulll(p1,y[3]);
    garde = addll(addmul(p1,y[2]), garde);
    z[lzz] = overflow+hiremainder;
  }
  else z[lzz]=0;
  for (j=lz-2, y1=y-j; j>=3; j--)
  {
    p1 = x[j]; y1++;
    if (p1)
    {
      (void)mulll(p1,y1[lz+1]);
      garde = addll(addmul(p1,y1[lz]), garde);
      for (i=lzz; i>j; i--)
      {
        hiremainder += overflow;
        z[i] = addll(addmul(p1,y1[i]), z[i]);
      }
      z[j] = hiremainder+overflow;
    }
    else z[j]=0;
  }
  p1 = x[2]; y1++;
  garde = addll(mulll(p1,y1[lz]), garde);
  for (i=lzz; i>2; i--)
  {
    hiremainder += overflow;
    z[i] = addll(addmul(p1,y1[i]), z[i]);
  }
  z[2] = hiremainder+overflow;
  mulrrz_end(z, z, lz, sz, ez, garde);
}

GEN
mulrr(GEN x, GEN y)
{
  long flag, ly, lz, sx, sy;
  GEN z;

  if (x == y) return sqrr(x);
  sx = signe(x); if (!sx) return real_0_bit(expo(x) + expo(y));
  sy = signe(y); if (!sy) return real_0_bit(expo(x) + expo(y));
  if (sy < 0) sx = -sx;
  lz = lg(x);
  ly = lg(y);
  if (lz > ly) { lz = ly; swap(x, y); flag = 1; } else flag = (lz != ly);
  z = cgetr(lz);
  mulrrz_i(z, x,y, lz,flag, sx);
  return z;
}

GEN
sqrr(GEN x)
{
  long lz, sx = signe(x);
  GEN z;

  if (!sx) return real_0_bit(2*expo(x));
  lz = lg(x); z = cgetr(lz);
  sqrz_i(z, x, lz);
  return z;
}

GEN
mulir(GEN x, GEN y)
{
  long sx = signe(x), sy;
  if (!sx) return mul0r(y);
  if (lgefint(x) == 3) {
    GEN z = mulur(uel(x,2), y);
    if (sx < 0) togglesign(z);
    return z;
  }
  sy = signe(y);
  if (!sy) return real_0_bit(expi(x) + expo(y));
  if (sy < 0) sx = -sx;
  {
    long lz = lg(y), lx = lgefint(x);
    GEN hi, z = cgetr(lz);
    pari_sp av = avma;
    if (lx < (lz>>1) || (lx < lz && lz > MULRR_MULII_LIMIT))
    { /* size mantissa of x < half size of mantissa z, or lx < lz so large
       * that mulrr will call mulii anyway: mulii */
      x = itor(x, lx);
      hi = muliispec_mirror(y+2, x+2, lz-2, lx-2);
      mulrrz_end(z, hi, lz, sx, expo(x)+expo(y), hi[lz]);
    }
    else /* dubious: complete x with 0s and call mulrr */
      mulrrz_i(z, itor(x,lz), y, lz, 0, sx);
    set_avma(av); return z;
  }
}

/* x + y*z, generic. If lgefint(z) <= 3, caller should use faster variants  */
static GEN
addmulii_gen(GEN x, GEN y, GEN z, long lz)
{
  long lx = lgefint(x), ly;
  pari_sp av;
  GEN t;
  if (lx == 2) return mulii(z,y);
  ly = lgefint(y);
  if (ly == 2) return icopy(x); /* y = 0, wasteful copy */
  av = avma; (void)new_chunk(lx+ly+lz); /*HACK*/
  t = mulii(z, y);
  set_avma(av); return addii(t,x);
}
/* x + y*z, lgefint(z) == 3 */
static GEN
addmulii_lg3(GEN x, GEN y, GEN z)
{
  long s = signe(z), lx, ly;
  ulong w = z[2];
  pari_sp av;
  GEN t;
  if (w == 1) return (s > 0)? addii(x,y): subii(x,y); /* z = +- 1 */
  lx = lgefint(x);
  ly = lgefint(y);
  if (lx == 2)
  { /* x = 0 */
    if (ly == 2) return gen_0;
    t = muluispec(w, y+2, ly-2);
    if (signe(y) < 0) s = -s;
    setsigne(t, s); return t;
  }
  if (ly == 2) return icopy(x); /* y = 0, wasteful copy */
  av = avma; (void)new_chunk(1+lx+ly);/*HACK*/
  t = muluispec(w, y+2, ly-2);
  if (signe(y) < 0) s = -s;
  setsigne(t, s);
  set_avma(av); return addii(x,t);
}
/* x + y*z */
GEN
addmulii(GEN x, GEN y, GEN z)
{
  long lz = lgefint(z);
  switch(lz)
  {
    case 2: return icopy(x); /* z = 0, wasteful copy */
    case 3: return addmulii_lg3(x, y, z);
    default:return addmulii_gen(x, y, z, lz);
  }
}
/* x + y*z, returns x itself and not a copy when y*z = 0 */
GEN
addmulii_inplace(GEN x, GEN y, GEN z)
{
  long lz;
  if (lgefint(y) == 2) return x;
  lz = lgefint(z);
  switch(lz)
  {
    case 2: return x;
    case 3: return addmulii_lg3(x, y, z);
    default:return addmulii_gen(x, y, z, lz);
  }
}

/* written by Bruno Haible following an idea of Robert Harley */
long
vals(ulong z)
{
  static char tab[64]={-1,0,1,12,2,6,-1,13,3,-1,7,-1,-1,-1,-1,14,10,4,-1,-1,8,-1,-1,25,-1,-1,-1,-1,-1,21,27,15,31,11,5,-1,-1,-1,-1,-1,9,-1,-1,24,-1,-1,20,26,30,-1,-1,-1,-1,23,-1,19,29,-1,22,18,28,17,16,-1};
#ifdef LONG_IS_64BIT
  long s;
#endif

  if (!z) return -1;
#ifdef LONG_IS_64BIT
  if (! (z&0xffffffff)) { s = 32; z >>=32; } else s = 0;
#endif
  z |= ~z + 1;
  z += z << 4;
  z += z << 6;
  z ^= z << 16; /* or  z -= z<<16 */
#ifdef LONG_IS_64BIT
  return s + tab[(z&0xffffffff)>>26];
#else
  return tab[z>>26];
#endif
}

GEN
divsi(long x, GEN y)
{
  long p1, s = signe(y);
  LOCAL_HIREMAINDER;

  if (!s) pari_err_INV("divsi",gen_0);
  if (!x || lgefint(y)>3 || ((long)y[2])<0) return gen_0;
  hiremainder=0; p1=divll(labs(x),y[2]);
  if (x<0) { hiremainder = -((long)hiremainder); p1 = -p1; }
  if (s<0) p1 = -p1;
  return stoi(p1);
}

GEN
divir(GEN x, GEN y)
{
  GEN z;
  long ly = lg(y), lx = lgefint(x);
  pari_sp av;

  if (ly == 2) pari_err_INV("divir",y);
  if (lx == 2) return div0r(y);
  if (lx == 3) {
    z = divur(x[2], y);
    if (signe(x) < 0) togglesign(z);
    return z;
  }
  z = cgetr(ly); av = avma;
  affrr(divrr(itor(x, ly+1), y), z);
  set_avma(av); return z;
}

GEN
divur(ulong x, GEN y)
{
  pari_sp av;
  long ly = lg(y);
  GEN z;

  if (ly == 2) pari_err_INV("divur",y);
  if (!x) return div0r(y);
  if (ly > INVNEWTON_LIMIT) {
    av = avma; z = invr(y);
    if (x == 1) return z;
    return gerepileuptoleaf(av, mulur(x, z));
  }
  z = cgetr(ly); av = avma;
  affrr(divrr(utor(x,ly+1), y), z);
  set_avma(av); return z;
}

GEN
divsr(long x, GEN y)
{
  pari_sp av;
  long ly = lg(y);
  GEN z;

  if (ly == 2) pari_err_INV("divsr",y);
  if (!x) return div0r(y);
  if (ly > INVNEWTON_LIMIT) {
    av = avma; z = invr(y);
    if (x == 1) return z;
    if (x ==-1) { togglesign(z); return z; }
    return gerepileuptoleaf(av, mulsr(x, z));
  }
  z = cgetr(ly); av = avma;
  affrr(divrr(stor(x,ly+1), y), z);
  set_avma(av); return z;
}

/* returns 1/y, assume y != 0 */
static GEN
invr_basecase(GEN y)
{
  long ly = lg(y);
  GEN z = cgetr(ly);
  pari_sp av = avma;
  affrr(divrr(real_1(ly+1), y), z);
  set_avma(av); return z;
}
/* returns 1/b, Newton iteration */
GEN
invr(GEN b)
{
  const long s = 6;
  long i, p, l = lg(b);
  GEN x, a;
  ulong mask;

  if (l <= maxss(INVNEWTON_LIMIT, (1L<<s) + 2)) {
    if (l == 2) pari_err_INV("invr",b);
    return invr_basecase(b);
  }
  mask = quadratic_prec_mask(l-2);
  for(i=0, p=1; i<s; i++) { p <<= 1; if (mask & 1) p--; mask >>= 1; }
  x = cgetr(l);
  a = rcopy(b); a[1] = _evalexpo(0) | evalsigne(1);
  affrr(invr_basecase(rtor(a, p+2)), x);
  while (mask > 1)
  {
    p <<= 1; if (mask & 1) p--;
    mask >>= 1;
    setlg(a, p + 2);
    setlg(x, p + 2);
    /* TODO: mulrr(a,x) should be a half product (the higher half is known).
     * mulrr(x, ) already is */
    affrr(addrr(x, mulrr(x, subsr(1, mulrr(a,x)))), x);
    set_avma((pari_sp)a);
  }
  x[1] = (b[1] & SIGNBITS) | evalexpo(expo(x)-expo(b));
  set_avma((pari_sp)x); return x;
}

GEN
modii(GEN x, GEN y)
{
  switch(signe(x))
  {
    case 0: return gen_0;
    case 1: return remii(x,y);
    default:
    {
      pari_sp av = avma;
      (void)new_chunk(lgefint(y));
      x = remii(x,y); set_avma(av);
      if (x==gen_0) return x;
      return subiispec(y+2,x+2,lgefint(y)-2,lgefint(x)-2);
    }
  }
}

void
modiiz(GEN x, GEN y, GEN z)
{
  const pari_sp av = avma;
  affii(modii(x,y),z); set_avma(av);
}

GEN
divrs(GEN x, long y)
{
  GEN z;
  if (y < 0)
  {
    z = divru(x, (ulong)-y);
    togglesign(z);
  }
  else
    z = divru(x, (ulong)y);
  return z;
}

GEN
divru(GEN x, ulong y)
{
  long i, lx, sh, e, s = signe(x);
  ulong garde;
  GEN z;
  LOCAL_HIREMAINDER;

  if (!y) pari_err_INV("divru",gen_0);
  if (!s) return real_0_bit(expo(x) - expu(y));
  if (!(y & (y-1))) /* power of 2 */
  {
    if (y == 1) return rcopy(x);
    return shiftr(x, -expu(y));
  }
  e = expo(x);
  lx = lg(x);
  z = cgetr(lx);
  if (lx == 3)
  {
    if (y <= uel(x,2))
    {
      hiremainder = 0;
      z[2] = divll(x[2],y);
      /* we may have hiremainder != 0 ==> garde */
      garde = divll(0,y);
    }
    else
    {
      hiremainder = x[2];
      z[2] = divll(0,y);
      garde = hiremainder;
      e -= BITS_IN_LONG;
    }
  }
  else
  {
    ulong yp = get_Fl_red(y);
    if (y <= uel(x,2))
    {
      hiremainder = 0;
      for (i=2; i<lx; i++) z[i] = divll_pre(x[i],y,yp);
      /* we may have hiremainder != 0 ==> garde */
      garde = divll_pre(0,y,yp);
    }
    else
    {
      long l = lx-1;
      hiremainder = x[2];
      for (i=2; i<l; i++) z[i] = divll_pre(x[i+1],y,yp);
      z[i] = divll_pre(0,y,yp);
      garde = hiremainder;
      e -= BITS_IN_LONG;
    }
  }
  sh=bfffo(z[2]); /* z[2] != 0 */
  if (sh) shift_left(z,z, 2,lx-1, garde,sh);
  z[1] = evalsigne(s) | evalexpo(e-sh);
  if ((garde << sh) & HIGHBIT) roundr_up_ip(z, lx);
  return z;
}

GEN
truedvmdii(GEN x, GEN y, GEN *z)
{
  pari_sp av;
  GEN r, q, *gptr[2];
  if (!is_bigint(y)) return truedvmdis(x, itos(y), z);
  if (z == ONLY_REM) return modii(x,y);

  av = avma;
  q = dvmdii(x,y,&r); /* assume that r is last on stack */
  switch(signe(r))
  {
    case 0:
      if (z) *z = gen_0;
      return q;
    case 1:
      if (z) *z = r; else cgiv(r);
      return q;
    case -1: break;
  }
  q = addis(q, -signe(y));
  if (!z) return gerepileuptoint(av, q);

  *z = subiispec(y+2,r+2, lgefint(y)-2,lgefint(r)-2);
  gptr[0]=&q; gptr[1]=z; gerepilemanysp(av,(pari_sp)r,gptr,2);
  return q;
}
GEN
truedvmdis(GEN x, long y, GEN *z)
{
  pari_sp av = avma;
  long r;
  GEN q;

  if (z == ONLY_REM) return modis(x, y);
  q = divis_rem(x,y,&r);

  if (r >= 0)
  {
    if (z) *z = utoi(r);
    return q;
  }
  q = gerepileuptoint(av, addis(q, (y < 0)? 1: -1));
  if (z) *z = utoi(r + labs(y));
  return q;
}
GEN
truedvmdsi(long x, GEN y, GEN *z)
{
  long q, r;
  if (z == ONLY_REM) return modsi(x, y);
  q = sdivsi_rem(x,y,&r);
  if (r >= 0) {
    if (z) *z = utoi(r);
    return stoi(q);
  }
  q = q - signe(y);
  if (!z) return stoi(q);

  *z = subiuspec(y+2,(ulong)-r, lgefint(y)-2);
  return stoi(q);
}

/* 2^n = shifti(gen_1, n) */
GEN
int2n(long n) {
  long i, m, l;
  GEN z;
  if (n < 0) return gen_0;
  if (n == 0) return gen_1;

  l = dvmdsBIL(n, &m) + 3;
  z = cgetipos(l);
  for (i = 2; i < l; i++) z[i] = 0;
  *int_MSW(z) = 1UL << m; return z;
}
/* To avoid problems when 2^(BIL-1) < n. Overflow cleanly, where int2n
 * returns gen_0 */
GEN
int2u(ulong n) {
  ulong i, m, l;
  GEN z;
  if (n == 0) return gen_1;

  l = dvmduBIL(n, &m) + 3;
  z = cgetipos(l);
  for (i = 2; i < l; i++) z[i] = 0;
  *int_MSW(z) = 1UL << m; return z;
}
/* 2^n - 1 */
GEN
int2um1(ulong n) {
  ulong i, m, l;
  GEN z;
  if (n == 0) return gen_0;

  l = dvmduBIL(n, &m);
  l += m? 3: 2;
  z = cgetipos(l);
  for (i = 2; i < l; i++) z[i] = ~0UL;
  if (m) *int_MSW(z) = (1UL << m) - 1;
  return z;
}

GEN
shifti(GEN x, long n)
{
  long s = signe(x);
  GEN y;

  if(s == 0) return gen_0;
  y = shiftispec(x + 2, lgefint(x) - 2, n);
  if (signe(y)) setsigne(y, s);
  return y;
}

/* actual operations will take place on a+2 and b+2: we strip the codewords */
GEN
mulii(GEN a,GEN b)
{
  long sa,sb;
  GEN z;

  sa=signe(a); if (!sa) return gen_0;
  sb=signe(b); if (!sb) return gen_0;
  if (sb<0) sa = -sa;
  z = muliispec(a+2,b+2, lgefint(a)-2,lgefint(b)-2);
  setsigne(z,sa); return z;
}

GEN
sqri(GEN a) { return sqrispec(a+2, lgefint(a)-2); }

/* sqrt()'s result may be off by 1 when a is not representable exactly as a
 * double [64bit machine] */
ulong
usqrt(ulong a)
{
  ulong x = (ulong)sqrt((double)a);
#ifdef LONG_IS_64BIT
  if (x > LOWMASK || x*x > a) x--;
#endif
  return x;
}

/********************************************************************/
/**                                                                **/
/**              EXPONENT / CONVERSION t_REAL --> double           **/
/**                                                                **/
/********************************************************************/

#ifdef LONG_IS_64BIT
long
dblexpo(double x)
{
  union { double f; ulong i; } fi;
  const int mant_len = 52;  /* mantissa bits (excl. hidden bit) */
  const int exp_mid = 0x3ff;/* exponent bias */

  if (x==0.) return -exp_mid;
  fi.f = x;
  return ((fi.i & (HIGHBIT-1)) >> mant_len) - exp_mid;
}

ulong
dblmantissa(double x)
{
  union { double f; ulong i; } fi;
  const int expo_len = 11; /* number of bits of exponent */

  if (x==0.) return 0;
  fi.f = x;
  return (fi.i << expo_len) | HIGHBIT;
}

GEN
dbltor(double x)
{
  GEN z;
  long e;
  union { double f; ulong i; } fi;
  const int mant_len = 52;  /* mantissa bits (excl. hidden bit) */
  const int exp_mid = 0x3ff;/* exponent bias */
  const int expo_len = 11; /* number of bits of exponent */

  if (x==0.) return real_0_bit(-exp_mid);
  fi.f = x; z = cgetr(DEFAULTPREC);
  {
    const ulong a = fi.i;
    ulong A;
    e = ((a & (HIGHBIT-1)) >> mant_len) - exp_mid;
    if (e == exp_mid+1) pari_err_OVERFLOW("dbltor [NaN or Infinity]");
    A = a << expo_len;
    if (e == -exp_mid)
    { /* unnormalized values */
      int sh = bfffo(A);
      e -= sh-1;
      z[2] = A << sh;
    }
    else
      z[2] = HIGHBIT | A;
    z[1] = _evalexpo(e) | evalsigne(x<0? -1: 1);
  }
  return z;
}

double
rtodbl(GEN x)
{
  long ex,s=signe(x);
  ulong a;
  union { double f; ulong i; } fi;
  const int mant_len = 52;  /* mantissa bits (excl. hidden bit) */
  const int exp_mid = 0x3ff;/* exponent bias */
  const int expo_len = 11; /* number of bits of exponent */

  if (!s || (ex=expo(x)) < - exp_mid) return 0.0;

  /* start by rounding to closest */
  a = (x[2] & (HIGHBIT-1)) + 0x400;
  if (a & HIGHBIT) { ex++; a=0; }
  if (ex >= exp_mid) pari_err_OVERFLOW("t_REAL->double conversion");
  fi.i = ((ex + exp_mid) << mant_len) | (a >> expo_len);
  if (s<0) fi.i |= HIGHBIT;
  return fi.f;
}

#else /* LONG_IS_64BIT */

#if   PARI_DOUBLE_FORMAT == 1
#  define INDEX0 1
#  define INDEX1 0
#elif PARI_DOUBLE_FORMAT == 0
#  define INDEX0 0
#  define INDEX1 1
#endif

long
dblexpo(double x)
{
  union { double f; ulong i[2]; } fi;
  const int mant_len = 52;  /* mantissa bits (excl. hidden bit) */
  const int exp_mid = 0x3ff;/* exponent bias */
  const int shift = mant_len-32;

  if (x==0.) return -exp_mid;
  fi.f = x;
  {
    const ulong a = fi.i[INDEX0];
    return ((a & (HIGHBIT-1)) >> shift) - exp_mid;
  }
}

ulong
dblmantissa(double x)
{
  union { double f; ulong i[2]; } fi;
  const int expo_len = 11; /* number of bits of exponent */

  if (x==0.) return 0;
  fi.f = x;
  {
    const ulong a = fi.i[INDEX0];
    const ulong b = fi.i[INDEX1];
    return HIGHBIT | b >> (BITS_IN_LONG-expo_len) | (a << expo_len);
  }
}

GEN
dbltor(double x)
{
  GEN z;
  long e;
  union { double f; ulong i[2]; } fi;
  const int mant_len = 52;  /* mantissa bits (excl. hidden bit) */
  const int exp_mid = 0x3ff;/* exponent bias */
  const int expo_len = 11; /* number of bits of exponent */
  const int shift = mant_len-32;

  if (x==0.) return real_0_bit(-exp_mid);
  fi.f = x; z = cgetr(DEFAULTPREC);
  {
    const ulong a = fi.i[INDEX0];
    const ulong b = fi.i[INDEX1];
    ulong A, B;
    e = ((a & (HIGHBIT-1)) >> shift) - exp_mid;
    if (e == exp_mid+1) pari_err_OVERFLOW("dbltor [NaN or Infinity]");
    A = b >> (BITS_IN_LONG-expo_len) | (a << expo_len);
    B = b << expo_len;
    if (e == -exp_mid)
    { /* unnormalized values */
      int sh;
      if (A)
      {
        sh = bfffo(A);
        e -= sh-1;
        z[2] = (A << sh) | (B >> (32-sh));
        z[3] = B << sh;
      }
      else
      {
        sh = bfffo(B); /* B != 0 */
        e -= sh-1 + 32;
        z[2] = B << sh;
        z[3] = 0;
      }
    }
    else
    {
      z[3] = B;
      z[2] = HIGHBIT | A;
    }
    z[1] = _evalexpo(e) | evalsigne(x<0? -1: 1);
  }
  return z;
}

double
rtodbl(GEN x)
{
  long ex,s=signe(x),lx=lg(x);
  ulong a,b,k;
  union { double f; ulong i[2]; } fi;
  const int mant_len = 52;  /* mantissa bits (excl. hidden bit) */
  const int exp_mid = 0x3ff;/* exponent bias */
  const int expo_len = 11; /* number of bits of exponent */
  const int shift = mant_len-32;

  if (!s || (ex=expo(x)) < - exp_mid) return 0.0;

  /* start by rounding to closest */
  a = x[2] & (HIGHBIT-1);
  if (lx > 3)
  {
    b = x[3] + 0x400UL; if (b < 0x400UL) a++;
    if (a & HIGHBIT) { ex++; a=0; }
  }
  else b = 0;
  if (ex >= exp_mid) pari_err_OVERFLOW("t_REAL->double conversion");
  ex += exp_mid;
  k = (a >> expo_len) | (ex << shift);
  if (s<0) k |= HIGHBIT;
  fi.i[INDEX0] = k;
  fi.i[INDEX1] = (a << (BITS_IN_LONG-expo_len)) | (b >> expo_len);
  return fi.f;
}
#endif /* LONG_IS_64BIT */

#line 2 "../src/kernel/none/add.c"
/* Copyright (C) 2002-2003  The PARI group.

This file is part of the PARI/GP package.

PARI/GP is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version. It is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY WHATSOEVER.

Check the License for details. You should have received a copy of it, along
with the package; see the file 'COPYING'. If not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

INLINE GEN
icopy_sign(GEN x, long sx)
{
  GEN y=icopy(x);
  setsigne(y,sx);
  return y;
}

GEN
addsi_sign(long x, GEN y, long sy)
{
  long sx,ly;
  GEN z;

  if (!x) return icopy_sign(y, sy);
  if (!sy) return stoi(x);
  if (x<0) { sx=-1; x=-x; } else sx=1;
  if (sx==sy)
  {
    z = adduispec(x,y+2, lgefint(y)-2);
    setsigne(z,sy); return z;
  }
  ly=lgefint(y);
  if (ly==3)
  {
    const long d = (long)(uel(y,2) - (ulong)x);
    if (!d) return gen_0;
    z=cgeti(3);
    if (y[2] < 0 || d > 0) {
      z[1] = evalsigne(sy) | evallgefint(3);
      z[2] = d;
    }
    else {
      z[1] = evalsigne(-sy) | evallgefint(3);
      z[2] =-d;
    }
    return z;
  }
  z = subiuspec(y+2,x, ly-2);
  setsigne(z,sy); return z;
}
GEN
addui_sign(ulong x, GEN y, long sy)
{
  long ly;
  GEN z;

  if (!x) return icopy_sign(y, sy);
  if (!sy) return utoipos(x);
  if (sy == 1) return adduispec(x,y+2, lgefint(y)-2);
  ly=lgefint(y);
  if (ly==3)
  {
    const ulong t = y[2];
    if (x == t) return gen_0;
    z=cgeti(3);
    if (x < t) {
      z[1] = evalsigne(-1) | evallgefint(3);
      z[2] = t - x;
    }
    else {
      z[1] = evalsigne(1) | evallgefint(3);
      z[2] = x - t;
    }
    return z;
  }
  z = subiuspec(y+2,x, ly-2);
  setsigne(z,-1); return z;
}

/* return gen_0 when the sign is 0 */
GEN
addii_sign(GEN x, long sx, GEN y, long sy)
{
  long lx,ly;
  GEN z;

  if (!sx) return sy? icopy_sign(y, sy): gen_0;
  if (!sy) return icopy_sign(x, sx);
  lx = lgefint(x);
  ly = lgefint(y);
  if (sx==sy)
    z = addiispec(x+2,y+2,lx-2,ly-2);
  else
  { /* sx != sy */
    long i = cmpiispec(x+2,y+2,lx-2,ly-2);
    if (!i) return gen_0;
    /* we must ensure |x| > |y| for subiispec */
    if (i < 0) {
      sx = sy;
      z = subiispec(y+2,x+2,ly-2,lx-2);
    }
    else
      z = subiispec(x+2,y+2,lx-2,ly-2);
  }
  setsigne(z,sx); return z;
}

INLINE GEN
rcopy_sign(GEN x, long sx) { GEN y = rcopy(x); setsigne(y,sx); return y; }

GEN
addir_sign(GEN x, long sx, GEN y, long sy)
{
  long e, l, ly;
  GEN z;

  if (!sx) return rcopy_sign(y, sy);
  e = expo(y) - expi(x);
  if (!sy)
  {
    if (e >= 0) return rcopy_sign(y, sy);
    z = itor(x, nbits2prec(-e));
    setsigne(z, sx); return z;
  }

  ly = lg(y);
  if (e > 0)
  {
    l = ly - divsBIL(e);
    if (l < 3) return rcopy_sign(y, sy);
  }
  else l = ly + nbits2extraprec(-e);
  z = (GEN)avma;
  y = addrr_sign(itor(x,l), sx, y, sy);
  ly = lg(y); while (ly--) *--z = y[ly];
  set_avma((pari_sp)z); return z;
}

static GEN
addsr_sign(long x, GEN y, long sy)
{
  long e, l, ly, sx;
  GEN z;

  if (!x) return rcopy_sign(y, sy);
  if (x < 0) { sx = -1; x = -x; } else sx = 1;
  e = expo(y) - expu(x);
  if (!sy)
  {
    if (e >= 0) return rcopy_sign(y, sy);
    if (sx == -1) x = -x;
    return stor(x, nbits2prec(-e));
  }

  ly = lg(y);
  if (e > 0)
  {
    l = ly - divsBIL(e);
    if (l < 3) return rcopy_sign(y, sy);
  }
  else l = ly + nbits2extraprec(-e);
  z = (GEN)avma;
  y = addrr_sign(stor(x,l), sx, y, sy);
  ly = lg(y); while (ly--) *--z = y[ly];
  set_avma((pari_sp)z); return z;
}

GEN
addsr(long x, GEN y) { return addsr_sign(x, y, signe(y)); }

GEN
subsr(long x, GEN y) { return addsr_sign(x, y, -signe(y)); }

GEN
addrr_sign(GEN x, long sx, GEN y, long sy)
{
  long lx, ex = expo(x);
  long ly, ey = expo(y), e = ey - ex;
  long i, j, lz, ez, m;
  int extend, f2;
  GEN z;
  LOCAL_OVERFLOW;

  if (!sy)
  {
    if (!sx)
    {
      if (e > 0) ex = ey;
      return real_0_bit(ex);
    }
    if (e >= 0) return real_0_bit(ey);
    lz = nbits2prec(-e);
    lx = lg(x); if (lz > lx) lz = lx;
    z = cgetr(lz); while(--lz) z[lz] = x[lz];
    setsigne(z,sx); return z;
  }
  if (!sx)
  {
    if (e <= 0) return real_0_bit(ex);
    lz = nbits2prec(e);
    ly = lg(y); if (lz > ly) lz = ly;
    z = cgetr(lz); while (--lz) z[lz] = y[lz];
    setsigne(z,sy); return z;
  }

  if (e < 0) { swap(x,y); lswap(sx,sy); ey=ex; e=-e; }
  /* now ey >= ex */
  lx = lg(x);
  ly = lg(y);
  /* If exponents differ, need to shift one argument, here x. If
   * extend = 1: extension of x,z by m < BIL bits (round to 1 word) */
  /* in this case, lz = lx + d + 1, otherwise lx + d */
  extend = 0;
  if (e)
  {
    long d = dvmdsBIL(e, &m), l = ly-d;
    if (l <= 2) return rcopy_sign(y, sy);
    if (l > lx) { lz = lx + d + 1; extend = 1; }
    else        { lz = ly; lx = l; }
    if (m)
    { /* shift x right m bits */
      const pari_sp av = avma;
      const ulong sh = BITS_IN_LONG-m;
      GEN p1 = x; x = new_chunk(lx + lz + 1);
      shift_right(x,p1,2,lx, 0,m);
      if (extend) uel(x,lx) = uel(p1,lx-1) << sh;
      set_avma(av); /* HACK: cgetr(lz) will not overwrite x */
    }
  }
  else
  { /* d = 0 */
    m = 0;
    if (lx > ly) lx = ly;
    lz = lx;
  }

  if (sx == sy)
  { /* addition */
    i = lz-1;
    j = lx-1;
    if (extend) {
      ulong garde = addll(x[lx], y[i]);
      if (m < 4) /* don't extend for few correct bits */
        z = cgetr(--lz);
      else
      {
        z = cgetr(lz);
        z[i] = garde;
      }
    }
    else
    {
      z = cgetr(lz);
      z[i] = addll(x[j], y[i]); j--;
    }
    i--;
    for (; j>=2; i--,j--) z[i] = addllx(x[j],y[i]);
    if (overflow)
    {
      z[1] = 1; /* stops since z[1] != 0 */
      for (;;) { z[i] = uel(y,i)+1; if (z[i--]) break; }
      if (i <= 0)
      {
        shift_right(z,z, 2,lz, 1,1);
        z[1] = evalsigne(sx) | evalexpo(ey+1); return z;
      }
    }
    for (; i>=2; i--) z[i] = y[i];
    z[1] = evalsigne(sx) | evalexpo(ey); return z;
  }

  /* subtraction */
  if (e) f2 = 1;
  else
  {
    i = 2; while (i < lx && x[i] == y[i]) i++;
    if (i==lx) return real_0_bit(ey+1 - prec2nbits(lx));
    f2 = (uel(y,i) > uel(x,i));
  }
  /* result is nonzero. f2 = (y > x) */
  i = lz-1; z = cgetr(lz);
  if (f2)
  {
    j = lx-1;
    if (extend) z[i] = subll(y[i], x[lx]);
    else        z[i] = subll(y[i], x[j--]);
    for (i--; j>=2; i--) z[i] = subllx(y[i], x[j--]);
    if (overflow) /* stops since y[1] != 0 */
      for (;;) { z[i] = uel(y,i)-1; if (y[i--]) break; }
    for (; i>=2; i--) z[i] = y[i];
    sx = sy;
  }
  else
  {
    if (extend) z[i] = subll(x[lx], y[i]);
    else        z[i] = subll(x[i],  y[i]);
    for (i--; i>=2; i--) z[i] = subllx(x[i], y[i]);
  }

  x = z+2; i = 0; while (!x[i]) i++;
  lz -= i; z += i;
  j = bfffo(z[2]); /* need to shift left by j bits to normalize mantissa */
  ez = ey - (j | (i * BITS_IN_LONG));
  if (extend)
  { /* z was extended by d+1 words [should be e bits = d words + m bits] */
    /* not worth keeping extra word if less than 5 significant bits in there */
    if (m - j < 5 && lz > 3)
    { /* shorten z */
      ulong last = (ulong)z[--lz]; /* cancelled word */

      /* if we need to shift anyway, shorten from left
       * If not, shorten from right, neutralizing last word of z */
      if (j == 0)
        /* stackdummy((pari_sp)(z + lz+1), (pari_sp)(z + lz)); */
        z[lz] = evaltyp(t_VECSMALL) | _evallg(1);
      else
      {
        GEN t = z;
        z++; shift_left(z,t,2,lz-1, last,j);
      }
      if ((last<<j) & HIGHBIT)
      { /* round up */
        i = lz-1;
        while (++((ulong*)z)[i] == 0 && i > 1) i--;
        if (i == 1) { ez++; z[2] = (long)HIGHBIT; }
      }
    }
    else if (j) shift_left(z,z,2,lz-1, 0,j);
  }
  else if (j) shift_left(z,z,2,lz-1, 0,j);
  z[1] = evalsigne(sx) | evalexpo(ez);
  z[0] = evaltyp(t_REAL) | evallg(lz);
  set_avma((pari_sp)z); return z;
}
