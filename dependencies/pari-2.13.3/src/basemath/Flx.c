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

#include "pari.h"
#include "paripriv.h"

/* Not so fast arithmetic with polynomials with small coefficients. */

static GEN
get_Flx_red(GEN T, GEN *B)
{
  if (typ(T)!=t_VEC) { *B=NULL; return T; }
  *B = gel(T,1); return gel(T,2);
}

/***********************************************************************/
/**                                                                   **/
/**               Flx                                                 **/
/**                                                                   **/
/***********************************************************************/
/* Flx objects are defined as follows:
   Let l an ulong. An Flx is a t_VECSMALL:
   x[0] = codeword
   x[1] = evalvarn(variable number)  (signe is not stored).
   x[2] = a_0 x[3] = a_1, etc.
   With 0 <= a_i < l

   signe(x) is not valid. Use degpol(x)>=0 instead.
*/
/***********************************************************************/
/**                                                                   **/
/**          Conversion from Flx                                      **/
/**                                                                   **/
/***********************************************************************/

GEN
Flx_to_ZX(GEN z)
{
  long i, l = lg(z);
  GEN x = cgetg(l,t_POL);
  for (i=2; i<l; i++) gel(x,i) = utoi(z[i]);
  x[1] = evalsigne(l-2!=0)| z[1]; return x;
}

GEN
Flx_to_FlxX(GEN z, long sv)
{
  long i, l = lg(z);
  GEN x = cgetg(l,t_POL);
  for (i=2; i<l; i++) gel(x,i) = Fl_to_Flx(z[i], sv);
  x[1] = evalsigne(l-2!=0)| z[1]; return x;
}

/* same as Flx_to_ZX, in place */
GEN
Flx_to_ZX_inplace(GEN z)
{
  long i, l = lg(z);
  for (i=2; i<l; i++) gel(z,i) = utoi(z[i]);
  settyp(z, t_POL); z[1]=evalsigne(l-2!=0)|z[1]; return z;
}

/*Flx_to_Flv=zx_to_zv*/
GEN
Flx_to_Flv(GEN x, long N)
{
  long i, l;
  GEN z = cgetg(N+1,t_VECSMALL);
  if (typ(x) != t_VECSMALL) pari_err_TYPE("Flx_to_Flv",x);
  l = lg(x)-1; x++;
  for (i=1; i<l ; i++) z[i]=x[i];
  for (   ; i<=N; i++) z[i]=0;
  return z;
}

/*Flv_to_Flx=zv_to_zx*/
GEN
Flv_to_Flx(GEN x, long sv)
{
  long i, l=lg(x)+1;
  GEN z = cgetg(l,t_VECSMALL); z[1]=sv;
  x--;
  for (i=2; i<l ; i++) z[i]=x[i];
  return Flx_renormalize(z,l);
}

/*Flm_to_FlxV=zm_to_zxV*/
GEN
Flm_to_FlxV(GEN x, long sv)
{ pari_APPLY_type(t_VEC, Flv_to_Flx(gel(x,i), sv)) }

/*FlxC_to_ZXC=zxC_to_ZXC*/
GEN
FlxC_to_ZXC(GEN x)
{ pari_APPLY_type(t_COL, Flx_to_ZX(gel(x,i))) }

/*FlxC_to_ZXC=zxV_to_ZXV*/
GEN
FlxV_to_ZXV(GEN x)
{ pari_APPLY_type(t_VEC, Flx_to_ZX(gel(x,i))) }

void
FlxV_to_ZXV_inplace(GEN v)
{
  long i;
  for(i=1;i<lg(v);i++) gel(v,i)= Flx_to_ZX(gel(v,i));
}

/*FlxM_to_ZXM=zxM_to_ZXM*/
GEN
FlxM_to_ZXM(GEN x)
{ pari_APPLY_same(FlxC_to_ZXC(gel(x,i))) }

GEN
FlxV_to_FlxX(GEN x, long v)
{
  long i, l = lg(x)+1;
  GEN z = cgetg(l,t_POL); z[1] = evalvarn(v);
  x--;
  for (i=2; i<l ; i++) gel(z,i) = gel(x,i);
  return FlxX_renormalize(z,l);
}

GEN
FlxM_Flx_add_shallow(GEN x, GEN y, ulong p)
{
  long l = lg(x), i, j;
  GEN z = cgetg(l,t_MAT);

  if (l==1) return z;
  if (l != lgcols(x)) pari_err_OP( "+", x, y);
  for (i=1; i<l; i++)
  {
    GEN zi = cgetg(l,t_COL), xi = gel(x,i);
    gel(z,i) = zi;
    for (j=1; j<l; j++) gel(zi,j) = gel(xi,j);
    gel(zi,i) = Flx_add(gel(zi,i), y, p);
  }
  return z;
}

/***********************************************************************/
/**                                                                   **/
/**          Conversion to Flx                                        **/
/**                                                                   **/
/***********************************************************************/
/* Take an integer and return a scalar polynomial mod p,  with evalvarn=vs */
GEN
Fl_to_Flx(ulong x, long sv)
{
  return x? mkvecsmall2(sv, x): pol0_Flx(sv);
}

/* a X^d */
GEN
monomial_Flx(ulong a, long d, long vs)
{
  GEN P;
  if (a==0) return pol0_Flx(vs);
  P = const_vecsmall(d+2, 0);
  P[1] = vs; P[d+2] = a;
  return P;
}

GEN
Z_to_Flx(GEN x, ulong p, long sv)
{
  long u = umodiu(x,p);
  return u? mkvecsmall2(sv, u): pol0_Flx(sv);
}

/* return x[0 .. dx] mod p as t_VECSMALL. Assume x a t_POL*/
GEN
ZX_to_Flx(GEN x, ulong p)
{
  long i, lx = lg(x);
  GEN a = cgetg(lx, t_VECSMALL);
  a[1]=((ulong)x[1])&VARNBITS;
  for (i=2; i<lx; i++) a[i] = umodiu(gel(x,i), p);
  return Flx_renormalize(a,lx);
}

/* return x[0 .. dx] mod p as t_VECSMALL. Assume x a t_POL*/
GEN
zx_to_Flx(GEN x, ulong p)
{
  long i, lx = lg(x);
  GEN a = cgetg(lx, t_VECSMALL);
  a[1] = x[1];
  for (i=2; i<lx; i++) uel(a,i) = umodsu(x[i], p);
  return Flx_renormalize(a,lx);
}

ulong
Rg_to_Fl(GEN x, ulong p)
{
  switch(typ(x))
  {
    case t_INT: return umodiu(x, p);
    case t_FRAC: {
      ulong z = umodiu(gel(x,1), p);
      if (!z) return 0;
      return Fl_div(z, umodiu(gel(x,2), p), p);
    }
    case t_PADIC: return padic_to_Fl(x, p);
    case t_INTMOD: {
      GEN q = gel(x,1), a = gel(x,2);
      if (absequaliu(q, p)) return itou(a);
      if (!dvdiu(q,p)) pari_err_MODULUS("Rg_to_Fl", q, utoi(p));
      return umodiu(a, p);
    }
    default: pari_err_TYPE("Rg_to_Fl",x);
      return 0; /* LCOV_EXCL_LINE */
  }
}

ulong
Rg_to_F2(GEN x)
{
  switch(typ(x))
  {
    case t_INT: return mpodd(x);
    case t_FRAC:
      if (!mpodd(gel(x,2))) (void)Fl_inv(0,2); /* error */
      return mpodd(gel(x,1));
    case t_PADIC:
      if (!absequaliu(gel(x,2),2)) pari_err_OP("",x, mkintmodu(1,2));
      if (valp(x) < 0) (void)Fl_inv(0,2);
      return valp(x) & 1;
    case t_INTMOD: {
      GEN q = gel(x,1), a = gel(x,2);
      if (mpodd(q)) pari_err_MODULUS("Rg_to_F2", q, gen_2);
      return mpodd(a);
    }
    default: pari_err_TYPE("Rg_to_F2",x);
      return 0; /* LCOV_EXCL_LINE */
  }
}

GEN
RgX_to_Flx(GEN x, ulong p)
{
  long i, lx = lg(x);
  GEN a = cgetg(lx, t_VECSMALL);
  a[1]=((ulong)x[1])&VARNBITS;
  for (i=2; i<lx; i++) a[i] = Rg_to_Fl(gel(x,i), p);
  return Flx_renormalize(a,lx);
}

GEN
RgXV_to_FlxV(GEN x, ulong p)
{ pari_APPLY_type(t_VEC, RgX_to_Flx(gel(x,i), p)) }

/* If x is a POLMOD, assume modulus is a multiple of T. */
GEN
Rg_to_Flxq(GEN x, GEN T, ulong p)
{
  long ta, tx = typ(x), v = get_Flx_var(T);
  GEN a, b;
  if (is_const_t(tx))
  {
    if (tx == t_FFELT) return FF_to_Flxq(x);
    return Fl_to_Flx(Rg_to_Fl(x, p), v);
  }
  switch(tx)
  {
    case t_POLMOD:
      b = gel(x,1);
      a = gel(x,2); ta = typ(a);
      if (is_const_t(ta)) return Fl_to_Flx(Rg_to_Fl(a, p), v);
      b = RgX_to_Flx(b, p); if (b[1] != v) break;
      a = RgX_to_Flx(a, p); if (Flx_equal(b,T)) return a;
      if (lgpol(Flx_rem(b,T,p))==0) return Flx_rem(a, T, p);
      break;
    case t_POL:
      x = RgX_to_Flx(x,p);
      if (x[1] != v) break;
      return Flx_rem(x, T, p);
    case t_RFRAC:
      a = Rg_to_Flxq(gel(x,1), T,p);
      b = Rg_to_Flxq(gel(x,2), T,p);
      return Flxq_div(a,b, T,p);
  }
  pari_err_TYPE("Rg_to_Flxq",x);
  return NULL; /* LCOV_EXCL_LINE */
}

/***********************************************************************/
/**                                                                   **/
/**          Basic operation on Flx                                   **/
/**                                                                   **/
/***********************************************************************/
/* = zx_renormalize. Similar to normalizepol, in place */
GEN
Flx_renormalize(GEN /*in place*/ x, long lx)
{
  long i;
  for (i = lx-1; i>1; i--)
    if (x[i]) break;
  stackdummy((pari_sp)(x + lg(x)), (pari_sp)(x + i+1));
  setlg(x, i+1); return x;
}

GEN
Flx_red(GEN z, ulong p)
{
  long i, l = lg(z);
  GEN x = cgetg(l, t_VECSMALL);
  x[1] = z[1];
  for (i=2; i<l; i++) x[i] = uel(z,i)%p;
  return Flx_renormalize(x,l);
}

int
Flx_equal(GEN V, GEN W)
{
  long l = lg(V);
  if (lg(W) != l) return 0;
  while (--l > 1) /* do not compare variables, V[1] */
    if (V[l] != W[l]) return 0;
  return 1;
}

GEN
random_Flx(long d1, long vs, ulong p)
{
  long i, d = d1+2;
  GEN y = cgetg(d,t_VECSMALL); y[1] = vs;
  for (i=2; i<d; i++) y[i] = random_Fl(p);
  return Flx_renormalize(y,d);
}

static GEN
Flx_addspec(GEN x, GEN y, ulong p, long lx, long ly)
{
  long i,lz;
  GEN z;

  if (ly>lx) swapspec(x,y, lx,ly);
  lz = lx+2; z = cgetg(lz, t_VECSMALL);
  for (i=0; i<ly; i++) z[i+2] = Fl_add(x[i], y[i], p);
  for (   ; i<lx; i++) z[i+2] = x[i];
  z[1] = 0; return Flx_renormalize(z, lz);
}

GEN
Flx_add(GEN x, GEN y, ulong p)
{
  long i,lz;
  GEN z;
  long lx=lg(x);
  long ly=lg(y);
  if (ly>lx) swapspec(x,y, lx,ly);
  lz = lx; z = cgetg(lz, t_VECSMALL); z[1]=x[1];
  for (i=2; i<ly; i++) z[i] = Fl_add(x[i], y[i], p);
  for (   ; i<lx; i++) z[i] = x[i];
  return Flx_renormalize(z, lz);
}

GEN
Flx_Fl_add(GEN y, ulong x, ulong p)
{
  GEN z;
  long lz, i;
  if (!lgpol(y))
    return Fl_to_Flx(x,y[1]);
  lz=lg(y);
  z=cgetg(lz,t_VECSMALL);
  z[1]=y[1];
  z[2] = Fl_add(y[2],x,p);
  for(i=3;i<lz;i++)
    z[i] = y[i];
  if (lz==3) z = Flx_renormalize(z,lz);
  return z;
}

static GEN
Flx_subspec(GEN x, GEN y, ulong p, long lx, long ly)
{
  long i,lz;
  GEN z;

  if (ly <= lx)
  {
    lz = lx+2; z = cgetg(lz, t_VECSMALL);
    for (i=0; i<ly; i++) z[i+2] = Fl_sub(x[i],y[i],p);
    for (   ; i<lx; i++) z[i+2] = x[i];
  }
  else
  {
    lz = ly+2; z = cgetg(lz, t_VECSMALL);
    for (i=0; i<lx; i++) z[i+2] = Fl_sub(x[i],y[i],p);
    for (   ; i<ly; i++) z[i+2] = Fl_neg(y[i],p);
  }
  z[1] = 0; return Flx_renormalize(z, lz);
}

GEN
Flx_sub(GEN x, GEN y, ulong p)
{
  long i,lz,lx = lg(x), ly = lg(y);
  GEN z;

  if (ly <= lx)
  {
    lz = lx; z = cgetg(lz, t_VECSMALL);
    for (i=2; i<ly; i++) z[i] = Fl_sub(x[i],y[i],p);
    for (   ; i<lx; i++) z[i] = x[i];
  }
  else
  {
    lz = ly; z = cgetg(lz, t_VECSMALL);
    for (i=2; i<lx; i++) z[i] = Fl_sub(x[i],y[i],p);
    for (   ; i<ly; i++) z[i] = y[i]? (long)(p - y[i]): y[i];
  }
  z[1]=x[1]; return Flx_renormalize(z, lz);
}

GEN
Flx_Fl_sub(GEN y, ulong x, ulong p)
{
  GEN z;
  long lz = lg(y), i;
  if (lz==2)
    return Fl_to_Flx(Fl_neg(x, p),y[1]);
  z = cgetg(lz, t_VECSMALL);
  z[1] = y[1];
  z[2] = Fl_sub(uel(y,2), x, p);
  for(i=3; i<lz; i++)
    z[i] = y[i];
  if (lz==3) z = Flx_renormalize(z,lz);
  return z;
}

static GEN
Flx_negspec(GEN x, ulong p, long l)
{
  long i;
  GEN z = cgetg(l+2, t_VECSMALL) + 2;
  for (i=0; i<l; i++) z[i] = Fl_neg(x[i], p);
  return z-2;
}

GEN
Flx_neg(GEN x, ulong p)
{
  GEN z = Flx_negspec(x+2, p, lgpol(x));
  z[1] = x[1];
  return z;
}

GEN
Flx_neg_inplace(GEN x, ulong p)
{
  long i, l = lg(x);
  for (i=2; i<l; i++)
    if (x[i]) x[i] = p - x[i];
  return x;
}

GEN
Flx_double(GEN y, ulong p)
{
  long i, l;
  GEN z = cgetg_copy(y, &l); z[1] = y[1];
  for(i=2; i<l; i++) z[i] = Fl_double(y[i], p);
  return Flx_renormalize(z, l);
}
GEN
Flx_triple(GEN y, ulong p)
{
  long i, l;
  GEN z = cgetg_copy(y, &l); z[1] = y[1];
  for(i=2; i<l; i++) z[i] = Fl_triple(y[i], p);
  return Flx_renormalize(z, l);
}
GEN
Flx_Fl_mul(GEN y, ulong x, ulong p)
{
  GEN z;
  long i, l;
  if (!x) return pol0_Flx(y[1]);
  z = cgetg_copy(y, &l); z[1] = y[1];
  if (HIGHWORD(x | p))
    for(i=2; i<l; i++) z[i] = Fl_mul(y[i], x, p);
  else
    for(i=2; i<l; i++) z[i] = (y[i] * x) % p;
  return Flx_renormalize(z, l);
}
GEN
Flx_Fl_mul_to_monic(GEN y, ulong x, ulong p)
{
  GEN z;
  long i, l;
  z = cgetg_copy(y, &l); z[1] = y[1];
  if (HIGHWORD(x | p))
    for(i=2; i<l-1; i++) z[i] = Fl_mul(y[i], x, p);
  else
    for(i=2; i<l-1; i++) z[i] = (y[i] * x) % p;
  z[l-1] = 1; return z;
}

/* Return a*x^n if n>=0 and a\x^(-n) if n<0 */
GEN
Flx_shift(GEN a, long n)
{
  long i, l = lg(a);
  GEN  b;
  if (l==2 || !n) return Flx_copy(a);
  if (l+n<=2) return pol0_Flx(a[1]);
  b = cgetg(l+n, t_VECSMALL);
  b[1] = a[1];
  if (n < 0)
    for (i=2-n; i<l; i++) b[i+n] = a[i];
  else
  {
    for (i=0; i<n; i++) b[2+i] = 0;
    for (i=2; i<l; i++) b[i+n] = a[i];
  }
  return b;
}

GEN
Flx_normalize(GEN z, ulong p)
{
  long l = lg(z)-1;
  ulong p1 = z[l]; /* leading term */
  if (p1 == 1) return z;
  return Flx_Fl_mul_to_monic(z, Fl_inv(p1,p), p);
}

/* return (x * X^d) + y. Assume d > 0, shallow if x == 0*/
static GEN
Flx_addshift(GEN x, GEN y, ulong p, long d)
{
  GEN xd,yd,zd = (GEN)avma;
  long a,lz,ny = lgpol(y), nx = lgpol(x);
  long vs = x[1];
  if (nx == 0) return y;
  x += 2; y += 2; a = ny-d;
  if (a <= 0)
  {
    lz = (a>nx)? ny+2: nx+d+2;
    (void)new_chunk(lz); xd = x+nx; yd = y+ny;
    while (xd > x) *--zd = *--xd;
    x = zd + a;
    while (zd > x) *--zd = 0;
  }
  else
  {
    xd = new_chunk(d); yd = y+d;
    x = Flx_addspec(x,yd,p, nx,a);
    lz = (a>nx)? ny+2: lg(x)+d;
    x += 2; while (xd > x) *--zd = *--xd;
  }
  while (yd > y) *--zd = *--yd;
  *--zd = vs;
  *--zd = evaltyp(t_VECSMALL) | evallg(lz); return zd;
}

/* shift polynomial + gerepile */
/* Do not set evalvarn*/
static GEN
Flx_shiftip(pari_sp av, GEN x, long v)
{
  long i, lx = lg(x), ly;
  GEN y;
  if (!v || lx==2) return gerepileuptoleaf(av, x);
  ly = lx + v; /* result length */
  (void)new_chunk(ly); /* check that result fits */
  x += lx; y = (GEN)av;
  for (i = 2; i<lx; i++) *--y = *--x;
  for (i = 0; i< v; i++) *--y = 0;
  y -= 2; y[0] = evaltyp(t_VECSMALL) | evallg(ly);
  return gc_const((pari_sp)y, y);
}

static long
get_Fl_threshold(ulong p, long mul, long mul2)
{
  return SMALL_ULONG(p) ? mul: mul2;
}

#define BITS_IN_QUARTULONG (BITS_IN_HALFULONG >> 1)
#define QUARTMASK ((1UL<<BITS_IN_QUARTULONG)-1UL)
#define LLQUARTWORD(x) ((x) & QUARTMASK)
#define HLQUARTWORD(x) (((x) >> BITS_IN_QUARTULONG) & QUARTMASK)
#define LHQUARTWORD(x) (((x) >> (2*BITS_IN_QUARTULONG)) & QUARTMASK)
#define HHQUARTWORD(x) (((x) >> (3*BITS_IN_QUARTULONG)) & QUARTMASK)
INLINE long
maxbitcoeffpol(ulong p, long n)
{
  GEN z = muliu(sqru(p - 1), n);
  long b = expi(z) + 1;
  /* only do expensive bit-packing if it saves at least 1 limb */
  if (b <= BITS_IN_QUARTULONG)
  {
    if (nbits2nlong(n*b) == (n + 3)>>2)
      b = BITS_IN_QUARTULONG;
  }
  else if (b <= BITS_IN_HALFULONG)
  {
    if (nbits2nlong(n*b) == (n + 1)>>1)
      b = BITS_IN_HALFULONG;
  }
  else
  {
    long l = lgefint(z) - 2;
    if (nbits2nlong(n*b) == n*l)
      b = l*BITS_IN_LONG;
  }
  return b;
}

INLINE ulong
Flx_mullimb_ok(GEN x, GEN y, ulong p, long a, long b)
{ /* Assume OK_ULONG*/
  ulong p1 = 0;
  long i;
  for (i=a; i<b; i++)
    if (y[i])
    {
      p1 += y[i] * x[-i];
      if (p1 & HIGHBIT) p1 %= p;
    }
  return p1 % p;
}

INLINE ulong
Flx_mullimb(GEN x, GEN y, ulong p, ulong pi, long a, long b)
{
  ulong p1 = 0;
  long i;
  for (i=a; i<b; i++)
    if (y[i])
      p1 = Fl_addmul_pre(p1, y[i], x[-i], p, pi);
  return p1;
}

/* assume nx >= ny > 0 */
static GEN
Flx_mulspec_basecase(GEN x, GEN y, ulong p, long nx, long ny)
{
  long i,lz,nz;
  GEN z;

  lz = nx+ny+1; nz = lz-2;
  z = cgetg(lz, t_VECSMALL) + 2; /* x:y:z [i] = term of degree i */
  if (SMALL_ULONG(p))
  {
    for (i=0; i<ny; i++)z[i] = Flx_mullimb_ok(x+i,y,p,0,i+1);
    for (  ; i<nx; i++) z[i] = Flx_mullimb_ok(x+i,y,p,0,ny);
    for (  ; i<nz; i++) z[i] = Flx_mullimb_ok(x+i,y,p,i-nx+1,ny);
  }
  else
  {
    ulong pi = get_Fl_red(p);
    for (i=0; i<ny; i++)z[i] = Flx_mullimb(x+i,y,p,pi,0,i+1);
    for (  ; i<nx; i++) z[i] = Flx_mullimb(x+i,y,p,pi,0,ny);
    for (  ; i<nz; i++) z[i] = Flx_mullimb(x+i,y,p,pi,i-nx+1,ny);
  }
  z -= 2; return Flx_renormalize(z, lz);
}

static GEN
int_to_Flx(GEN z, ulong p)
{
  long i, l = lgefint(z);
  GEN x = cgetg(l, t_VECSMALL);
  for (i=2; i<l; i++) x[i] = uel(z,i)%p;
  return Flx_renormalize(x, l);
}

INLINE GEN
Flx_mulspec_mulii(GEN a, GEN b, ulong p, long na, long nb)
{
  GEN z=muliispec(a,b,na,nb);
  return int_to_Flx(z,p);
}

static GEN
Flx_to_int_halfspec(GEN a, long na)
{
  long j;
  long n = (na+1)>>1UL;
  GEN V = cgetipos(2+n);
  GEN w;
  for (w = int_LSW(V), j=0; j+1<na; j+=2, w=int_nextW(w))
    *w = a[j]|(a[j+1]<<BITS_IN_HALFULONG);
  if (j<na)
    *w = a[j];
  return V;
}

static GEN
int_to_Flx_half(GEN z, ulong p)
{
  long i;
  long lx = (lgefint(z)-2)*2+2;
  GEN w, x = cgetg(lx, t_VECSMALL);
  for (w = int_LSW(z), i=2; i<lx; i+=2, w=int_nextW(w))
  {
    x[i]   = LOWWORD((ulong)*w)%p;
    x[i+1] = HIGHWORD((ulong)*w)%p;
  }
  return Flx_renormalize(x, lx);
}

static GEN
Flx_mulspec_halfmulii(GEN a, GEN b, ulong p, long na, long nb)
{
  GEN A = Flx_to_int_halfspec(a,na);
  GEN B = Flx_to_int_halfspec(b,nb);
  GEN z = mulii(A,B);
  return int_to_Flx_half(z,p);
}

static GEN
Flx_to_int_quartspec(GEN a, long na)
{
  long j;
  long n = (na+3)>>2UL;
  GEN V = cgetipos(2+n);
  GEN w;
  for (w = int_LSW(V), j=0; j+3<na; j+=4, w=int_nextW(w))
    *w = a[j]|(a[j+1]<<BITS_IN_QUARTULONG)|(a[j+2]<<(2*BITS_IN_QUARTULONG))|(a[j+3]<<(3*BITS_IN_QUARTULONG));
  switch (na-j)
  {
  case 3:
    *w = a[j]|(a[j+1]<<BITS_IN_QUARTULONG)|(a[j+2]<<(2*BITS_IN_QUARTULONG));
    break;
  case 2:
    *w = a[j]|(a[j+1]<<BITS_IN_QUARTULONG);
    break;
  case 1:
    *w = a[j];
    break;
  case 0:
    break;
  }
  return V;
}

static GEN
int_to_Flx_quart(GEN z, ulong p)
{
  long i;
  long lx = (lgefint(z)-2)*4+2;
  GEN w, x = cgetg(lx, t_VECSMALL);
  for (w = int_LSW(z), i=2; i<lx; i+=4, w=int_nextW(w))
  {
    x[i]   = LLQUARTWORD((ulong)*w)%p;
    x[i+1] = HLQUARTWORD((ulong)*w)%p;
    x[i+2] = LHQUARTWORD((ulong)*w)%p;
    x[i+3] = HHQUARTWORD((ulong)*w)%p;
  }
  return Flx_renormalize(x, lx);
}

static GEN
Flx_mulspec_quartmulii(GEN a, GEN b, ulong p, long na, long nb)
{
  GEN A = Flx_to_int_quartspec(a,na);
  GEN B = Flx_to_int_quartspec(b,nb);
  GEN z = mulii(A,B);
  return int_to_Flx_quart(z,p);
}

/*Eval x in 2^(k*BIL) in linear time, k==2 or 3*/
static GEN
Flx_eval2BILspec(GEN x, long k, long l)
{
  long i, lz = k*l, ki;
  GEN pz = cgetipos(2+lz);
  for (i=0; i < lz; i++)
    *int_W(pz,i) = 0UL;
  for (i=0, ki=0; i<l; i++, ki+=k)
    *int_W(pz,ki) = x[i];
  return int_normalize(pz,0);
}

static GEN
Z_mod2BIL_Flx_2(GEN x, long d, ulong p)
{
  long i, offset, lm = lgefint(x)-2, l = d+3;
  ulong pi = get_Fl_red(p);
  GEN pol = cgetg(l, t_VECSMALL);
  pol[1] = 0;
  for (i=0, offset=0; offset+1 < lm; i++, offset += 2)
    pol[i+2] = remll_pre(*int_W(x,offset+1), *int_W(x,offset), p, pi);
  if (offset < lm)
    pol[i+2] = (*int_W(x,offset)) % p;
  return Flx_renormalize(pol,l);
}

static GEN
Z_mod2BIL_Flx_3(GEN x, long d, ulong p)
{
  long i, offset, lm = lgefint(x)-2, l = d+3;
  ulong pi = get_Fl_red(p);
  GEN pol = cgetg(l, t_VECSMALL);
  pol[1] = 0;
  for (i=0, offset=0; offset+2 < lm; i++, offset += 3)
    pol[i+2] = remlll_pre(*int_W(x,offset+2), *int_W(x,offset+1),
                          *int_W(x,offset), p, pi);
  if (offset+1 < lm)
    pol[i+2] = remll_pre(*int_W(x,offset+1), *int_W(x,offset), p, pi);
  else if (offset < lm)
    pol[i+2] = (*int_W(x,offset)) % p;
  return Flx_renormalize(pol,l);
}

static GEN
Z_mod2BIL_Flx(GEN x, long bs, long d, ulong p)
{
  return bs==2 ? Z_mod2BIL_Flx_2(x, d, p): Z_mod2BIL_Flx_3(x, d, p);
}

static GEN
Flx_mulspec_mulii_inflate(GEN x, GEN y, long N, ulong p, long nx, long ny)
{
  pari_sp av = avma;
  GEN z = mulii(Flx_eval2BILspec(x,N,nx), Flx_eval2BILspec(y,N,ny));
  return gerepileupto(av, Z_mod2BIL_Flx(z, N, nx+ny-2, p));
}

static GEN
kron_pack_Flx_spec_bits(GEN x, long b, long l) {
  GEN y;
  long i;
  if (l == 0)
    return gen_0;
  y = cgetg(l + 1, t_VECSMALL);
  for(i = 1; i <= l; i++)
    y[i] = x[l - i];
  return nv_fromdigits_2k(y, b);
}

/* assume b < BITS_IN_LONG */
static GEN
kron_unpack_Flx_bits_narrow(GEN z, long b, ulong p) {
  GEN v = binary_2k_nv(z, b), x;
  long i, l = lg(v) + 1;
  x = cgetg(l, t_VECSMALL);
  for (i = 2; i < l; i++)
    x[i] = v[l - i] % p;
  return Flx_renormalize(x, l);
}

static GEN
kron_unpack_Flx_bits_wide(GEN z, long b, ulong p, ulong pi) {
  GEN v = binary_2k(z, b), x, y;
  long i, l = lg(v) + 1, ly;
  x = cgetg(l, t_VECSMALL);
  for (i = 2; i < l; i++) {
    y = gel(v, l - i);
    ly = lgefint(y);
    switch (ly) {
    case 2: x[i] = 0; break;
    case 3: x[i] = *int_W_lg(y, 0, ly) % p; break;
    case 4: x[i] = remll_pre(*int_W_lg(y, 1, ly), *int_W_lg(y, 0, ly), p, pi); break;
    case 5: x[i] = remlll_pre(*int_W_lg(y, 2, ly), *int_W_lg(y, 1, ly),
                              *int_W_lg(y, 0, ly), p, pi); break;
    default: x[i] = umodiu(gel(v, l - i), p);
    }
  }
  return Flx_renormalize(x, l);
}

static GEN
Flx_mulspec_Kronecker(GEN A, GEN B, long b, ulong p, long lA, long lB)
{
  GEN C, D;
  pari_sp av = avma;
  A =  kron_pack_Flx_spec_bits(A, b, lA);
  B =  kron_pack_Flx_spec_bits(B, b, lB);
  C = gerepileuptoint(av, mulii(A, B));
  if (b < BITS_IN_LONG)
    D =  kron_unpack_Flx_bits_narrow(C, b, p);
  else
  {
    ulong pi = get_Fl_red(p);
    D = kron_unpack_Flx_bits_wide(C, b, p, pi);
  }
  return D;
}

static GEN
Flx_sqrspec_Kronecker(GEN A, long b, ulong p, long lA)
{
  GEN C, D;
  A =  kron_pack_Flx_spec_bits(A, b, lA);
  C = sqri(A);
  if (b < BITS_IN_LONG)
    D =  kron_unpack_Flx_bits_narrow(C, b, p);
  else
  {
    ulong pi = get_Fl_red(p);
    D = kron_unpack_Flx_bits_wide(C, b, p, pi);
  }
  return D;
}

/* fast product (Karatsuba) of polynomials a,b. These are not real GENs, a+2,
 * b+2 were sent instead. na, nb = number of terms of a, b.
 * Only c, c0, c1, c2 are genuine GEN.
 */
static GEN
Flx_mulspec(GEN a, GEN b, ulong p, long na, long nb)
{
  GEN a0,c,c0;
  long n0, n0a, i, v = 0;
  pari_sp av;

  while (na && !a[0]) { a++; na--; v++; }
  while (nb && !b[0]) { b++; nb--; v++; }
  if (na < nb) swapspec(a,b, na,nb);
  if (!nb) return pol0_Flx(0);

  av = avma;
  if (nb >= get_Fl_threshold(p, Flx_MUL_MULII_LIMIT, Flx_MUL2_MULII_LIMIT))
  {
    long m = maxbitcoeffpol(p,nb);
    switch (m)
    {
    case BITS_IN_QUARTULONG:
      return Flx_shiftip(av,Flx_mulspec_quartmulii(a,b,p,na,nb), v);
    case BITS_IN_HALFULONG:
      return Flx_shiftip(av,Flx_mulspec_halfmulii(a,b,p,na,nb), v);
    case BITS_IN_LONG:
      return Flx_shiftip(av,Flx_mulspec_mulii(a,b,p,na,nb), v);
    case 2*BITS_IN_LONG:
      return Flx_shiftip(av,Flx_mulspec_mulii_inflate(a,b,2,p,na,nb), v);
    case 3*BITS_IN_LONG:
      return Flx_shiftip(av,Flx_mulspec_mulii_inflate(a,b,3,p,na,nb), v);
    default:
      return Flx_shiftip(av,Flx_mulspec_Kronecker(a,b,m,p,na,nb), v);
    }
  }
  if (nb < get_Fl_threshold(p, Flx_MUL_KARATSUBA_LIMIT, Flx_MUL2_KARATSUBA_LIMIT))
    return Flx_shiftip(av,Flx_mulspec_basecase(a,b,p,na,nb), v);
  i=(na>>1); n0=na-i; na=i;
  a0=a+n0; n0a=n0;
  while (n0a && !a[n0a-1]) n0a--;

  if (nb > n0)
  {
    GEN b0,c1,c2;
    long n0b;

    nb -= n0; b0 = b+n0; n0b = n0;
    while (n0b && !b[n0b-1]) n0b--;
    c =  Flx_mulspec(a,b,p,n0a,n0b);
    c0 = Flx_mulspec(a0,b0,p,na,nb);

    c2 = Flx_addspec(a0,a,p,na,n0a);
    c1 = Flx_addspec(b0,b,p,nb,n0b);

    c1 = Flx_mul(c1,c2,p);
    c2 = Flx_add(c0,c,p);

    c2 = Flx_neg_inplace(c2,p);
    c2 = Flx_add(c1,c2,p);
    c0 = Flx_addshift(c0,c2 ,p, n0);
  }
  else
  {
    c  = Flx_mulspec(a,b,p,n0a,nb);
    c0 = Flx_mulspec(a0,b,p,na,nb);
  }
  c0 = Flx_addshift(c0,c,p,n0);
  return Flx_shiftip(av,c0, v);
}

GEN
Flx_mul(GEN x, GEN y, ulong p)
{
  GEN z = Flx_mulspec(x+2,y+2,p, lgpol(x),lgpol(y));
  z[1] = x[1]; return z;
}

static GEN
Flx_sqrspec_basecase(GEN x, ulong p, long nx)
{
  long i, lz, nz;
  ulong p1;
  GEN z;

  if (!nx) return pol0_Flx(0);
  lz = (nx << 1) + 1, nz = lz-2;
  z = cgetg(lz, t_VECSMALL) + 2;
  if (SMALL_ULONG(p))
  {
    z[0] = x[0]*x[0]%p;
    for (i=1; i<nx; i++)
    {
      p1 = Flx_mullimb_ok(x+i,x,p,0, (i+1)>>1);
      p1 <<= 1;
      if ((i&1) == 0) p1 += x[i>>1] * x[i>>1];
      z[i] = p1 % p;
    }
    for (  ; i<nz; i++)
    {
      p1 = Flx_mullimb_ok(x+i,x,p,i-nx+1, (i+1)>>1);
      p1 <<= 1;
      if ((i&1) == 0) p1 += x[i>>1] * x[i>>1];
      z[i] = p1 % p;
    }
  }
  else
  {
    ulong pi = get_Fl_red(p);
    z[0] = Fl_sqr_pre(x[0], p, pi);
    for (i=1; i<nx; i++)
    {
      p1 = Flx_mullimb(x+i,x,p,pi,0, (i+1)>>1);
      p1 = Fl_add(p1, p1, p);
      if ((i&1) == 0) p1 = Fl_add(p1, Fl_sqr_pre(x[i>>1], p, pi), p);
      z[i] = p1;
    }
    for (  ; i<nz; i++)
    {
      p1 = Flx_mullimb(x+i,x,p,pi,i-nx+1, (i+1)>>1);
      p1 = Fl_add(p1, p1, p);
      if ((i&1) == 0) p1 = Fl_add(p1, Fl_sqr_pre(x[i>>1], p, pi), p);
      z[i] = p1;
    }
  }
  z -= 2; return Flx_renormalize(z, lz);
}

static GEN
Flx_sqrspec_sqri(GEN a, ulong p, long na)
{
  GEN z=sqrispec(a,na);
  return int_to_Flx(z,p);
}

static GEN
Flx_sqrspec_halfsqri(GEN a, ulong p, long na)
{
  GEN z = sqri(Flx_to_int_halfspec(a,na));
  return int_to_Flx_half(z,p);
}

static GEN
Flx_sqrspec_quartsqri(GEN a, ulong p, long na)
{
  GEN z = sqri(Flx_to_int_quartspec(a,na));
  return int_to_Flx_quart(z,p);
}

static GEN
Flx_sqrspec_sqri_inflate(GEN x, long N, ulong p, long nx)
{
  pari_sp av = avma;
  GEN  z = sqri(Flx_eval2BILspec(x,N,nx));
  return gerepileupto(av, Z_mod2BIL_Flx(z, N, (nx-1)*2, p));
}

static GEN
Flx_sqrspec(GEN a, ulong p, long na)
{
  GEN a0, c, c0;
  long n0, n0a, i, v = 0, m;
  pari_sp av;

  while (na && !a[0]) { a++; na--; v += 2; }
  if (!na) return pol0_Flx(0);

  av = avma;
  if (na >= get_Fl_threshold(p, Flx_SQR_SQRI_LIMIT, Flx_SQR2_SQRI_LIMIT))
  {
    m = maxbitcoeffpol(p,na);
    switch(m)
    {
    case BITS_IN_QUARTULONG:
      return Flx_shiftip(av, Flx_sqrspec_quartsqri(a,p,na), v);
    case BITS_IN_HALFULONG:
      return Flx_shiftip(av, Flx_sqrspec_halfsqri(a,p,na), v);
    case BITS_IN_LONG:
      return Flx_shiftip(av, Flx_sqrspec_sqri(a,p,na), v);
    case 2*BITS_IN_LONG:
      return Flx_shiftip(av, Flx_sqrspec_sqri_inflate(a,2,p,na), v);
    case 3*BITS_IN_LONG:
      return Flx_shiftip(av, Flx_sqrspec_sqri_inflate(a,3,p,na), v);
    default:
      return Flx_shiftip(av, Flx_sqrspec_Kronecker(a,m,p,na), v);
    }
  }
  if (na < get_Fl_threshold(p, Flx_SQR_KARATSUBA_LIMIT, Flx_SQR2_KARATSUBA_LIMIT))
    return Flx_shiftip(av, Flx_sqrspec_basecase(a,p,na), v);
  i=(na>>1); n0=na-i; na=i;
  a0=a+n0; n0a=n0;
  while (n0a && !a[n0a-1]) n0a--;

  c = Flx_sqrspec(a,p,n0a);
  c0= Flx_sqrspec(a0,p,na);
  if (p == 2) n0 *= 2;
  else
  {
    GEN c1, t = Flx_addspec(a0,a,p,na,n0a);
    t = Flx_sqr(t,p);
    c1= Flx_add(c0,c, p);
    c1= Flx_sub(t, c1, p);
    c0 = Flx_addshift(c0,c1,p,n0);
  }
  c0 = Flx_addshift(c0,c,p,n0);
  return Flx_shiftip(av,c0,v);
}

GEN
Flx_sqr(GEN x, ulong p)
{
  GEN z = Flx_sqrspec(x+2,p, lgpol(x));
  z[1] = x[1]; return z;
}

GEN
Flx_powu(GEN x, ulong n, ulong p)
{
  GEN y = pol1_Flx(x[1]), z;
  ulong m;
  if (n == 0) return y;
  m = n; z = x;
  for (;;)
  {
    if (m&1UL) y = Flx_mul(y,z, p);
    m >>= 1; if (!m) return y;
    z = Flx_sqr(z, p);
  }
}

GEN
Flx_halve(GEN y, ulong p)
{
  GEN z;
  long i, l;
  z = cgetg_copy(y, &l); z[1] = y[1];
  for(i=2; i<l; i++) uel(z,i) = Fl_halve(uel(y,i), p);
  return z;
}

static GEN
Flx_recipspec(GEN x, long l, long n)
{
  long i;
  GEN z=cgetg(n+2,t_VECSMALL)+2;
  for(i=0; i<l; i++)
    z[n-i-1] = x[i];
  for(   ; i<n; i++)
    z[n-i-1] = 0;
  return Flx_renormalize(z-2,n+2);
}

GEN
Flx_recip(GEN x)
{
  GEN z=Flx_recipspec(x+2,lgpol(x),lgpol(x));
  z[1]=x[1];
  return z;
}

/* Return h^degpol(P) P(x / h) */
GEN
Flx_rescale(GEN P, ulong h, ulong p)
{
  long i, l = lg(P);
  GEN Q = cgetg(l,t_VECSMALL);
  ulong hi = h;
  Q[l-1] = P[l-1];
  for (i=l-2; i>=2; i--)
  {
    Q[i] = Fl_mul(P[i], hi, p);
    if (i == 2) break;
    hi = Fl_mul(hi,h, p);
  }
  Q[1] = P[1]; return Q;
}

/*
 * x/polrecip(P)+O(x^n)
 */
static GEN
Flx_invBarrett_basecase(GEN T, ulong p)
{
  long i, l=lg(T)-1, lr=l-1, k;
  GEN r=cgetg(lr,t_VECSMALL); r[1] = T[1];
  r[2] = 1;
  if (SMALL_ULONG(p))
    for (i=3;i<lr;i++)
    {
      ulong u = uel(T, l-i+2);
      for (k=3; k<i; k++)
        { u += uel(T,l-i+k) * uel(r, k); if (u & HIGHBIT) u %= p; }
      r[i] = Fl_neg(u % p, p);
    }
  else
    for (i=3;i<lr;i++)
    {
      ulong u = Fl_neg(uel(T,l-i+2), p);
      for (k=3; k<i; k++)
        u = Fl_sub(u, Fl_mul(uel(T,l-i+k), uel(r,k), p), p);
      r[i] = u;
    }
  return Flx_renormalize(r,lr);
}

/* Return new lgpol */
static long
Flx_lgrenormalizespec(GEN x, long lx)
{
  long i;
  for (i = lx-1; i>=0; i--)
    if (x[i]) break;
  return i+1;
}
static GEN
Flx_invBarrett_Newton(GEN T, ulong p)
{
  long nold, lx, lz, lq, l = degpol(T), lQ;
  GEN q, y, z, x = zero_zv(l+1) + 2;
  ulong mask = quadratic_prec_mask(l-2); /* assume l > 2 */
  pari_sp av;

  y = T+2;
  q = Flx_recipspec(y,l+1,l+1); lQ = lgpol(q); q+=2;
  av = avma;
  /* We work on _spec_ Flx's, all the l[xzq12] below are lgpol's */

  /* initialize */
  x[0] = Fl_inv(q[0], p);
  if (lQ>1 && q[1])
  {
    ulong u = q[1];
    if (x[0] != 1) u = Fl_mul(u, Fl_sqr(x[0],p), p);
    x[1] = p - u; lx = 2;
  }
  else
    lx = 1;
  nold = 1;
  for (; mask > 1; set_avma(av))
  { /* set x -= x(x*q - 1) + O(t^(nnew + 1)), knowing x*q = 1 + O(t^(nold+1)) */
    long i, lnew, nnew = nold << 1;

    if (mask & 1) nnew--;
    mask >>= 1;

    lnew = nnew + 1;
    lq = Flx_lgrenormalizespec(q, minss(lQ, lnew));
    z = Flx_mulspec(x, q, p, lx, lq); /* FIXME: high product */
    lz = lgpol(z); if (lz > lnew) lz = lnew;
    z += 2;
    /* subtract 1 [=>first nold words are 0]: renormalize so that z(0) != 0 */
    for (i = nold; i < lz; i++) if (z[i]) break;
    nold = nnew;
    if (i >= lz) continue; /* z-1 = 0(t^(nnew + 1)) */

    /* z + i represents (x*q - 1) / t^i */
    lz = Flx_lgrenormalizespec (z+i, lz-i);
    z = Flx_mulspec(x, z+i, p, lx, lz); /* FIXME: low product */
    lz = lgpol(z); z += 2;
    if (lz > lnew-i) lz = Flx_lgrenormalizespec(z, lnew-i);

    lx = lz+ i;
    y  = x + i; /* x -= z * t^i, in place */
    for (i = 0; i < lz; i++) y[i] = Fl_neg(z[i], p);
  }
  x -= 2; setlg(x, lx + 2); x[1] = T[1];
  return x;
}

GEN
Flx_invBarrett(GEN T, ulong p)
{
  pari_sp ltop = avma;
  long l = lgpol(T);
  GEN r;
  if (l < 3) return pol0_Flx(T[1]);
  if (l < get_Fl_threshold(p, Flx_INVBARRETT_LIMIT, Flx_INVBARRETT2_LIMIT))
  {
    ulong c = T[l+1];
    if (c!=1)
    {
      ulong ci = Fl_inv(c,p);
      T=Flx_Fl_mul(T, ci, p);
      r=Flx_invBarrett_basecase(T,p);
      r=Flx_Fl_mul(r,ci,p);
    }
    else
      r=Flx_invBarrett_basecase(T,p);
  }
  else
    r = Flx_invBarrett_Newton(T,p);
  return gerepileuptoleaf(ltop, r);
}

GEN
Flx_get_red(GEN T, ulong p)
{
  if (typ(T)!=t_VECSMALL
    || lgpol(T) < get_Fl_threshold(p, Flx_BARRETT_LIMIT,
                                       Flx_BARRETT2_LIMIT))
    return T;
  retmkvec2(Flx_invBarrett(T,p),T);
}

/* separate from Flx_divrem for maximal speed. */
static GEN
Flx_rem_basecase(GEN x, GEN y, ulong p)
{
  pari_sp av;
  GEN z, c;
  long dx,dy,dy1,dz,i,j;
  ulong p1,inv;
  long vs=x[1];

  dy = degpol(y); if (!dy) return pol0_Flx(x[1]);
  dx = degpol(x);
  dz = dx-dy; if (dz < 0) return Flx_copy(x);
  x += 2; y += 2;
  inv = y[dy];
  if (inv != 1UL) inv = Fl_inv(inv,p);
  for (dy1=dy-1; dy1>=0 && !y[dy1]; dy1--);

  c = cgetg(dy+3, t_VECSMALL); c[1]=vs; c += 2; av=avma;
  z = cgetg(dz+3, t_VECSMALL); z[1]=vs; z += 2;

  if (SMALL_ULONG(p))
  {
    z[dz] = (inv*x[dx]) % p;
    for (i=dx-1; i>=dy; --i)
    {
      p1 = p - x[i]; /* compute -p1 instead of p1 (pb with ulongs otherwise) */
      for (j=i-dy1; j<=i && j<=dz; j++)
      {
        p1 += z[j]*y[i-j];
        if (p1 & HIGHBIT) p1 %= p;
      }
      p1 %= p;
      z[i-dy] = p1? ((p - p1)*inv) % p: 0;
    }
    for (i=0; i<dy; i++)
    {
      p1 = z[0]*y[i];
      for (j=maxss(1,i-dy1); j<=i && j<=dz; j++)
      {
        p1 += z[j]*y[i-j];
        if (p1 & HIGHBIT) p1 %= p;
      }
      c[i] = Fl_sub(x[i], p1%p, p);
    }
  }
  else
  {
    ulong pi = get_Fl_red(p);
    z[dz] = Fl_mul_pre(inv, x[dx], p, pi);
    for (i=dx-1; i>=dy; --i)
    {
      p1 = p - x[i]; /* compute -p1 instead of p1 (pb with ulongs otherwise) */
      for (j=i-dy1; j<=i && j<=dz; j++)
        p1 = Fl_addmul_pre(p1, z[j], y[i - j], p, pi);
      z[i-dy] = p1? Fl_mul_pre(p - p1, inv, p, pi): 0;
    }
    for (i=0; i<dy; i++)
    {
      p1 = Fl_mul_pre(z[0],y[i],p,pi);
      for (j=maxss(1,i-dy1); j<=i && j<=dz; j++)
        p1 = Fl_addmul_pre(p1, z[j], y[i - j], p, pi);
      c[i] = Fl_sub(x[i], p1, p);
    }
  }
  i = dy-1; while (i>=0 && !c[i]) i--;
  set_avma(av); return Flx_renormalize(c-2, i+3);
}

/* as FpX_divrem but working only on ulong types.
 * if relevant, *pr is the last object on stack */
static GEN
Flx_divrem_basecase(GEN x, GEN y, ulong p, GEN *pr)
{
  GEN z,q,c;
  long dx,dy,dy1,dz,i,j;
  ulong p1,inv;
  long sv=x[1];

  dy = degpol(y);
  if (dy<0) pari_err_INV("Flx_divrem",y);
  if (pr == ONLY_REM) return Flx_rem_basecase(x, y, p);
  if (!dy)
  {
    if (pr && pr != ONLY_DIVIDES) *pr = pol0_Flx(sv);
    if (y[2] == 1UL) return Flx_copy(x);
    return Flx_Fl_mul(x, Fl_inv(y[2], p), p);
  }
  dx = degpol(x);
  dz = dx-dy;
  if (dz < 0)
  {
    q = pol0_Flx(sv);
    if (pr && pr != ONLY_DIVIDES) *pr = Flx_copy(x);
    return q;
  }
  x += 2;
  y += 2;
  z = cgetg(dz + 3, t_VECSMALL); z[1] = sv; z += 2;
  inv = uel(y, dy);
  if (inv != 1UL) inv = Fl_inv(inv,p);
  for (dy1=dy-1; dy1>=0 && !y[dy1]; dy1--);

  if (SMALL_ULONG(p))
  {
    z[dz] = (inv*x[dx]) % p;
    for (i=dx-1; i>=dy; --i)
    {
      p1 = p - x[i]; /* compute -p1 instead of p1 (pb with ulongs otherwise) */
      for (j=i-dy1; j<=i && j<=dz; j++)
      {
        p1 += z[j]*y[i-j];
        if (p1 & HIGHBIT) p1 %= p;
      }
      p1 %= p;
      z[i-dy] = p1? (long) ((p - p1)*inv) % p: 0;
    }
  }
  else
  {
    z[dz] = Fl_mul(inv, x[dx], p);
    for (i=dx-1; i>=dy; --i)
    { /* compute -p1 instead of p1 (pb with ulongs otherwise) */
      p1 = p - uel(x,i);
      for (j=i-dy1; j<=i && j<=dz; j++)
        p1 = Fl_add(p1, Fl_mul(z[j],y[i-j],p), p);
      z[i-dy] = p1? Fl_mul(p - p1, inv, p): 0;
    }
  }
  q = Flx_renormalize(z-2, dz+3);
  if (!pr) return q;

  c = cgetg(dy + 3, t_VECSMALL); c[1] = sv; c += 2;
  if (SMALL_ULONG(p))
  {
    for (i=0; i<dy; i++)
    {
      p1 = (ulong)z[0]*y[i];
      for (j=maxss(1,i-dy1); j<=i && j<=dz; j++)
      {
        p1 += (ulong)z[j]*y[i-j];
        if (p1 & HIGHBIT) p1 %= p;
      }
      c[i] = Fl_sub(x[i], p1%p, p);
    }
  }
  else
  {
    for (i=0; i<dy; i++)
    {
      p1 = Fl_mul(z[0],y[i],p);
      for (j=maxss(1,i-dy1); j<=i && j<=dz; j++)
        p1 = Fl_add(p1, Fl_mul(z[j],y[i-j],p), p);
      c[i] = Fl_sub(x[i], p1, p);
    }
  }
  i=dy-1; while (i>=0 && !c[i]) i--;
  c = Flx_renormalize(c-2, i+3);
  if (pr == ONLY_DIVIDES)
  { if (lg(c) != 2) return NULL; }
  else
    *pr = c;
  return q;
}

/* Compute x mod T where 2 <= degpol(T) <= l+1 <= 2*(degpol(T)-1)
 * and mg is the Barrett inverse of T. */
static GEN
Flx_divrem_Barrettspec(GEN x, long l, GEN mg, GEN T, ulong p, GEN *pr)
{
  GEN q, r;
  long lt = degpol(T); /*We discard the leading term*/
  long ld, lm, lT, lmg;
  ld = l-lt;
  lm = minss(ld, lgpol(mg));
  lT  = Flx_lgrenormalizespec(T+2,lt);
  lmg = Flx_lgrenormalizespec(mg+2,lm);
  q = Flx_recipspec(x+lt,ld,ld);               /* q = rec(x)      lz<=ld*/
  q = Flx_mulspec(q+2,mg+2,p,lgpol(q),lmg);    /* q = rec(x) * mg lz<=ld+lm*/
  q = Flx_recipspec(q+2,minss(ld,lgpol(q)),ld);/* q = rec (rec(x) * mg) lz<=ld*/
  if (!pr) return q;
  r = Flx_mulspec(q+2,T+2,p,lgpol(q),lT);      /* r = q*pol       lz<=ld+lt*/
  r = Flx_subspec(x,r+2,p,lt,minss(lt,lgpol(r)));/* r = x - q*pol lz<=lt */
  if (pr == ONLY_REM) return r;
  *pr = r; return q;
}

static GEN
Flx_divrem_Barrett(GEN x, GEN mg, GEN T, ulong p, GEN *pr)
{
  GEN q = NULL, r = Flx_copy(x);
  long l = lgpol(x), lt = degpol(T), lm = 2*lt-1, v = T[1];
  long i;
  if (l <= lt)
  {
    if (pr == ONLY_REM) return Flx_copy(x);
    if (pr == ONLY_DIVIDES) return lgpol(x)? NULL: pol0_Flx(v);
    if (pr) *pr = Flx_copy(x);
    return pol0_Flx(v);
  }
  if (lt <= 1)
    return Flx_divrem_basecase(x,T,p,pr);
  if (pr != ONLY_REM && l>lm)
  { q = zero_zv(l-lt+1); q[1] = T[1]; }
  while (l>lm)
  {
    GEN zr, zq = Flx_divrem_Barrettspec(r+2+l-lm,lm,mg,T,p,&zr);
    long lz = lgpol(zr);
    if (pr != ONLY_REM)
    {
      long lq = lgpol(zq);
      for(i=0; i<lq; i++) q[2+l-lm+i] = zq[2+i];
    }
    for(i=0; i<lz; i++)   r[2+l-lm+i] = zr[2+i];
    l = l-lm+lz;
  }
  if (pr == ONLY_REM)
  {
    if (l > lt)
      r = Flx_divrem_Barrettspec(r+2,l,mg,T,p,ONLY_REM);
    else
      r = Flx_renormalize(r, l+2);
    r[1] = v; return r;
  }
  if (l > lt)
  {
    GEN zq = Flx_divrem_Barrettspec(r+2,l,mg,T,p, pr ? &r: NULL);
    if (!q) q = zq;
    else
    {
      long lq = lgpol(zq);
      for(i=0; i<lq; i++) q[2+i] = zq[2+i];
    }
  }
  else if (pr)
    r = Flx_renormalize(r, l+2);
  q[1] = v; q = Flx_renormalize(q, lg(q));
  if (pr == ONLY_DIVIDES) return lgpol(r)? NULL: q;
  if (pr) { r[1] = v; *pr = r; }
  return q;
}

GEN
Flx_divrem(GEN x, GEN T, ulong p, GEN *pr)
{
  GEN B, y;
  long dy, dx, d;
  if (pr==ONLY_REM) return Flx_rem(x, T, p);
  y = get_Flx_red(T, &B);
  dy = degpol(y); dx = degpol(x); d = dx-dy;
  if (!B && d+3 < get_Fl_threshold(p, Flx_DIVREM_BARRETT_LIMIT,Flx_DIVREM2_BARRETT_LIMIT))
    return Flx_divrem_basecase(x,y,p,pr);
  else
  {
    pari_sp av = avma;
    GEN mg = B? B: Flx_invBarrett(y, p);
    GEN q1 = Flx_divrem_Barrett(x,mg,y,p,pr);
    if (!q1) return gc_NULL(av);
    if (!pr || pr==ONLY_DIVIDES) return gerepileuptoleaf(av, q1);
    gerepileall(av,2,&q1,pr);
    return q1;
  }
}

GEN
Flx_rem(GEN x, GEN T, ulong p)
{
  GEN B, y = get_Flx_red(T, &B);
  long dy = degpol(y), dx = degpol(x), d = dx-dy;
  if (d < 0) return Flx_copy(x);
  if (!B && d+3 < get_Fl_threshold(p, Flx_REM_BARRETT_LIMIT,Flx_REM2_BARRETT_LIMIT))
    return Flx_rem_basecase(x,y,p);
  else
  {
    pari_sp av=avma;
    GEN mg = B ? B: Flx_invBarrett(y, p);
    GEN r  = Flx_divrem_Barrett(x, mg, y, p, ONLY_REM);
    return gerepileuptoleaf(av, r);
  }
}

/* reduce T mod (X^n - 1, p). Shallow function */
GEN
Flx_mod_Xnm1(GEN T, ulong n, ulong p)
{
  long i, j, L = lg(T), l = n+2;
  GEN S;
  if (L <= l || n & ~LGBITS) return T;
  S = cgetg(l, t_VECSMALL);
  S[1] = T[1];
  for (i = 2; i < l; i++) S[i] = T[i];
  for (j = 2; i < L; i++) {
    S[j] = Fl_add(S[j], T[i], p);
    if (++j == l) j = 2;
  }
  return Flx_renormalize(S, l);
}
/* reduce T mod (X^n + 1, p). Shallow function */
GEN
Flx_mod_Xn1(GEN T, ulong n, ulong p)
{
  long i, j, L = lg(T), l = n+2;
  GEN S;
  if (L <= l || n & ~LGBITS) return T;
  S = cgetg(l, t_VECSMALL);
  S[1] = T[1];
  for (i = 2; i < l; i++) S[i] = T[i];
  for (j = 2; i < L; i++) {
    S[j] = Fl_sub(S[j], T[i], p);
    if (++j == l) j = 2;
  }
  return Flx_renormalize(S, l);
}

struct _Flxq {
  GEN aut;
  GEN T;
  ulong p;
};

static GEN
_Flx_divrem(void * E, GEN x, GEN y, GEN *r)
{
  struct _Flxq *D = (struct _Flxq*) E;
  return Flx_divrem(x, y, D->p, r);
}
static GEN
_Flx_add(void * E, GEN x, GEN y) {
  struct _Flxq *D = (struct _Flxq*) E;
  return Flx_add(x, y, D->p);
}
static GEN
_Flx_mul(void *E, GEN x, GEN y) {
  struct _Flxq *D = (struct _Flxq*) E;
  return Flx_mul(x, y, D->p);
}
static GEN
_Flx_sqr(void *E, GEN x) {
  struct _Flxq *D = (struct _Flxq*) E;
  return Flx_sqr(x, D->p);
}

static struct bb_ring Flx_ring = { _Flx_add,_Flx_mul,_Flx_sqr };

GEN
Flx_digits(GEN x, GEN T, ulong p)
{
  pari_sp av = avma;
  struct _Flxq D;
  long d = degpol(T), n = (lgpol(x)+d-1)/d;
  GEN z;
  D.p = p;
  z = gen_digits(x,T,n,(void *)&D, &Flx_ring, _Flx_divrem);
  return gerepileupto(av, z);
}

GEN
FlxV_Flx_fromdigits(GEN x, GEN T, ulong p)
{
  pari_sp av = avma;
  struct _Flxq D;
  GEN z;
  D.p = p;
  z = gen_fromdigits(x,T,(void *)&D, &Flx_ring);
  return gerepileupto(av, z);
}

long
Flx_val(GEN x)
{
  long i, l=lg(x);
  if (l==2)  return LONG_MAX;
  for (i=2; i<l && x[i]==0; i++) /*empty*/;
  return i-2;
}
long
Flx_valrem(GEN x, GEN *Z)
{
  long v, i, l=lg(x);
  GEN y;
  if (l==2) { *Z = Flx_copy(x); return LONG_MAX; }
  for (i=2; i<l && x[i]==0; i++) /*empty*/;
  v = i-2;
  if (v == 0) { *Z = x; return 0; }
  l -= v;
  y = cgetg(l, t_VECSMALL); y[1] = x[1];
  for (i=2; i<l; i++) y[i] = x[i+v];
  *Z = y; return v;
}

GEN
Flx_deriv(GEN z, ulong p)
{
  long i,l = lg(z)-1;
  GEN x;
  if (l < 2) l = 2;
  x = cgetg(l, t_VECSMALL); x[1] = z[1]; z++;
  if (HIGHWORD(l | p))
    for (i=2; i<l; i++) x[i] = Fl_mul((ulong)i-1, z[i], p);
  else
    for (i=2; i<l; i++) x[i] = ((i-1) * z[i]) % p;
  return Flx_renormalize(x,l);
}

static GEN
Flx_integXn(GEN x, long n, ulong p)
{
  long i, lx = lg(x);
  GEN y;
  if (lx == 2) return Flx_copy(x);
  y = cgetg(lx, t_VECSMALL); y[1] = x[1];
  for (i=2; i<lx; i++)
  {
    ulong xi = uel(x,i);
    if (xi == 0)
      uel(y,i) = 0;
    else
    {
      ulong j = n+i-1;
      ulong d = ugcd(j, xi);
      if (d==1)
        uel(y,i) = Fl_div(xi, j, p);
      else
        uel(y,i) = Fl_div(xi/d, j/d, p);
    }
  }
  return Flx_renormalize(y, lx);;
}

GEN
Flx_integ(GEN x, ulong p)
{
  long i, lx = lg(x);
  GEN y;
  if (lx == 2) return Flx_copy(x);
  y = cgetg(lx+1, t_VECSMALL); y[1] = x[1];
  uel(y,2) = 0;
  for (i=3; i<=lx; i++)
    uel(y,i) = uel(x,i-1) ? Fl_div(uel(x,i-1), (i-2)%p, p): 0UL;
  return Flx_renormalize(y, lx+1);;
}

/* assume p prime */
GEN
Flx_diff1(GEN P, ulong p)
{
  return Flx_sub(Flx_translate1(P, p), P, p);
}

GEN
Flx_deflate(GEN x0, long d)
{
  GEN z, y, x;
  long i,id, dy, dx = degpol(x0);
  if (d == 1 || dx <= 0) return Flx_copy(x0);
  dy = dx/d;
  y = cgetg(dy+3, t_VECSMALL); y[1] = x0[1];
  z = y + 2;
  x = x0+ 2;
  for (i=id=0; i<=dy; i++,id+=d) z[i] = x[id];
  return y;
}

GEN
Flx_inflate(GEN x0, long d)
{
  long i, id, dy, dx = degpol(x0);
  GEN x = x0 + 2, z, y;
  if (dx <= 0) return Flx_copy(x0);
  dy = dx*d;
  y = cgetg(dy+3, t_VECSMALL); y[1] = x0[1];
  z = y + 2;
  for (i=0; i<=dy; i++) z[i] = 0;
  for (i=id=0; i<=dx; i++,id+=d) z[id] = x[i];
  return y;
}

/* write p(X) = a_0(X^k) + X*a_1(X^k) + ... + X^(k-1)*a_{k-1}(X^k) */
GEN
Flx_splitting(GEN p, long k)
{
  long n = degpol(p), v = p[1], m, i, j, l;
  GEN r;

  m = n/k;
  r = cgetg(k+1,t_VEC);
  for(i=1; i<=k; i++)
  {
    gel(r,i) = cgetg(m+3, t_VECSMALL);
    mael(r,i,1) = v;
  }
  for (j=1, i=0, l=2; i<=n; i++)
  {
    mael(r,j,l) = p[2+i];
    if (j==k) { j=1; l++; } else j++;
  }
  for(i=1; i<=k; i++)
    gel(r,i) = Flx_renormalize(gel(r,i),i<j?l+1:l);
  return r;
}
static GEN
Flx_halfgcd_basecase(GEN a, GEN b, ulong p)
{
  pari_sp av=avma;
  GEN u,u1,v,v1;
  long vx = a[1];
  long n = lgpol(a)>>1;
  u1 = v = pol0_Flx(vx);
  u = v1 = pol1_Flx(vx);
  while (lgpol(b)>n)
  {
    GEN r, q = Flx_divrem(a,b,p, &r);
    a = b; b = r; swap(u,u1); swap(v,v1);
    u1 = Flx_sub(u1, Flx_mul(u, q, p), p);
    v1 = Flx_sub(v1, Flx_mul(v, q ,p), p);
    if (gc_needed(av,2))
    {
      if (DEBUGMEM>1) pari_warn(warnmem,"Flx_halfgcd (d = %ld)",degpol(b));
      gerepileall(av,6, &a,&b,&u1,&v1,&u,&v);
    }
  }
  return gerepilecopy(av, mkmat2(mkcol2(u,u1), mkcol2(v,v1)));
}
/* ux + vy */
static GEN
Flx_addmulmul(GEN u, GEN v, GEN x, GEN y, ulong p)
{ return Flx_add(Flx_mul(u,x, p), Flx_mul(v,y, p), p); }

static GEN
FlxM_Flx_mul2(GEN M, GEN x, GEN y, ulong p)
{
  GEN res = cgetg(3, t_COL);
  gel(res, 1) = Flx_addmulmul(gcoeff(M,1,1), gcoeff(M,1,2), x, y, p);
  gel(res, 2) = Flx_addmulmul(gcoeff(M,2,1), gcoeff(M,2,2), x, y, p);
  return res;
}

#if 0
static GEN
FlxM_mul2_old(GEN M, GEN N, ulong p)
{
  GEN res = cgetg(3, t_MAT);
  gel(res, 1) = FlxM_Flx_mul2(M,gcoeff(N,1,1),gcoeff(N,2,1),p);
  gel(res, 2) = FlxM_Flx_mul2(M,gcoeff(N,1,2),gcoeff(N,2,2),p);
  return res;
}
#endif
/* A,B are 2x2 matrices, Flx entries. Return A x B using Strassen 7M formula */
static GEN
FlxM_mul2(GEN A, GEN B, ulong p)
{
  GEN A11=gcoeff(A,1,1),A12=gcoeff(A,1,2), B11=gcoeff(B,1,1),B12=gcoeff(B,1,2);
  GEN A21=gcoeff(A,2,1),A22=gcoeff(A,2,2), B21=gcoeff(B,2,1),B22=gcoeff(B,2,2);
  GEN M1 = Flx_mul(Flx_add(A11,A22, p), Flx_add(B11,B22, p), p);
  GEN M2 = Flx_mul(Flx_add(A21,A22, p), B11, p);
  GEN M3 = Flx_mul(A11, Flx_sub(B12,B22, p), p);
  GEN M4 = Flx_mul(A22, Flx_sub(B21,B11, p), p);
  GEN M5 = Flx_mul(Flx_add(A11,A12, p), B22, p);
  GEN M6 = Flx_mul(Flx_sub(A21,A11, p), Flx_add(B11,B12, p), p);
  GEN M7 = Flx_mul(Flx_sub(A12,A22, p), Flx_add(B21,B22, p), p);
  GEN T1 = Flx_add(M1,M4, p), T2 = Flx_sub(M7,M5, p);
  GEN T3 = Flx_sub(M1,M2, p), T4 = Flx_add(M3,M6, p);
  retmkmat2(mkcol2(Flx_add(T1,T2, p), Flx_add(M2,M4, p)),
            mkcol2(Flx_add(M3,M5, p), Flx_add(T3,T4, p)));
}

/* Return [0,1;1,-q]*M */
static GEN
Flx_FlxM_qmul(GEN q, GEN M, ulong p)
{
  GEN u, v, res = cgetg(3, t_MAT);
  u = Flx_sub(gcoeff(M,1,1), Flx_mul(gcoeff(M,2,1), q, p), p);
  gel(res,1) = mkcol2(gcoeff(M,2,1), u);
  v = Flx_sub(gcoeff(M,1,2), Flx_mul(gcoeff(M,2,2), q, p), p);
  gel(res,2) = mkcol2(gcoeff(M,2,2), v);
  return res;
}

static GEN
matid2_FlxM(long v)
{
  return mkmat2(mkcol2(pol1_Flx(v),pol0_Flx(v)),
                mkcol2(pol0_Flx(v),pol1_Flx(v)));
}

static GEN
Flx_halfgcd_split(GEN x, GEN y, ulong p)
{
  pari_sp av=avma;
  GEN R, S, V;
  GEN y1, r, q;
  long l = lgpol(x), n = l>>1, k;
  if (lgpol(y)<=n) return matid2_FlxM(x[1]);
  R = Flx_halfgcd(Flx_shift(x,-n),Flx_shift(y,-n),p);
  V = FlxM_Flx_mul2(R,x,y,p); y1 = gel(V,2);
  if (lgpol(y1)<=n) return gerepilecopy(av, R);
  q = Flx_divrem(gel(V,1), y1, p, &r);
  k = 2*n-degpol(y1);
  S = Flx_halfgcd(Flx_shift(y1,-k), Flx_shift(r,-k),p);
  return gerepileupto(av, FlxM_mul2(S,Flx_FlxM_qmul(q,R,p),p));
}

/* Return M in GL_2(Fl[X]) such that:
if [a',b']~=M*[a,b]~ then degpol(a')>= (lgpol(a)>>1) >degpol(b')
*/

static GEN
Flx_halfgcd_i(GEN x, GEN y, ulong p)
{
  if (lgpol(x) < get_Fl_threshold(p, Flx_HALFGCD_LIMIT, Flx_HALFGCD2_LIMIT))
    return Flx_halfgcd_basecase(x,y,p);
  return Flx_halfgcd_split(x,y,p);
}

GEN
Flx_halfgcd(GEN x, GEN y, ulong p)
{
  pari_sp av;
  GEN M,q,r;
  long lx=lgpol(x), ly=lgpol(y);
  if (!lx)
  {
      long v = x[1];
      retmkmat2(mkcol2(pol0_Flx(v),pol1_Flx(v)),
                mkcol2(pol1_Flx(v),pol0_Flx(v)));
  }
  if (ly < lx) return Flx_halfgcd_i(x,y,p);
  av = avma;
  q = Flx_divrem(y,x,p,&r);
  M = Flx_halfgcd_i(x,r,p);
  gcoeff(M,1,1) = Flx_sub(gcoeff(M,1,1), Flx_mul(q, gcoeff(M,1,2), p), p);
  gcoeff(M,2,1) = Flx_sub(gcoeff(M,2,1), Flx_mul(q, gcoeff(M,2,2), p), p);
  return gerepilecopy(av, M);
}

/*Do not garbage collect*/
static GEN
Flx_gcd_basecase(GEN a, GEN b, ulong p)
{
  pari_sp av = avma;
  ulong iter = 0;
  if (lg(b) > lg(a)) swap(a, b);
  while (lgpol(b))
  {
    GEN c = Flx_rem(a,b,p);
    iter++; a = b; b = c;
    if (gc_needed(av,2))
    {
      if (DEBUGMEM>1) pari_warn(warnmem,"Flx_gcd (d = %ld)",degpol(c));
      gerepileall(av,2, &a,&b);
    }
  }
  return iter < 2 ? Flx_copy(a) : a;
}

GEN
Flx_gcd(GEN x, GEN y, ulong p)
{
  pari_sp av = avma;
  long lim;
  if (!lgpol(x)) return Flx_copy(y);
  lim = get_Fl_threshold(p, Flx_GCD_LIMIT, Flx_GCD2_LIMIT);
  while (lgpol(y) >= lim)
  {
    GEN c;
    if (lgpol(y)<=(lgpol(x)>>1))
    {
      GEN r = Flx_rem(x, y, p);
      x = y; y = r;
    }
    c = FlxM_Flx_mul2(Flx_halfgcd(x,y, p), x, y, p);
    x = gel(c,1); y = gel(c,2);
    if (gc_needed(av,2))
    {
      if (DEBUGMEM>1) pari_warn(warnmem,"Flx_gcd (y = %ld)",degpol(y));
      gerepileall(av,2,&x,&y);
    }
  }
  return gerepileuptoleaf(av, Flx_gcd_basecase(x,y,p));
}

int
Flx_is_squarefree(GEN z, ulong p)
{
  pari_sp av = avma;
  GEN d = Flx_gcd(z, Flx_deriv(z,p) , p);
  return gc_bool(av, degpol(d) == 0);
}

static long
Flx_is_smooth_squarefree(GEN f, long r, ulong p)
{
  pari_sp av = avma;
  long i;
  GEN sx = polx_Flx(f[1]), a = sx;
  for(i=1;;i++)
  {
    if (degpol(f)<=r) return gc_long(av,1);
    a = Flxq_powu(Flx_rem(a,f,p), p, f, p);
    if (Flx_equal(a, sx)) return gc_long(av,1);
    if (i==r) return gc_long(av,0);
    f = Flx_div(f, Flx_gcd(Flx_sub(a,sx,p),f,p),p);
  }
}

static long
Flx_is_l_pow(GEN x, ulong p)
{
  ulong i, lx = lgpol(x);
  for (i=1; i<lx; i++)
    if (x[i+2] && i%p) return 0;
  return 1;
}

int
Flx_is_smooth(GEN g, long r, ulong p)
{
  while (1)
  {
    GEN f = Flx_gcd(g, Flx_deriv(g, p), p);
    if (!Flx_is_smooth_squarefree(Flx_div(g, f, p), r, p))
      return 0;
    if (degpol(f)==0) return 1;
    g = Flx_is_l_pow(f,p) ? Flx_deflate(f, p): f;
  }
}

static GEN
Flx_extgcd_basecase(GEN a, GEN b, ulong p, GEN *ptu, GEN *ptv)
{
  pari_sp av=avma;
  GEN u,v,d,d1,v1;
  long vx = a[1];
  d = a; d1 = b;
  v = pol0_Flx(vx); v1 = pol1_Flx(vx);
  while (lgpol(d1))
  {
    GEN r, q = Flx_divrem(d,d1,p, &r);
    v = Flx_sub(v,Flx_mul(q,v1,p),p);
    u=v; v=v1; v1=u;
    u=r; d=d1; d1=u;
    if (gc_needed(av,2))
    {
      if (DEBUGMEM>1) pari_warn(warnmem,"Flx_extgcd (d = %ld)",degpol(d));
      gerepileall(av,5, &d,&d1,&u,&v,&v1);
    }
  }
  if (ptu) *ptu = Flx_div(Flx_sub(d, Flx_mul(b,v,p), p), a, p);
  *ptv = v; return d;
}

static GEN
Flx_extgcd_halfgcd(GEN x, GEN y, ulong p, GEN *ptu, GEN *ptv)
{
  pari_sp av=avma;
  GEN u,v,R = matid2_FlxM(x[1]);
  long lim = get_Fl_threshold(p, Flx_EXTGCD_LIMIT, Flx_EXTGCD2_LIMIT);
  while (lgpol(y) >= lim)
  {
    GEN M, c;
    if (lgpol(y)<=(lgpol(x)>>1))
    {
      GEN r, q = Flx_divrem(x, y, p, &r);
      x = y; y = r;
      R = Flx_FlxM_qmul(q, R, p);
    }
    M = Flx_halfgcd(x,y, p);
    c = FlxM_Flx_mul2(M, x,y, p);
    R = FlxM_mul2(M, R, p);
    x = gel(c,1); y = gel(c,2);
    gerepileall(av,3,&x,&y,&R);
  }
  y = Flx_extgcd_basecase(x,y,p,&u,&v);
  if (ptu) *ptu = Flx_addmulmul(u,v,gcoeff(R,1,1),gcoeff(R,2,1),p);
  *ptv = Flx_addmulmul(u,v,gcoeff(R,1,2),gcoeff(R,2,2),p);
  return y;
}

/* x and y in Z[X], return lift(gcd(x mod p, y mod p)). Set u and v st
 * ux + vy = gcd (mod p) */
GEN
Flx_extgcd(GEN x, GEN y, ulong p, GEN *ptu, GEN *ptv)
{
  GEN d;
  pari_sp ltop=avma;
  long lim = get_Fl_threshold(p, Flx_EXTGCD_LIMIT, Flx_EXTGCD2_LIMIT);
  if (lgpol(y) >= lim)
    d = Flx_extgcd_halfgcd(x, y, p, ptu, ptv);
  else
    d = Flx_extgcd_basecase(x, y, p, ptu, ptv);
  gerepileall(ltop,ptu?3:2,&d,ptv,ptu);
  return d;
}

ulong
Flx_resultant(GEN a, GEN b, ulong p)
{
  long da,db,dc,cnt;
  ulong lb, res = 1UL;
  pari_sp av;
  GEN c;

  if (lgpol(a)==0 || lgpol(b)==0) return 0;
  da = degpol(a);
  db = degpol(b);
  if (db > da)
  {
    swapspec(a,b, da,db);
    if (both_odd(da,db)) res = p-res;
  }
  else if (!da) return 1; /* = res * a[2] ^ db, since 0 <= db <= da = 0 */
  cnt = 0; av = avma;
  while (db)
  {
    lb = b[db+2];
    c = Flx_rem(a,b, p);
    a = b; b = c; dc = degpol(c);
    if (dc < 0) return gc_long(av,0);

    if (both_odd(da,db)) res = p - res;
    if (lb != 1) res = Fl_mul(res, Fl_powu(lb, da - dc, p), p);
    if (++cnt == 100) { cnt = 0; gerepileall(av, 2, &a, &b); }
    da = db; /* = degpol(a) */
    db = dc; /* = degpol(b) */
  }
  return gc_ulong(av, Fl_mul(res, Fl_powu(b[2], da, p), p));
}

/* If resultant is 0, *ptU and *ptV are not set */
ulong
Flx_extresultant(GEN a, GEN b, ulong p, GEN *ptU, GEN *ptV)
{
  GEN z,q,u,v, x = a, y = b;
  ulong lb, res = 1UL;
  pari_sp av = avma;
  long dx, dy, dz;
  long vs=a[1];

  dx = degpol(x);
  dy = degpol(y);
  if (dy > dx)
  {
    swap(x,y); lswap(dx,dy); pswap(ptU, ptV);
    a = x; b = y;
    if (both_odd(dx,dy)) res = p-res;
  }
  /* dy <= dx */
  if (dy < 0) return 0;

  u = pol0_Flx(vs);
  v = pol1_Flx(vs); /* v = 1 */
  while (dy)
  { /* b u = x (a), b v = y (a) */
    lb = y[dy+2];
    q = Flx_divrem(x,y, p, &z);
    x = y; y = z; /* (x,y) = (y, x - q y) */
    dz = degpol(z); if (dz < 0) return gc_ulong(av,0);
    z = Flx_sub(u, Flx_mul(q,v, p), p);
    u = v; v = z; /* (u,v) = (v, u - q v) */

    if (both_odd(dx,dy)) res = p - res;
    if (lb != 1) res = Fl_mul(res, Fl_powu(lb, dx-dz, p), p);
    dx = dy; /* = degpol(x) */
    dy = dz; /* = degpol(y) */
  }
  res = Fl_mul(res, Fl_powu(y[2], dx, p), p);
  lb = Fl_mul(res, Fl_inv(y[2],p), p);
  v = gerepileuptoleaf(av, Flx_Fl_mul(v, lb, p));
  av = avma;
  u = Flx_sub(Fl_to_Flx(res,vs), Flx_mul(b,v,p), p);
  u = gerepileuptoleaf(av, Flx_div(u,a,p)); /* = (res - b v) / a */
  *ptU = u;
  *ptV = v; return res;
}

ulong
Flx_eval_powers_pre(GEN x, GEN y, ulong p, ulong pi)
{
  ulong l0, l1, h0, h1, v1,  i = 1, lx = lg(x)-1;
  LOCAL_OVERFLOW;
  LOCAL_HIREMAINDER;
  x++;

  if (lx == 1)
    return 0;
  l1 = mulll(uel(x,i), uel(y,i)); h1 = hiremainder; v1 = 0;
  while (++i < lx) {
    l0 = mulll(uel(x,i), uel(y,i)); h0 = hiremainder;
    l1 = addll(l0, l1); h1 = addllx(h0, h1); v1 += overflow;
  }
  if (v1 == 0) return remll_pre(h1, l1, p, pi);
  else return remlll_pre(v1, h1, l1, p, pi);
}

INLINE ulong
Flx_eval_pre_i(GEN x, ulong y, ulong p, ulong pi)
{
  ulong p1;
  long i=lg(x)-1;
  if (i<=2)
    return (i==2)? x[2]: 0;
  p1 = x[i];
  for (i--; i>=2; i--)
    p1 = Fl_addmul_pre(uel(x, i), p1, y, p, pi);
  return p1;
}

ulong
Flx_eval_pre(GEN x, ulong y, ulong p, ulong pi)
{
  if (degpol(x) > 15)
  {
    pari_sp av = avma;
    GEN v = Fl_powers_pre(y, degpol(x), p, pi);
    ulong r =  Flx_eval_powers_pre(x, v, p, pi);
    return gc_ulong(av,r);
  }
  else
    return Flx_eval_pre_i(x, y, p, pi);
}

ulong
Flx_eval(GEN x, ulong y, ulong p)
{
  return Flx_eval_pre(x, y, p, get_Fl_red(p));
}

ulong
Flv_prod_pre(GEN x, ulong p, ulong pi)
{
  pari_sp ltop = avma;
  GEN v;
  long i,k,lx = lg(x);
  if (lx == 1) return 1UL;
  if (lx == 2) return uel(x,1);
  v = cgetg(1+(lx << 1), t_VECSMALL);
  k = 1;
  for (i=1; i<lx-1; i+=2)
    uel(v,k++) = Fl_mul_pre(uel(x,i), uel(x,i+1), p, pi);
  if (i < lx) uel(v,k++) = uel(x,i);
  while (k > 2)
  {
    lx = k; k = 1;
    for (i=1; i<lx-1; i+=2)
      uel(v,k++) = Fl_mul_pre(uel(v,i), uel(v,i+1), p, pi);
    if (i < lx) uel(v,k++) = uel(v,i);
  }
  return gc_ulong(ltop, uel(v,1));
}

ulong
Flv_prod(GEN v, ulong p)
{
  return Flv_prod_pre(v, p, get_Fl_red(p));
}

GEN
FlxV_prod(GEN V, ulong p)
{
  struct _Flxq D;
  D.T = NULL; D.aut = NULL; D.p = p;
  return gen_product(V, (void *)&D, &_Flx_mul);
}

/* compute prod (x - a[i]) */
GEN
Flv_roots_to_pol(GEN a, ulong p, long vs)
{
  struct _Flxq D;
  long i,k,lx = lg(a);
  GEN p1;
  if (lx == 1) return pol1_Flx(vs);
  p1 = cgetg(lx, t_VEC);
  for (k=1,i=1; i<lx-1; i+=2)
    gel(p1,k++) = mkvecsmall4(vs, Fl_mul(a[i], a[i+1], p),
                              Fl_neg(Fl_add(a[i],a[i+1],p),p), 1);
  if (i < lx)
    gel(p1,k++) = mkvecsmall3(vs, Fl_neg(a[i],p), 1);
  D.T = NULL; D.aut = NULL; D.p = p;
  setlg(p1, k); return gen_product(p1, (void *)&D, _Flx_mul);
}

/* set v[i] = w[i]^{-1}; may be called with w = v, suitable for "large" p */
INLINE void
Flv_inv_pre_indir(GEN w, GEN v, ulong p, ulong pi)
{
  pari_sp av = avma;
  long n = lg(w), i;
  ulong u;
  GEN c;

  if (n == 1) return;
  c = cgetg(n, t_VECSMALL); c[1] = w[1];
  for (i = 2; i < n; ++i) c[i] = Fl_mul_pre(w[i], c[i-1], p, pi);
  i = n-1; u = Fl_inv(c[i], p);
  for ( ; i > 1; --i)
  {
    ulong t = Fl_mul_pre(u, c[i-1], p, pi);
    u = Fl_mul_pre(u, w[i], p, pi); v[i] = t;
  }
  v[1] = u; set_avma(av);
}

void
Flv_inv_pre_inplace(GEN v, ulong p, ulong pi) { Flv_inv_pre_indir(v,v, p, pi); }

GEN
Flv_inv_pre(GEN w, ulong p, ulong pi)
{ GEN v = cgetg(lg(w), t_VECSMALL); Flv_inv_pre_indir(w, v, p, pi); return v; }

/* set v[i] = w[i]^{-1}; may be called with w = v, suitable for SMALL_ULONG p */
INLINE void
Flv_inv_indir(GEN w, GEN v, ulong p)
{
  pari_sp av = avma;
  long n = lg(w), i;
  ulong u;
  GEN c;

  if (n == 1) return;
  c = cgetg(n, t_VECSMALL); c[1] = w[1];
  for (i = 2; i < n; ++i) c[i] = Fl_mul(w[i], c[i-1], p);
  i = n-1; u = Fl_inv(c[i], p);
  for ( ; i > 1; --i)
  {
    ulong t = Fl_mul(u, c[i-1], p);
    u = Fl_mul(u, w[i], p); v[i] = t;
  }
  v[1] = u; set_avma(av);
}
static void
Flv_inv_i(GEN v, GEN w, ulong p)
{
  if (SMALL_ULONG(p)) Flv_inv_indir(w, v, p);
  else Flv_inv_pre_indir(w, v, p, get_Fl_red(p));
}
void
Flv_inv_inplace(GEN v, ulong p) { Flv_inv_i(v, v, p); }
GEN
Flv_inv(GEN w, ulong p)
{ GEN v = cgetg(lg(w), t_VECSMALL); Flv_inv_i(v, w, p); return v; }

GEN
Flx_div_by_X_x(GEN a, ulong x, ulong p, ulong *rem)
{
  long l = lg(a), i;
  GEN a0, z0, z;
  if (l <= 3)
  {
    if (rem) *rem = l == 2? 0: a[2];
    return zero_Flx(a[1]);
  }
  z = cgetg(l-1,t_VECSMALL); z[1] = a[1];
  a0 = a + l-1;
  z0 = z + l-2; *z0 = *a0--;
  if (SMALL_ULONG(p))
  {
    for (i=l-3; i>1; i--) /* z[i] = (a[i+1] + x*z[i+1]) % p */
    {
      ulong t = (*a0-- + x *  *z0--) % p;
      *z0 = (long)t;
    }
    if (rem) *rem = (*a0 + x *  *z0) % p;
  }
  else
  {
    for (i=l-3; i>1; i--)
    {
      ulong t = Fl_add((ulong)*a0--, Fl_mul(x, *z0--, p), p);
      *z0 = (long)t;
    }
    if (rem) *rem = Fl_add((ulong)*a0, Fl_mul(x, *z0, p), p);
  }
  return z;
}

/* xa, ya = t_VECSMALL */
static GEN
Flv_producttree(GEN xa, GEN s, ulong p, long vs)
{
  long n = lg(xa)-1;
  long m = n==1 ? 1: expu(n-1)+1;
  long i, j, k, ls = lg(s);
  GEN T = cgetg(m+1, t_VEC);
  GEN t = cgetg(ls, t_VEC);
  for (j=1, k=1; j<ls; k+=s[j++])
    gel(t, j) = s[j] == 1 ?
             mkvecsmall3(vs, Fl_neg(xa[k], p), 1):
             mkvecsmall4(vs, Fl_mul(xa[k], xa[k+1], p),
                 Fl_neg(Fl_add(xa[k],xa[k+1],p),p), 1);
  gel(T,1) = t;
  for (i=2; i<=m; i++)
  {
    GEN u = gel(T, i-1);
    long n = lg(u)-1;
    GEN t = cgetg(((n+1)>>1)+1, t_VEC);
    for (j=1, k=1; k<n; j++, k+=2)
      gel(t, j) = Flx_mul(gel(u, k), gel(u, k+1), p);
    gel(T, i) = t;
  }
  return T;
}

static GEN
Flx_Flv_multieval_tree(GEN P, GEN xa, GEN T, ulong p)
{
  long i,j,k;
  long m = lg(T)-1;
  GEN R = cgetg(lg(xa), t_VECSMALL);
  GEN Tp = cgetg(m+1, t_VEC), t;
  gel(Tp, m) = mkvec(P);
  for (i=m-1; i>=1; i--)
  {
    GEN u = gel(T, i), v = gel(Tp, i+1);
    long n = lg(u)-1;
    t = cgetg(n+1, t_VEC);
    for (j=1, k=1; k<n; j++, k+=2)
    {
      gel(t, k)   = Flx_rem(gel(v, j), gel(u, k), p);
      gel(t, k+1) = Flx_rem(gel(v, j), gel(u, k+1), p);
    }
    gel(Tp, i) = t;
  }
  {
    GEN u = gel(T, i+1), v = gel(Tp, i+1);
    long n = lg(u)-1;
    for (j=1, k=1; j<=n; j++)
    {
      long c, d = degpol(gel(u,j));
      for (c=1; c<=d; c++, k++) R[k] = Flx_eval(gel(v, j), xa[k], p);
    }
    return gc_const((pari_sp)R, R);
  }
}

static GEN
FlvV_polint_tree(GEN T, GEN R, GEN s, GEN xa, GEN ya, ulong p, long vs)
{
  pari_sp av = avma;
  long m = lg(T)-1;
  long i, j, k, ls = lg(s);
  GEN Tp = cgetg(m+1, t_VEC);
  GEN t = cgetg(ls, t_VEC);
  for (j=1, k=1; j<ls; k+=s[j++])
    if (s[j]==2)
    {
      ulong a = Fl_mul(ya[k], R[k], p);
      ulong b = Fl_mul(ya[k+1], R[k+1], p);
      gel(t, j) = mkvecsmall3(vs, Fl_neg(Fl_add(Fl_mul(xa[k], b, p ),
                  Fl_mul(xa[k+1], a, p), p), p), Fl_add(a, b, p));
      gel(t, j) = Flx_renormalize(gel(t, j), 4);
    }
    else
      gel(t, j) = Fl_to_Flx(Fl_mul(ya[k], R[k], p), vs);
  gel(Tp, 1) = t;
  for (i=2; i<=m; i++)
  {
    GEN u = gel(T, i-1);
    GEN t = cgetg(lg(gel(T,i)), t_VEC);
    GEN v = gel(Tp, i-1);
    long n = lg(v)-1;
    for (j=1, k=1; k<n; j++, k+=2)
      gel(t, j) = Flx_add(Flx_mul(gel(u, k), gel(v, k+1), p),
                          Flx_mul(gel(u, k+1), gel(v, k), p), p);
    gel(Tp, i) = t;
  }
  return gerepileuptoleaf(av, gmael(Tp,m,1));
}

GEN
Flx_Flv_multieval(GEN P, GEN xa, ulong p)
{
  pari_sp av = avma;
  GEN s = producttree_scheme(lg(xa)-1);
  GEN T = Flv_producttree(xa, s, p, P[1]);
  return gerepileuptoleaf(av, Flx_Flv_multieval_tree(P, xa, T, p));
}

static GEN
FlxV_Flv_multieval_tree(GEN x, GEN xa, GEN T, ulong p)
{ pari_APPLY_same(Flx_Flv_multieval_tree(gel(x,i), xa, T, p)) }

GEN
FlxV_Flv_multieval(GEN P, GEN xa, ulong p)
{
  pari_sp av = avma;
  GEN s = producttree_scheme(lg(xa)-1);
  GEN T = Flv_producttree(xa, s, p, P[1]);
  return gerepileupto(av, FlxV_Flv_multieval_tree(P, xa, T, p));
}

GEN
Flv_polint(GEN xa, GEN ya, ulong p, long vs)
{
  pari_sp av = avma;
  GEN s = producttree_scheme(lg(xa)-1);
  GEN T = Flv_producttree(xa, s, p, vs);
  long m = lg(T)-1;
  GEN P = Flx_deriv(gmael(T, m, 1), p);
  GEN R = Flv_inv(Flx_Flv_multieval_tree(P, xa, T, p), p);
  return gerepileuptoleaf(av, FlvV_polint_tree(T, R, s, xa, ya, p, vs));
}

GEN
Flv_Flm_polint(GEN xa, GEN ya, ulong p, long vs)
{
  pari_sp av = avma;
  GEN s = producttree_scheme(lg(xa)-1);
  GEN T = Flv_producttree(xa, s, p, vs);
  long i, m = lg(T)-1, l = lg(ya)-1;
  GEN P = Flx_deriv(gmael(T, m, 1), p);
  GEN R = Flv_inv(Flx_Flv_multieval_tree(P, xa, T, p), p);
  GEN M = cgetg(l+1, t_VEC);
  for (i=1; i<=l; i++)
    gel(M,i) = FlvV_polint_tree(T, R, s, xa, gel(ya,i), p, vs);
  return gerepileupto(av, M);
}

GEN
Flv_invVandermonde(GEN L, ulong den, ulong p)
{
  pari_sp av = avma;
  long i, n = lg(L);
  GEN M, R;
  GEN s = producttree_scheme(n-1);
  GEN tree = Flv_producttree(L, s, p, 0);
  long m = lg(tree)-1;
  GEN T = gmael(tree, m, 1);
  R = Flv_inv(Flx_Flv_multieval_tree(Flx_deriv(T, p), L, tree, p), p);
  if (den!=1) R = Flv_Fl_mul(R, den, p);
  M = cgetg(n, t_MAT);
  for (i = 1; i < n; i++)
  {
    GEN P = Flx_Fl_mul(Flx_div_by_X_x(T, uel(L,i), p, NULL), uel(R,i), p);
    gel(M,i) = Flx_to_Flv(P, n-1);
  }
  return gerepilecopy(av, M);
}

/***********************************************************************/
/**                                                                   **/
/**                               Flxq                                **/
/**                                                                   **/
/***********************************************************************/
/* Flxq objects are defined as follows:
   They are Flx modulo another Flx called q.
*/

/* Product of y and x in Z/pZ[X]/(T), as t_VECSMALL. */
GEN
Flxq_mul(GEN x,GEN y,GEN T,ulong p)
{
  return Flx_rem(Flx_mul(x,y,p),T,p);
}

/* Square of y in Z/pZ[X]/(T), as t_VECSMALL. */
GEN
Flxq_sqr(GEN x,GEN T,ulong p)
{
  return Flx_rem(Flx_sqr(x,p),T,p);
}

static GEN
_Flxq_red(void *E, GEN x)
{ struct _Flxq *s = (struct _Flxq *)E;
  return Flx_rem(x, s->T, s->p); }
#if 0
static GEN
_Flx_sub(void *E, GEN x, GEN y)
{ struct _Flxq *s = (struct _Flxq *)E;
  return Flx_sub(x,y,s->p); }
#endif
static GEN
_Flxq_sqr(void *data, GEN x)
{
  struct _Flxq *D = (struct _Flxq*)data;
  return Flxq_sqr(x, D->T, D->p);
}
static GEN
_Flxq_mul(void *data, GEN x, GEN y)
{
  struct _Flxq *D = (struct _Flxq*)data;
  return Flxq_mul(x,y, D->T, D->p);
}
static GEN
_Flxq_one(void *data)
{
  struct _Flxq *D = (struct _Flxq*)data;
  return pol1_Flx(get_Flx_var(D->T));
}
#if 0
static GEN
_Flxq_zero(void *data)
{
  struct _Flxq *D = (struct _Flxq*)data;
  return pol0_Flx(get_Flx_var(D->T));
}
static GEN
_Flxq_cmul(void *data, GEN P, long a, GEN x)
{
  struct _Flxq *D = (struct _Flxq*)data;
  return Flx_Fl_mul(x, P[a+2], D->p);
}
#endif

/* n-Power of x in Z/pZ[X]/(T), as t_VECSMALL. */
GEN
Flxq_powu(GEN x, ulong n, GEN T, ulong p)
{
  pari_sp av = avma;
  struct _Flxq D;
  GEN y;
  switch(n)
  {
    case 0: return pol1_Flx(get_Flx_var(T));
    case 1: return Flx_copy(x);
    case 2: return Flxq_sqr(x, T, p);
  }
  D.T = Flx_get_red(T, p); D.p = p;
  y = gen_powu_i(x, n, (void*)&D, &_Flxq_sqr, &_Flxq_mul);
  return gerepileuptoleaf(av, y);
}

/* n-Power of x in Z/pZ[X]/(T), as t_VECSMALL. */
GEN
Flxq_pow(GEN x, GEN n, GEN T, ulong p)
{
  pari_sp av = avma;
  struct _Flxq D;
  GEN y;
  long s = signe(n);
  if (!s) return pol1_Flx(get_Flx_var(T));
  if (s < 0)
    x = Flxq_inv(x,T,p);
  if (is_pm1(n)) return s < 0 ? x : Flx_copy(x);
  D.T = Flx_get_red(T, p); D.p = p;
  y = gen_pow_i(x, n, (void*)&D, &_Flxq_sqr, &_Flxq_mul);
  return gerepileuptoleaf(av, y);
}

GEN
Flxq_pow_init(GEN x, GEN n, long k,  GEN T, ulong p)
{
  struct _Flxq D;
  D.T = Flx_get_red(T, p); D.p = p;
  return gen_pow_init(x, n, k, (void*)&D, &_Flxq_sqr, &_Flxq_mul);
}

GEN
Flxq_pow_table(GEN R, GEN n, GEN T, ulong p)
{
  struct _Flxq D;
  D.T = Flx_get_red(T, p); D.p = p;
  return gen_pow_table(R, n, (void*)&D, &_Flxq_one, &_Flxq_mul);
}

/* Inverse of x in Z/lZ[X]/(T) or NULL if inverse doesn't exist
 * not stack clean.
 */
GEN
Flxq_invsafe(GEN x, GEN T, ulong p)
{
  GEN V, z = Flx_extgcd(get_Flx_mod(T), x, p, NULL, &V);
  ulong iz;
  if (degpol(z)) return NULL;
  iz = Fl_inv (uel(z,2), p);
  return Flx_Fl_mul(V, iz, p);
}

GEN
Flxq_inv(GEN x,GEN T,ulong p)
{
  pari_sp av=avma;
  GEN U = Flxq_invsafe(x, T, p);
  if (!U) pari_err_INV("Flxq_inv",Flx_to_ZX(x));
  return gerepileuptoleaf(av, U);
}

GEN
Flxq_div(GEN x,GEN y,GEN T,ulong p)
{
  pari_sp av = avma;
  return gerepileuptoleaf(av, Flxq_mul(x,Flxq_inv(y,T,p),T,p));
}

GEN
Flxq_powers(GEN x, long l, GEN T, ulong p)
{
  struct _Flxq D;
  int use_sqr = 2*degpol(x) >= get_Flx_degree(T);
  D.T = Flx_get_red(T, p); D.p = p;
  return gen_powers(x, l, use_sqr, (void*)&D, &_Flxq_sqr, &_Flxq_mul, &_Flxq_one);
}

GEN
Flxq_matrix_pow(GEN y, long n, long m, GEN P, ulong l)
{
  return FlxV_to_Flm(Flxq_powers(y,m-1,P,l),n);
}

GEN
Flx_Frobenius(GEN T, ulong p)
{
  return Flxq_powu(polx_Flx(get_Flx_var(T)), p, T, p);
}

GEN
Flx_matFrobenius(GEN T, ulong p)
{
  long n = get_Flx_degree(T);
  return Flxq_matrix_pow(Flx_Frobenius(T, p), n, n, T, p);
}

static GEN
Flx_blocks_Flm(GEN P, long n, long m)
{
  GEN z = cgetg(m+1,t_MAT);
  long i,j, k=2, l = lg(P);
  for(i=1; i<=m; i++)
  {
    GEN zi = cgetg(n+1,t_VECSMALL);
    gel(z,i) = zi;
    for(j=1; j<=n; j++)
      uel(zi, j) = k==l ? 0 : uel(P,k++);
  }
  return z;
}

GEN
Flx_blocks(GEN P, long n, long m)
{
  GEN z = cgetg(m+1,t_VEC);
  long i,j, k=2, l = lg(P);
  for(i=1; i<=m; i++)
  {
    GEN zi = cgetg(n+2,t_VECSMALL);
    zi[1] = P[1];
    gel(z,i) = zi;
    for(j=2; j<n+2; j++)
      uel(zi, j) = k==l ? 0 : uel(P,k++);
    zi = Flx_renormalize(zi, n+2);
  }
  return z;
}

static GEN
FlxV_to_Flm_lg(GEN x, long m, long n)
{
  long i;
  GEN y = cgetg(n+1, t_MAT);
  for (i=1; i<=n; i++) gel(y,i) = Flx_to_Flv(gel(x,i), m);
  return y;
}

GEN
Flx_FlxqV_eval(GEN Q, GEN x, GEN T, ulong p)
{
  pari_sp btop, av = avma;
  long sv = get_Flx_var(T), m = get_Flx_degree(T);
  long i, l = lg(x)-1, lQ = lgpol(Q), n,  d;
  GEN A, B, C, S, g;
  if (lQ == 0) return pol0_Flx(sv);
  if (lQ <= l)
  {
    n = l;
    d = 1;
  }
  else
  {
    n = l-1;
    d = (lQ+n-1)/n;
  }
  A = FlxV_to_Flm_lg(x, m, n);
  B = Flx_blocks_Flm(Q, n, d);
  C = gerepileupto(av, Flm_mul(A, B, p));
  g = gel(x, l);
  T = Flx_get_red(T, p);
  btop = avma;
  S = Flv_to_Flx(gel(C, d), sv);
  for (i = d-1; i>0; i--)
  {
    S = Flx_add(Flxq_mul(S, g, T, p), Flv_to_Flx(gel(C,i), sv), p);
    if (gc_needed(btop,1))
      S = gerepileuptoleaf(btop, S);
  }
  return gerepileuptoleaf(av, S);
}

GEN
Flx_Flxq_eval(GEN Q, GEN x, GEN T, ulong p)
{
  pari_sp av = avma;
  GEN z, V;
  long d = degpol(Q), rtd;
  if (d < 0) return pol0_Flx(get_Flx_var(T));
  rtd = (long) sqrt((double)d);
  T = Flx_get_red(T, p);
  V = Flxq_powers(x, rtd, T, p);
  z = Flx_FlxqV_eval(Q, V, T, p);
  return gerepileupto(av, z);
}

GEN
FlxC_FlxqV_eval(GEN x, GEN v, GEN T, ulong p)
{ pari_APPLY_type(t_COL, Flx_FlxqV_eval(gel(x,i), v, T, p))
}

GEN
FlxC_Flxq_eval(GEN x, GEN F, GEN T, ulong p)
{
  long d = brent_kung_optpow(degpol(T)-1,lg(x)-1,1);
  GEN Fp = Flxq_powers(F, d, T, p);
  return FlxC_FlxqV_eval(x, Fp, T, p);
}

#if 0
static struct bb_algebra Flxq_algebra = { _Flxq_red, _Flx_add, _Flx_sub,
              _Flxq_mul, _Flxq_sqr, _Flxq_one, _Flxq_zero};
#endif

static GEN
Flxq_autpow_sqr(void *E, GEN x)
{
  struct _Flxq *D = (struct _Flxq*)E;
  return Flx_Flxq_eval(x, x, D->T, D->p);
}
static GEN
Flxq_autpow_msqr(void *E, GEN x)
{
  struct _Flxq *D = (struct _Flxq*)E;
  return Flx_FlxqV_eval(Flxq_autpow_sqr(E, x), D->aut, D->T, D->p);
}

GEN
Flxq_autpow(GEN x, ulong n, GEN T, ulong p)
{
  pari_sp av = avma;
  struct _Flxq D;
  long d;
  if (n==0) return Flx_rem(polx_Flx(x[1]), T, p);
  if (n==1) return Flx_rem(x, T, p);
  D.T = Flx_get_red(T, p); D.p = p;
  d = brent_kung_optpow(get_Flx_degree(T), hammingl(n)-1, 1);
  D.aut = Flxq_powers(x, d, T, p);
  x = gen_powu_fold_i(x,n,(void*)&D,Flxq_autpow_sqr,Flxq_autpow_msqr);
  return gerepilecopy(av, x);
}

GEN
Flxq_autpowers(GEN x, ulong l, GEN T, ulong p)
{
  long d, vT = get_Flx_var(T), dT = get_Flx_degree(T);
  ulong i;
  pari_sp av = avma;
  GEN xp, V = cgetg(l+2,t_VEC);
  gel(V,1) = polx_Flx(vT); if (l==0) return V;
  gel(V,2) = gcopy(x); if (l==1) return V;
  T = Flx_get_red(T, p);
  d = brent_kung_optpow(dT-1, l-1, 1);
  xp = Flxq_powers(x, d, T, p);
  for(i = 3; i < l+2; i++)
    gel(V,i) = Flx_FlxqV_eval(gel(V,i-1), xp, T, p);
  return gerepilecopy(av, V);
}

static GEN
Flxq_autsum_mul(void *E, GEN x, GEN y)
{
  struct _Flxq *D = (struct _Flxq*)E;
  GEN T = D->T;
  ulong p = D->p;
  GEN phi1 = gel(x,1), a1 = gel(x,2);
  GEN phi2 = gel(y,1), a2 = gel(y,2);
  ulong d = brent_kung_optpow(maxss(degpol(phi1),degpol(a1)),2,1);
  GEN V2 = Flxq_powers(phi2, d, T, p);
  GEN phi3 = Flx_FlxqV_eval(phi1, V2, T, p);
  GEN aphi = Flx_FlxqV_eval(a1, V2, T, p);
  GEN a3 = Flxq_mul(aphi, a2, T, p);
  return mkvec2(phi3, a3);
}
static GEN
Flxq_autsum_sqr(void *E, GEN x)
{ return Flxq_autsum_mul(E, x, x); }

GEN
Flxq_autsum(GEN x, ulong n, GEN T, ulong p)
{
  pari_sp av = avma;
  struct _Flxq D;
  D.T = Flx_get_red(T, p); D.p = p;
  x = gen_powu_i(x,n,(void*)&D,Flxq_autsum_sqr,Flxq_autsum_mul);
  return gerepilecopy(av, x);
}

static GEN
Flxq_auttrace_mul(void *E, GEN x, GEN y)
{
  struct _Flxq *D = (struct _Flxq*)E;
  GEN T = D->T;
  ulong p = D->p;
  GEN phi1 = gel(x,1), a1 = gel(x,2);
  GEN phi2 = gel(y,1), a2 = gel(y,2);
  ulong d = brent_kung_optpow(maxss(degpol(phi1),degpol(a1)),2,1);
  GEN V1 = Flxq_powers(phi1, d, T, p);
  GEN phi3 = Flx_FlxqV_eval(phi2, V1, T, p);
  GEN aphi = Flx_FlxqV_eval(a2, V1, T, p);
  GEN a3 = Flx_add(a1, aphi, p);
  return mkvec2(phi3, a3);
}

static GEN
Flxq_auttrace_sqr(void *E, GEN x)
{ return Flxq_auttrace_mul(E, x, x); }

GEN
Flxq_auttrace(GEN x, ulong n, GEN T, ulong p)
{
  pari_sp av = avma;
  struct _Flxq D;
  D.T = Flx_get_red(T, p); D.p = p;
  x = gen_powu_i(x,n,(void*)&D,Flxq_auttrace_sqr,Flxq_auttrace_mul);
  return gerepilecopy(av, x);
}

static long
bounded_order(ulong p, GEN b, long k)
{
  long i;
  GEN a=modii(utoi(p),b);
  for(i=1;i<k;i++)
  {
    if (equali1(a))
      return i;
    a = modii(muliu(a,p),b);
  }
  return 0;
}

/*
  n = (p^d-a)\b
  b = bb*p^vb
  p^k = 1 [bb]
  d = m*k+r+vb
  u = (p^k-1)/bb;
  v = (p^(r+vb)-a)/b;
  w = (p^(m*k)-1)/(p^k-1)
  n = p^r*w*u+v
  w*u = p^vb*(p^(m*k)-1)/b
  n = p^(r+vb)*(p^(m*k)-1)/b+(p^(r+vb)-a)/b
*/

static GEN
Flxq_pow_Frobenius(GEN x, GEN n, GEN aut, GEN T, ulong p)
{
  pari_sp av=avma;
  long d = get_Flx_degree(T);
  GEN an = absi_shallow(n), z, q;
  if (abscmpiu(an,p)<0 || cmpis(an,d)<=0) return Flxq_pow(x, n, T, p);
  q = powuu(p, d);
  if (dvdii(q, n))
  {
    long vn = logint(an,utoi(p));
    GEN autvn = vn==1 ? aut: Flxq_autpow(aut,vn,T,p);
    z = Flx_Flxq_eval(x,autvn,T,p);
  } else
  {
    GEN b = diviiround(q, an), a = subii(q, mulii(an,b));
    GEN bb, u, v, autk;
    long vb = Z_lvalrem(b,p,&bb);
    long m, r, k = is_pm1(bb) ? 1 : bounded_order(p,bb,d);
    if (!k || d-vb<k) return Flxq_pow(x,n, T, p);
    m = (d-vb)/k; r = (d-vb)%k;
    u = diviiexact(subiu(powuu(p,k),1),bb);
    v = diviiexact(subii(powuu(p,r+vb),a),b);
    autk = k==1 ? aut: Flxq_autpow(aut,k,T,p);
    if (r)
    {
      GEN autr = r==1 ? aut: Flxq_autpow(aut,r,T,p);
      z = Flx_Flxq_eval(x,autr,T,p);
    } else z = x;
    if (m > 1) z = gel(Flxq_autsum(mkvec2(autk, z), m, T, p), 2);
    if (!is_pm1(u)) z = Flxq_pow(z, u, T, p);
    if (signe(v)) z = Flxq_mul(z, Flxq_pow(x, v, T, p), T, p);
  }
  return gerepileupto(av,signe(n)>0 ? z : Flxq_inv(z,T,p));
}

static GEN
_Flxq_pow(void *data, GEN x, GEN n)
{
  struct _Flxq *D = (struct _Flxq*)data;
  return Flxq_pow_Frobenius(x, n, D->aut, D->T, D->p);
}

static GEN
_Flxq_rand(void *data)
{
  pari_sp av=avma;
  struct _Flxq *D = (struct _Flxq*)data;
  GEN z;
  do
  {
    set_avma(av);
    z = random_Flx(get_Flx_degree(D->T),get_Flx_var(D->T),D->p);
  } while (lgpol(z)==0);
  return z;
}

/* discrete log in FpXQ for a in Fp^*, g in FpXQ^* of order ord */
static GEN
Fl_Flxq_log(ulong a, GEN g, GEN o, GEN T, ulong p)
{
  pari_sp av = avma;
  GEN q,n_q,ord,ordp, op;

  if (a == 1UL) return gen_0;
  /* p > 2 */

  ordp = utoi(p - 1);
  ord  = get_arith_Z(o);
  if (!ord) ord = T? subiu(powuu(p, get_FpX_degree(T)), 1): ordp;
  if (a == p - 1) /* -1 */
    return gerepileuptoint(av, shifti(ord,-1));
  ordp = gcdii(ordp, ord);
  op = typ(o)==t_MAT ? famat_Z_gcd(o, ordp) : ordp;

  q = NULL;
  if (T)
  { /* we want < g > = Fp^* */
    if (!equalii(ord,ordp)) {
      q = diviiexact(ord,ordp);
      g = Flxq_pow(g,q,T,p);
    }
  }
  n_q = Fp_log(utoi(a), utoi(uel(g,2)), op, utoi(p));
  if (lg(n_q)==1) return gerepileuptoleaf(av, n_q);
  if (q) n_q = mulii(q, n_q);
  return gerepileuptoint(av, n_q);
}

static GEN
Flxq_easylog(void* E, GEN a, GEN g, GEN ord)
{
  struct _Flxq *f = (struct _Flxq *)E;
  GEN T = f->T;
  ulong p = f->p;
  long d = get_Flx_degree(T);
  if (Flx_equal1(a)) return gen_0;
  if (Flx_equal(a,g)) return gen_1;
  if (!degpol(a))
    return Fl_Flxq_log(uel(a,2), g, ord, T, p);
  if (typ(ord)!=t_INT || d <= 4 || d == 6 || abscmpiu(ord,1UL<<27)<0)
    return NULL;
  return Flxq_log_index(a, g, ord, T, p);
}

static const struct bb_group Flxq_star={_Flxq_mul,_Flxq_pow,_Flxq_rand,hash_GEN,Flx_equal,Flx_equal1,Flxq_easylog};

const struct bb_group *
get_Flxq_star(void **E, GEN T, ulong p)
{
  struct _Flxq *e = (struct _Flxq *) stack_malloc(sizeof(struct _Flxq));
  e->T = T; e->p  = p; e->aut =  Flx_Frobenius(T, p);
  *E = (void*)e; return &Flxq_star;
}

GEN
Flxq_order(GEN a, GEN ord, GEN T, ulong p)
{
  void *E;
  const struct bb_group *S = get_Flxq_star(&E,T,p);
  return gen_order(a,ord,E,S);
}

GEN
Flxq_log(GEN a, GEN g, GEN ord, GEN T, ulong p)
{
  void *E;
  pari_sp av = avma;
  const struct bb_group *S = get_Flxq_star(&E,T,p);
  GEN v = get_arith_ZZM(ord), F = gmael(v,2,1);
  if (Flxq_log_use_index(gel(F,lg(F)-1), T, p))
    v = mkvec2(gel(v, 1), ZM_famat_limit(gel(v, 2), int2n(27)));
  return gerepileuptoleaf(av, gen_PH_log(a, g, v, E, S));
}

GEN
Flxq_sqrtn(GEN a, GEN n, GEN T, ulong p, GEN *zeta)
{
  if (!lgpol(a))
  {
    if (signe(n) < 0) pari_err_INV("Flxq_sqrtn",a);
    if (zeta)
      *zeta=pol1_Flx(get_Flx_var(T));
    return pol0_Flx(get_Flx_var(T));
  }
  else
  {
    void *E;
    pari_sp av = avma;
    const struct bb_group *S = get_Flxq_star(&E,T,p);
    GEN o = subiu(powuu(p,get_Flx_degree(T)), 1);
    GEN s = gen_Shanks_sqrtn(a,n,o,zeta,E,S);
    if (s) gerepileall(av, zeta?2:1, &s, zeta);
    return s;
  }
}

GEN
Flxq_sqrt(GEN a, GEN T, ulong p)
{
  return Flxq_sqrtn(a, gen_2, T, p, NULL);
}

/* assume T irreducible mod p */
int
Flxq_issquare(GEN x, GEN T, ulong p)
{
  if (lgpol(x) == 0 || p == 2) return 1;
  return krouu(Flxq_norm(x,T,p), p) == 1;
}

/* assume T irreducible mod p */
int
Flxq_is2npower(GEN x, long n, GEN T, ulong p)
{
  pari_sp av;
  GEN m;
  if (n==1) return Flxq_issquare(x, T, p);
  if (lgpol(x) == 0 || p == 2) return 1;
  av = avma;
  m = shifti(subiu(powuu(p, get_Flx_degree(T)), 1), -n);
  return gc_bool(av, Flx_equal1(Flxq_pow(x, m, T, p)));
}

GEN
Flxq_lroot_fast(GEN a, GEN sqx, GEN T, long p)
{
  pari_sp av=avma;
  GEN A = Flx_splitting(a,p);
  return gerepileuptoleaf(av, FlxqV_dotproduct(A,sqx,T,p));
}

GEN
Flxq_lroot(GEN a, GEN T, long p)
{
  pari_sp av=avma;
  long n = get_Flx_degree(T), d = degpol(a);
  GEN sqx, V;
  if (n==1) return leafcopy(a);
  if (n==2) return Flxq_powu(a, p, T, p);
  sqx = Flxq_autpow(Flx_Frobenius(T, p), n-1, T, p);
  if (d==1 && a[2]==0 && a[3]==1) return gerepileuptoleaf(av, sqx);
  if (d>=p)
  {
    V = Flxq_powers(sqx,p-1,T,p);
    return gerepileuptoleaf(av, Flxq_lroot_fast(a,V,T,p));
  } else
    return gerepileuptoleaf(av, Flx_Flxq_eval(a,sqx,T,p));
}

ulong
Flxq_norm(GEN x, GEN TB, ulong p)
{
  GEN T = get_Flx_mod(TB);
  ulong y = Flx_resultant(T, x, p);
  ulong L = Flx_lead(T);
  if ( L==1 || lgpol(x)==0) return y;
  return Fl_div(y, Fl_powu(L, (ulong)degpol(x), p), p);
}

ulong
Flxq_trace(GEN x, GEN TB, ulong p)
{
  pari_sp av = avma;
  ulong t;
  GEN T = get_Flx_mod(TB);
  long n = degpol(T)-1;
  GEN z = Flxq_mul(x, Flx_deriv(T, p), TB, p);
  t = degpol(z)<n ? 0 : Fl_div(z[2+n],T[3+n],p);
  return gc_ulong(av, t);
}

/*x must be reduced*/
GEN
Flxq_charpoly(GEN x, GEN TB, ulong p)
{
  pari_sp ltop=avma;
  GEN T = get_Flx_mod(TB);
  long vs = evalvarn(fetch_var());
  GEN xm1 = deg1pol_shallow(pol1_Flx(x[1]),Flx_neg(x,p),vs);
  GEN r = Flx_FlxY_resultant(T, xm1, p);
  r[1] = x[1];
  (void)delete_var(); return gerepileupto(ltop, r);
}

/* Computing minimal polynomial :                         */
/* cf Shoup 'Efficient Computation of Minimal Polynomials */
/*          in Algebraic Extensions of Finite Fields'     */

/* Let v a linear form, return the linear form z->v(tau*z)
   that is, v*(M_tau) */

static GEN
Flxq_transmul_init(GEN tau, GEN T, ulong p)
{
  GEN bht;
  GEN h, Tp = get_Flx_red(T, &h);
  long n = degpol(Tp), vT = Tp[1];
  GEN ft = Flx_recipspec(Tp+2, n+1, n+1);
  GEN bt = Flx_recipspec(tau+2, lgpol(tau), n);
  ft[1] = vT; bt[1] = vT;
  if (h)
    bht = Flxn_mul(bt, h, n-1, p);
  else
  {
    GEN bh = Flx_div(Flx_shift(tau, n-1), T, p);
    bht = Flx_recipspec(bh+2, lgpol(bh), n-1);
    bht[1] = vT;
  }
  return mkvec3(bt, bht, ft);
}

static GEN
Flxq_transmul(GEN tau, GEN a, long n, ulong p)
{
  pari_sp ltop = avma;
  GEN t1, t2, t3, vec;
  GEN bt = gel(tau, 1), bht = gel(tau, 2), ft = gel(tau, 3);
  if (lgpol(a)==0) return pol0_Flx(a[1]);
  t2  = Flx_shift(Flx_mul(bt, a, p),1-n);
  if (lgpol(bht)==0) return gerepileuptoleaf(ltop, t2);
  t1  = Flx_shift(Flx_mul(ft, a, p),-n);
  t3  = Flxn_mul(t1, bht, n-1, p);
  vec = Flx_sub(t2, Flx_shift(t3, 1), p);
  return gerepileuptoleaf(ltop, vec);
}

GEN
Flxq_minpoly(GEN x, GEN T, ulong p)
{
  pari_sp ltop = avma;
  long vT = get_Flx_var(T), n = get_Flx_degree(T);
  GEN v_x;
  GEN g = pol1_Flx(vT), tau = pol1_Flx(vT);
  T = Flx_get_red(T, p);
  v_x = Flxq_powers(x, usqrt(2*n), T, p);
  while (lgpol(tau) != 0)
  {
    long i, j, m, k1;
    GEN M, v, tr;
    GEN g_prime, c;
    if (degpol(g) == n) { tau = pol1_Flx(vT); g = pol1_Flx(vT); }
    v = random_Flx(n, vT, p);
    tr = Flxq_transmul_init(tau, T, p);
    v = Flxq_transmul(tr, v, n, p);
    m = 2*(n-degpol(g));
    k1 = usqrt(m);
    tr = Flxq_transmul_init(gel(v_x,k1+1), T, p);
    c = cgetg(m+2,t_VECSMALL);
    c[1] = vT;
    for (i=0; i<m; i+=k1)
    {
      long mj = minss(m-i, k1);
      for (j=0; j<mj; j++)
        uel(c,m+1-(i+j)) = Flx_dotproduct(v, gel(v_x,j+1), p);
      v = Flxq_transmul(tr, v, n, p);
    }
    c = Flx_renormalize(c, m+2);
    /* now c contains <v,x^i> , i = 0..m-1  */
    M = Flx_halfgcd(monomial_Flx(1, m, vT), c, p);
    g_prime = gmael(M, 2, 2);
    if (degpol(g_prime) < 1) continue;
    g = Flx_mul(g, g_prime, p);
    tau = Flxq_mul(tau, Flx_FlxqV_eval(g_prime, v_x, T, p), T, p);
  }
  g = Flx_normalize(g,p);
  return gerepileuptoleaf(ltop,g);
}

GEN
Flxq_conjvec(GEN x, GEN T, ulong p)
{
  long i, l = 1+get_Flx_degree(T);
  GEN z = cgetg(l,t_COL);
  T = Flx_get_red(T,p);
  gel(z,1) = Flx_copy(x);
  for (i=2; i<l; i++) gel(z,i) = Flxq_powu(gel(z,i-1), p, T, p);
  return z;
}

GEN
gener_Flxq(GEN T, ulong p, GEN *po)
{
  long i, j, vT = get_Flx_var(T), f = get_Flx_degree(T);
  ulong p_1;
  GEN g, L, L2, o, q, F;
  pari_sp av0, av;

  if (f == 1) {
    GEN fa;
    o = utoipos(p-1);
    fa = Z_factor(o);
    L = gel(fa,1);
    L = vecslice(L, 2, lg(L)-1); /* remove 2 for efficiency */
    g = Fl_to_Flx(pgener_Fl_local(p, vec_to_vecsmall(L)), vT);
    if (po) *po = mkvec2(o, fa);
    return g;
  }

  av0 = avma; p_1 = p - 1;
  q = diviuexact(subiu(powuu(p,f), 1), p_1);

  L = cgetg(1, t_VECSMALL);
  if (p > 3)
  {
    ulong t = p_1 >> vals(p_1);
    GEN P = gel(factoru(t), 1);
    L = cgetg_copy(P, &i);
    while (--i) L[i] = p_1 / P[i];
  }
  o = factor_pn_1(utoipos(p),f);
  L2 = leafcopy( gel(o, 1) );
  for (i = j = 1; i < lg(L2); i++)
  {
    if (umodui(p_1, gel(L2,i)) == 0) continue;
    gel(L2,j++) = diviiexact(q, gel(L2,i));
  }
  setlg(L2, j);
  F = Flx_Frobenius(T, p);
  for (av = avma;; set_avma(av))
  {
    GEN tt;
    g = random_Flx(f, vT, p);
    if (degpol(g) < 1) continue;
    if (p == 2) tt = g;
    else
    {
      ulong t = Flxq_norm(g, T, p);
      if (t == 1 || !is_gener_Fl(t, p, p_1, L)) continue;
      tt = Flxq_powu(g, p_1>>1, T, p);
    }
    for (i = 1; i < j; i++)
    {
      GEN a = Flxq_pow_Frobenius(tt, gel(L2,i), F, T, p);
      if (!degpol(a) && uel(a,2) == p_1) break;
    }
    if (i == j) break;
  }
  if (!po)
  {
    set_avma((pari_sp)g);
    g = gerepileuptoleaf(av0, g);
  }
  else {
    *po = mkvec2(subiu(powuu(p,f), 1), o);
    gerepileall(av0, 2, &g, po);
  }
  return g;
}

static GEN
_Flxq_neg(void *E, GEN x)
{ struct _Flxq *s = (struct _Flxq *)E;
  return Flx_neg(x,s->p); }

static GEN
_Flxq_rmul(void *E, GEN x, GEN y)
{ struct _Flxq *s = (struct _Flxq *)E;
  return Flx_mul(x,y,s->p); }

static GEN
_Flxq_inv(void *E, GEN x)
{ struct _Flxq *s = (struct _Flxq *)E;
  return Flxq_inv(x,s->T,s->p); }

static int
_Flxq_equal0(GEN x) { return lgpol(x)==0; }

static GEN
_Flxq_s(void *E, long x)
{ struct _Flxq *s = (struct _Flxq *)E;
  ulong u = x<0 ? s->p+x: (ulong)x;
  return Fl_to_Flx(u, get_Flx_var(s->T));
}

static const struct bb_field Flxq_field={_Flxq_red,_Flx_add,_Flxq_rmul,_Flxq_neg,
                                         _Flxq_inv,_Flxq_equal0,_Flxq_s};

const struct bb_field *get_Flxq_field(void **E, GEN T, ulong p)
{
  GEN z = new_chunk(sizeof(struct _Flxq));
  struct _Flxq *e = (struct _Flxq *) z;
  e->T = Flx_get_red(T, p); e->p  = p; *E = (void*)e;
  return &Flxq_field;
}

/***********************************************************************/
/**                                                                   **/
/**                               Flxn                                **/
/**                                                                   **/
/***********************************************************************/

GEN
Flx_invLaplace(GEN x, ulong p)
{
  long i, d = degpol(x);
  ulong t;
  GEN y;
  if (d <= 1) return Flx_copy(x);
  t = Fl_inv(factorial_Fl(d, p), p);
  y = cgetg(d+3, t_VECSMALL);
  y[1] = x[1];
  for (i=d; i>=2; i--)
  {
    uel(y,i+2) = Fl_mul(uel(x,i+2), t, p);
    t = Fl_mul(t, i, p);
  }
  uel(y,3) = uel(x,3);
  uel(y,2) = uel(x,2);
  return y;
}

GEN
Flx_Laplace(GEN x, ulong p)
{
  long i, d = degpol(x);
  ulong t = 1;
  GEN y;
  if (d <= 1) return Flx_copy(x);
  y = cgetg(d+3, t_VECSMALL);
  y[1] = x[1];
  uel(y,2) = uel(x,2);
  uel(y,3) = uel(x,3);
  for (i=2; i<=d; i++)
  {
    t = Fl_mul(t, i%p, p);
    uel(y,i+2) = Fl_mul(uel(x,i+2), t, p);
  }
  return y;
}

GEN
Flxn_red(GEN a, long n)
{
  long i, L, l = lg(a);
  GEN  b;
  if (l == 2 || !n) return zero_Flx(a[1]);
  L = n+2; if (L > l) L = l;
  b = cgetg(L, t_VECSMALL); b[1] = a[1];
  for (i=2; i<L; i++) b[i] = a[i];
  return Flx_renormalize(b,L);
}

GEN
Flxn_mul(GEN a, GEN b, long n, ulong p)
{ return Flxn_red(Flx_mul(a, b, p), n); }

GEN
Flxn_sqr(GEN a, long n, ulong p)
{ return Flxn_red(Flx_sqr(a, p), n); }

/* (f*g) \/ x^n */
static GEN
Flx_mulhigh_i(GEN f, GEN g, long n, ulong p)
{
  return Flx_shift(Flx_mul(f,g, p),-n);
}

static GEN
Flxn_mulhigh(GEN f, GEN g, long n2, long n, ulong p)
{
  GEN F = Flx_blocks(f, n2, 2), fl = gel(F,1), fh = gel(F,2);
  return Flx_add(Flx_mulhigh_i(fl, g, n2, p), Flxn_mul(fh, g, n - n2, p), p);
}

GEN
Flxn_inv(GEN f, long e, ulong p)
{
  pari_sp av = avma, av2;
  ulong mask;
  GEN W;
  long n=1;
  if (lg(f)==2) pari_err_INV("Flxn_inv",f);
  W = Fl_to_Flx(Fl_inv(uel(f,2),p), f[1]);
  mask = quadratic_prec_mask(e);
  av2 = avma;
  for (;mask>1;)
  {
    GEN u, fr;
    long n2 = n;
    n<<=1; if (mask & 1) n--;
    mask >>= 1;
    fr = Flxn_red(f, n);
    u = Flxn_mul(W, Flxn_mulhigh(fr, W, n2, n, p), n-n2, p);
    W = Flx_sub(W, Flx_shift(u, n2), p);
    if (gc_needed(av2,2))
    {
      if(DEBUGMEM>1) pari_warn(warnmem,"Flxn_inv, e = %ld", n);
      W = gerepileupto(av2, W);
    }
  }
  return gerepileupto(av, W);
}

GEN
Flxn_expint(GEN h, long e, ulong p)
{
  pari_sp av = avma, av2;
  long v = h[1], n=1;
  GEN f = pol1_Flx(v), g = pol1_Flx(v);
  ulong mask = quadratic_prec_mask(e);
  av2 = avma;
  for (;mask>1;)
  {
    GEN u, w;
    long n2 = n;
    n<<=1; if (mask & 1) n--;
    mask >>= 1;
    u = Flxn_mul(g, Flx_mulhigh_i(f, Flxn_red(h, n2-1), n2-1, p), n-n2, p);
    u = Flx_add(u, Flx_shift(Flxn_red(h, n-1), 1-n2), p);
    w = Flxn_mul(f, Flx_integXn(u, n2-1, p), n-n2, p);
    f = Flx_add(f, Flx_shift(w, n2), p);
    if (mask<=1) break;
    u = Flxn_mul(g, Flxn_mulhigh(f, g, n2, n, p), n-n2, p);
    g = Flx_sub(g, Flx_shift(u, n2), p);
    if (gc_needed(av2,2))
    {
      if (DEBUGMEM>1) pari_warn(warnmem,"Flxn_exp, e = %ld", n);
      gerepileall(av2, 2, &f, &g);
    }
  }
  return gerepileupto(av, f);
}

GEN
Flxn_exp(GEN h, long e, ulong p)
{
  if (degpol(h)<1 || uel(h,2)!=0)
    pari_err_DOMAIN("Flxn_exp","valuation", "<", gen_1, h);
  return Flxn_expint(Flx_deriv(h, p), e, p);
}

INLINE GEN
Flxn_recip(GEN x, long n)
{
  GEN z=Flx_recipspec(x+2,lgpol(x),n);
  z[1]=x[1];
  return z;
}

GEN
Flx_Newton(GEN P, long n, ulong p)
{
  pari_sp av = avma;
  long d = degpol(P);
  GEN dP = Flxn_recip(Flx_deriv(P, p), d);
  GEN Q = Flxn_mul(Flxn_inv(Flxn_recip(P, d+1), n, p), dP, n, p);
  return gerepileuptoleaf(av, Q);
}

GEN
Flx_fromNewton(GEN P, ulong p)
{
  pari_sp av = avma;
  ulong n = Flx_constant(P)+1;
  GEN z = Flx_neg(Flx_shift(P, -1), p);
  GEN Q = Flxn_recip(Flxn_expint(z, n, p), n);
  return gerepileuptoleaf(av, Q);
}

static long
newtonlogint(ulong n, ulong pp)
{
  long s = 0;
  while (n > pp)
  {
    s += ulogint(n, pp);
    n = (n+1)>>1;
  }
  return s;
}

static void
init_invlaplace(long d, ulong p, GEN *pt_P, GEN *pt_V)
{
  long i;
  ulong e;
  GEN P = cgetg(d+1, t_VECSMALL);
  GEN V = cgetg(d+1, t_VECSMALL);
  for (i=1, e=1; i<=d; i++, e++)
  {
    if (e==p)
    {
      e = 0;
      V[i] = u_lvalrem(i, p, &uel(P,i));
    } else
    {
      V[i] = 0; uel(P,i) = i;
    }
  }
  *pt_P = P; *pt_V = V;
}

/* return p^val * FpX_invLaplace(1+x+...x^(n-1), q), with q a power of p and
 * val large enough to compensate for the power of p in the factorials */

static GEN
ZpX_invLaplace_init(long n, GEN q, ulong p, long v, long var)
{
  pari_sp av = avma;
  long i, d = n-1, w;
  GEN y, W, E, t;
  init_invlaplace(d, p, &E, &W);
  t = Fp_inv(FpV_prod(Flv_to_ZV(E), q), q);
  w = zv_sum(W);
  if (v > w) t = Fp_mul(t, powuu(p, v-w), q);
  y = cgetg(d+3,t_POL);
  y[1] = evalsigne(1) | evalvarn(var);
  for (i=d; i>=1; i--)
  {
    gel(y,i+2) = t;
    t = Fp_mulu(t, uel(E,i), q);
    if (uel(W,i)) t = Fp_mul(t, powuu(p, uel(W,i)), q);
  }
  gel(y,2) = t;
  return gerepilecopy(av, ZX_renormalize(y, d+3));
}

static GEN
Flx_composedsum(GEN P, GEN Q, ulong p)
{
  long n = 1 + degpol(P)*degpol(Q);
  ulong lead = Fl_mul(Fl_powu(Flx_lead(P), degpol(Q), p),
                      Fl_powu(Flx_lead(Q), degpol(P), p), p);
  GEN R;
  if (p >= (ulong)n)
  {
    GEN Pl = Flx_invLaplace(Flx_Newton(P,n,p), p);
    GEN Ql = Flx_invLaplace(Flx_Newton(Q,n,p), p);
    GEN L  = Flx_Laplace(Flxn_mul(Pl, Ql, n, p), p);
    R = Flx_fromNewton(L, p);
  } else
  {
    long v = factorial_lval(n-1, p);
    long w = 1 + newtonlogint(n-1, p);
    GEN pv = powuu(p, v);
    GEN qf = powuu(p, w), q = mulii(pv, qf), q2 = mulii(q, pv);
    GEN iL = ZpX_invLaplace_init(n, q, p, v, varn(P));
    GEN Pl = FpX_convol(iL, FpX_Newton(Flx_to_ZX(P), n, qf), q);
    GEN Ql = FpX_convol(iL, FpX_Newton(Flx_to_ZX(Q), n, qf), q);
    GEN Ln = ZX_Z_divexact(FpXn_mul(Pl, Ql, n, q2), pv);
    GEN L  = ZX_Z_divexact(FpX_Laplace(Ln, q), pv);
    R = ZX_to_Flx(FpX_fromNewton(L, qf), p);
  }
  return Flx_Fl_mul(R, lead, p);
}

GEN
Flx_direct_compositum(GEN a, GEN b, ulong p)
{
  return Flx_composedsum(a, b, p);
}

static GEN
_Flx_direct_compositum(void *E, GEN a, GEN b)
{ return Flx_direct_compositum(a, b, (ulong)E); }

GEN
FlxV_direct_compositum(GEN V, ulong p)
{
  return gen_product(V, (void *)p, &_Flx_direct_compositum);
}

/* (x+1)^n mod p; assume 2 <= n < 2p prime */
static GEN
Fl_Xp1_powu(ulong n, ulong p, long v)
{
  ulong k, d = (n + 1) >> 1;
  GEN C, V = identity_zv(d);

  Flv_inv_inplace(V, p); /* could restrict to odd integers in [3,d] */
  C = cgetg(n+3, t_VECSMALL);
  C[1] = v;
  uel(C,2) = 1UL;
  uel(C,3) = n%p;
  uel(C,4) = Fl_mul(odd(n)? n: n-1, n >> 1, p);
    /* binom(n,k) = binom(n,k-1) * (n-k+1) / k */
  if (SMALL_ULONG(p))
    for (k = 3; k <= d; k++)
      uel(C,k+2) = Fl_mul(Fl_mul(n-k+1, uel(C,k+1), p), uel(V,k), p);
  else
  {
    ulong pi  = get_Fl_red(p);
    for (k = 3; k <= d; k++)
      uel(C,k+2) = Fl_mul_pre(Fl_mul(n-k+1, uel(C,k+1), p), uel(V,k), p, pi);
  }
  for (   ; k <= n; k++) uel(C,2+k) = uel(C,2+n-k);
  return C; /* normalized */
}

/* p arbitrary */
GEN
Flx_translate1_basecase(GEN P, ulong p)
{
  GEN R = Flx_copy(P);
  long i, k, n = degpol(P);
  for (i = 1; i <= n; i++)
    for (k = n-i; k < n; k++) uel(R,k+2) = Fl_add(uel(R,k+2), uel(R,k+3), p);
  return R;
}

static int
translate_basecase(long n, ulong p)
{
#ifdef LONG_IS_64BIT
  if (p <= 19) return n < 40;
  if (p < 1UL<<30) return n < 58;
  if (p < 1UL<<59) return n < 100;
  if (p < 1UL<<62) return n < 120;
  if (p < 1UL<<63) return n < 240;
  return n < 250;
#else
  if (p <= 13) return n < 18;
  if (p <= 17) return n < 22;
  if (p <= 29) return n < 39;
  if (p <= 67) return n < 69;
  if (p < 1UL<< 15) return n < 80;
  if (p < 1UL<< 16) return n < 100;
  if (p < 1UL<< 28) return n < 300;
  return n < 650;
#endif
}
/* assume p prime */
GEN
Flx_translate1(GEN P, ulong p)
{
  long d, n = degpol(P);
  GEN R, Q, S;
  if (translate_basecase(n, p)) return Flx_translate1_basecase(P, p);
  /* n > 0 */
  d = n >> 1;
  if ((ulong)n < p)
  {
    R = Flx_translate1(Flxn_red(P, d), p);
    Q = Flx_translate1(Flx_shift(P, -d), p);
    S = Fl_Xp1_powu(d, p, P[1]);
    return Flx_add(Flx_mul(Q, S, p), R, p);
  }
  else
  {
    ulong q;
    if ((ulong)d > p) (void)ulogintall(d, p, &q); else q = p;
    R = Flx_translate1(Flxn_red(P, q), p);
    Q = Flx_translate1(Flx_shift(P, -q), p);
    S = Flx_add(Flx_shift(Q, q), Q, p);
    return Flx_add(S, R, p); /* P(x+1) = Q(x+1) (x^q+1) + R(x+1) */
  }
}

static GEN
zl_Xp1_powu(ulong n, ulong p, ulong q, long e, long vs)
{
  ulong k, d = n >> 1, c, v = 0;
  GEN C, V, W, U = upowers(p, e-1);
  init_invlaplace(d, p, &V, &W);
  Flv_inv_inplace(V, q);
  C = cgetg(n+3, t_VECSMALL);
  C[1] = vs;
  uel(C,2) = 1UL;
  uel(C,3) = n%q;
  v = u_lvalrem(n, p, &c);
  for (k = 2; k <= d; k++)
  {
    ulong w;
    v += u_lvalrem(n-k+1, p, &w) - W[k];
    c = Fl_mul(Fl_mul(w%q, c, q), uel(V,k), q);
    uel(C,2+k) = v >= (ulong)e ? 0: v==0 ? c : Fl_mul(c, uel(U, v+1), q);
  }
  for (   ; k <= n; k++) uel(C,2+k) = uel(C,2+n-k);
  return C; /* normalized */
}

GEN
zlx_translate1(GEN P, ulong p, long e)
{
  ulong d, q = upowuu(p,e), n = degpol(P);
  GEN R, Q, S;
  if (translate_basecase(n, q)) return Flx_translate1_basecase(P, q);
  /* n > 0 */
  d = n >> 1;
  R = zlx_translate1(Flxn_red(P, d), p, e);
  Q = zlx_translate1(Flx_shift(P, -d), p, e);
  S = zl_Xp1_powu(d, p, q, e, P[1]);
  return Flx_add(Flx_mul(Q, S, q), R, q);
}

/***********************************************************************/
/**                                                                   **/
/**                               Fl2                                 **/
/**                                                                   **/
/***********************************************************************/
/* Fl2 objects are Flv of length 2 [a,b] representing a+bsqrt(D) for
 * a nonsquare D. */

INLINE GEN
mkF2(ulong a, ulong b) { return mkvecsmall2(a,b); }

GEN
Fl2_mul_pre(GEN x, GEN y, ulong D, ulong p, ulong pi)
{
  ulong xaya, xbyb, Db2, mid;
  ulong z1, z2;
  ulong x1 = x[1], x2 = x[2], y1 = y[1], y2 = y[2];
  xaya = Fl_mul_pre(x1,y1,p,pi);
  if (x2==0 && y2==0) return mkF2(xaya,0);
  if (x2==0) return mkF2(xaya,Fl_mul_pre(x1,y2,p,pi));
  if (y2==0) return mkF2(xaya,Fl_mul_pre(x2,y1,p,pi));
  xbyb = Fl_mul_pre(x2,y2,p,pi);
  mid = Fl_mul_pre(Fl_add(x1,x2,p), Fl_add(y1,y2,p),p,pi);
  Db2 = Fl_mul_pre(D, xbyb, p,pi);
  z1 = Fl_add(xaya,Db2,p);
  z2 = Fl_sub(mid,Fl_add(xaya,xbyb,p),p);
  return mkF2(z1,z2);
}

GEN
Fl2_sqr_pre(GEN x, ulong D, ulong p, ulong pi)
{
  ulong a = x[1], b = x[2];
  ulong a2, Db2, ab;
  a2 = Fl_sqr_pre(a,p,pi);
  if (b==0) return mkF2(a2,0);
  Db2= Fl_mul_pre(D, Fl_sqr_pre(b,p,pi), p,pi);
  ab = Fl_mul_pre(a,b,p,pi);
  return mkF2(Fl_add(a2,Db2,p), Fl_double(ab,p));
}

ulong
Fl2_norm_pre(GEN x, ulong D, ulong p, ulong pi)
{
  ulong a2 = Fl_sqr_pre(x[1],p,pi);
  return x[2]? Fl_sub(a2, Fl_mul_pre(D, Fl_sqr_pre(x[2], p,pi), p,pi), p): a2;
}

GEN
Fl2_inv_pre(GEN x, ulong D, ulong p, ulong pi)
{
  ulong n, ni;
  if (x[2] == 0) return mkF2(Fl_inv(x[1],p),0);
  n = Fl_sub(Fl_sqr_pre(x[1], p,pi),
             Fl_mul_pre(D, Fl_sqr_pre(x[2], p,pi), p,pi), p);
  ni = Fl_inv(n,p);
  return mkF2(Fl_mul_pre(x[1], ni, p,pi),
               Fl_neg(Fl_mul_pre(x[2], ni, p,pi), p));
}

int
Fl2_equal1(GEN x) { return x[1]==1 && x[2]==0; }

struct _Fl2 {
  ulong p, pi, D;
};

static GEN
_Fl2_sqr(void *data, GEN x)
{
  struct _Fl2 *D = (struct _Fl2*)data;
  return Fl2_sqr_pre(x, D->D, D->p, D->pi);
}
static GEN
_Fl2_mul(void *data, GEN x, GEN y)
{
  struct _Fl2 *D = (struct _Fl2*)data;
  return Fl2_mul_pre(x,y, D->D, D->p, D->pi);
}

/* n-Power of x in Z/pZ[X]/(T), as t_VECSMALL. */
GEN
Fl2_pow_pre(GEN x, GEN n, ulong D, ulong p, ulong pi)
{
  pari_sp av = avma;
  struct _Fl2 d;
  GEN y;
  long s = signe(n);
  if (!s) return mkF2(1,0);
  if (s < 0)
    x = Fl2_inv_pre(x,D,p,pi);
  if (is_pm1(n)) return s < 0 ? x : zv_copy(x);
  d.p = p; d.pi = pi; d.D=D;
  y = gen_pow_i(x, n, (void*)&d, &_Fl2_sqr, &_Fl2_mul);
  return gerepileuptoleaf(av, y);
}

static GEN
_Fl2_pow(void *data, GEN x, GEN n)
{
  struct _Fl2 *D = (struct _Fl2*)data;
  return Fl2_pow_pre(x, n, D->D, D->p, D->pi);
}

static GEN
_Fl2_rand(void *data)
{
  struct _Fl2 *D = (struct _Fl2*)data;
  ulong a = random_Fl(D->p), b=random_Fl(D->p-1)+1;
  return mkF2(a,b);
}

static const struct bb_group Fl2_star={_Fl2_mul, _Fl2_pow, _Fl2_rand,
       hash_GEN, zv_equal, Fl2_equal1, NULL};

GEN
Fl2_sqrtn_pre(GEN a, GEN n, ulong D, ulong p, ulong pi, GEN *zeta)
{
  struct _Fl2 E;
  GEN o;
  if (a[1]==0 && a[2]==0)
  {
    if (signe(n) < 0) pari_err_INV("Flxq_sqrtn",a);
    if (zeta) *zeta=mkF2(1,0);
    return zv_copy(a);
  }
  E.p=p; E.pi = pi; E.D = D;
  o = subiu(powuu(p,2), 1);
  return gen_Shanks_sqrtn(a,n,o,zeta,(void*)&E,&Fl2_star);
}

GEN
Flx_Fl2_eval_pre(GEN x, GEN y, ulong D, ulong p, ulong pi)
{
  GEN p1;
  long i = lg(x)-1;
  if (i <= 2)
    return mkF2(i == 2? x[2]: 0, 0);
  p1 = mkF2(x[i], 0);
  for (i--; i>=2; i--)
  {
    p1 = Fl2_mul_pre(p1, y, D, p, pi);
    uel(p1,1) = Fl_add(uel(p1,1), uel(x,i), p);
  }
  return p1;
}

/***********************************************************************/
/**                                                                   **/
/**                               FlxV                                **/
/**                                                                   **/
/***********************************************************************/
/* FlxV are t_VEC with Flx coefficients. */

GEN
FlxV_Flc_mul(GEN V, GEN W, ulong p)
{
  pari_sp ltop=avma;
  long i;
  GEN z = Flx_Fl_mul(gel(V,1),W[1],p);
  for(i=2;i<lg(V);i++)
    z=Flx_add(z,Flx_Fl_mul(gel(V,i),W[i],p),p);
  return gerepileuptoleaf(ltop,z);
}

GEN
ZXV_to_FlxV(GEN x, ulong p)
{ pari_APPLY_type(t_VEC, ZX_to_Flx(gel(x,i), p)) }

GEN
ZXT_to_FlxT(GEN x, ulong p)
{
  if (typ(x) == t_POL)
    return ZX_to_Flx(x, p);
  else
    pari_APPLY_type(t_VEC, ZXT_to_FlxT(gel(x,i), p))
}

GEN
FlxV_to_Flm(GEN x, long n)
{ pari_APPLY_type(t_MAT, Flx_to_Flv(gel(x,i), n)) }

GEN
FlxV_red(GEN x, ulong p)
{ pari_APPLY_type(t_VEC, Flx_red(gel(x,i), p)) }

GEN
FlxT_red(GEN x, ulong p)
{
  if (typ(x) == t_VECSMALL)
    return Flx_red(x, p);
  else
    pari_APPLY_type(t_VEC, FlxT_red(gel(x,i), p))
}

GEN
FlxqV_dotproduct(GEN x, GEN y, GEN T, ulong p)
{
  long i, lx = lg(x);
  pari_sp av;
  GEN c;
  if (lx == 1) return pol0_Flx(get_Flx_var(T));
  av = avma; c = Flx_mul(gel(x,1),gel(y,1), p);
  for (i=2; i<lx; i++) c = Flx_add(c, Flx_mul(gel(x,i),gel(y,i), p), p);
  return gerepileuptoleaf(av, Flx_rem(c,T,p));
}

GEN
FlxqX_dotproduct(GEN x, GEN y, GEN T, ulong p)
{
  long i, l = minss(lg(x), lg(y));
  pari_sp av;
  GEN c;
  if (l == 2) return pol0_Flx(get_Flx_var(T));
  av = avma; c = Flx_mul(gel(x,2),gel(y,2), p);
  for (i=3; i<l; i++) c = Flx_add(c, Flx_mul(gel(x,i),gel(y,i), p), p);
  return gerepileuptoleaf(av, Flx_rem(c,T,p));
}

GEN
FlxC_eval_powers_pre(GEN z, GEN x, ulong p, ulong pi)
{
  long i, l = lg(z);
  GEN y = cgetg(l, t_VECSMALL);
  for (i=1; i<l; i++)
    uel(y,i) = Flx_eval_powers_pre(gel(z,i), x, p, pi);
  return y;
}

/***********************************************************************/
/**                                                                   **/
/**                               FlxM                                **/
/**                                                                   **/
/***********************************************************************/

GEN
FlxM_eval_powers_pre(GEN z, GEN x, ulong p, ulong pi)
{
  long i, l = lg(z);
  GEN y = cgetg(l, t_MAT);
  for (i=1; i<l; i++)
    gel(y,i) = FlxC_eval_powers_pre(gel(z,i), x, p, pi);
  return y;
}

GEN
zero_FlxC(long n, long sv)
{
  long i;
  GEN x = cgetg(n + 1, t_COL);
  GEN z = zero_Flx(sv);
  for (i = 1; i <= n; i++)
    gel(x, i) = z;
  return x;
}

GEN
FlxC_neg(GEN x, ulong p)
{ pari_APPLY_type(t_COL, Flx_neg(gel(x, i), p)) }

GEN
FlxC_sub(GEN x, GEN y, ulong p)
{ pari_APPLY_type(t_COL, Flx_sub(gel(x, i), gel(y, i), p)) }

GEN
zero_FlxM(long r, long c, long sv)
{
  long j;
  GEN x = cgetg(c + 1, t_MAT);
  GEN z = zero_FlxC(r, sv);
  for (j = 1; j <= c; j++)
    gel(x, j) = z;
  return x;
}

GEN
FlxM_neg(GEN x, ulong p)
{ pari_APPLY_same(FlxC_neg(gel(x, i), p)) }

GEN
FlxM_sub(GEN x, GEN y, ulong p)
{ pari_APPLY_same(FlxC_sub(gel(x, i), gel(y,i), p)) }

GEN
FlxqC_Flxq_mul(GEN x, GEN y, GEN T, ulong p)
{ pari_APPLY_type(t_COL, Flxq_mul(gel(x, i), y, T, p)) }

GEN
FlxqM_Flxq_mul(GEN x, GEN y, GEN T, ulong p)
{ pari_APPLY_same(FlxqC_Flxq_mul(gel(x, i), y, T, p)) }

static GEN
FlxM_pack_ZM(GEN M, GEN (*pack)(GEN, long)) {
  long i, j, l, lc;
  GEN N = cgetg_copy(M, &l), x;
  if (l == 1)
    return N;
  lc = lgcols(M);
  for (j = 1; j < l; j++) {
    gel(N, j) = cgetg(lc, t_COL);
    for (i = 1; i < lc; i++) {
      x = gcoeff(M, i, j);
      gcoeff(N, i, j) = pack(x + 2, lgpol(x));
    }
  }
  return N;
}

static GEN
kron_pack_Flx_spec_half(GEN x, long l) {
  if (l == 0)
    return gen_0;
  return Flx_to_int_halfspec(x, l);
}

static GEN
kron_pack_Flx_spec(GEN x, long l) {
  long i;
  GEN w, y;
  if (l == 0)
    return gen_0;
  y = cgetipos(l + 2);
  for (i = 0, w = int_LSW(y); i < l; i++, w = int_nextW(w))
    *w = x[i];
  return y;
}

static GEN
kron_pack_Flx_spec_2(GEN x, long l) {
  return Flx_eval2BILspec(x, 2, l);
}

static GEN
kron_pack_Flx_spec_3(GEN x, long l) {
  return Flx_eval2BILspec(x, 3, l);
}

static GEN
kron_unpack_Flx(GEN z, ulong p)
{
  long i, l = lgefint(z);
  GEN x = cgetg(l, t_VECSMALL), w;
  for (w = int_LSW(z), i = 2; i < l; w = int_nextW(w), i++)
    x[i] = ((ulong) *w) % p;
  return Flx_renormalize(x, l);
}

static GEN
kron_unpack_Flx_2(GEN x, ulong p) {
  long d = (lgefint(x)-1)/2 - 1;
  return Z_mod2BIL_Flx_2(x, d, p);
}

static GEN
kron_unpack_Flx_3(GEN x, ulong p) {
  long d = lgefint(x)/3 - 1;
  return Z_mod2BIL_Flx_3(x, d, p);
}

static GEN
FlxM_pack_ZM_bits(GEN M, long b)
{
  long i, j, l, lc;
  GEN N = cgetg_copy(M, &l), x;
  if (l == 1)
    return N;
  lc = lgcols(M);
  for (j = 1; j < l; j++) {
    gel(N, j) = cgetg(lc, t_COL);
    for (i = 1; i < lc; i++) {
      x = gcoeff(M, i, j);
      gcoeff(N, i, j) = kron_pack_Flx_spec_bits(x + 2, b, lgpol(x));
    }
  }
  return N;
}

static GEN
ZM_unpack_FlxqM(GEN M, GEN T, ulong p, GEN (*unpack)(GEN, ulong))
{
  long i, j, l, lc, sv = get_Flx_var(T);
  GEN N = cgetg_copy(M, &l), x;
  if (l == 1)
    return N;
  lc = lgcols(M);
  for (j = 1; j < l; j++) {
    gel(N, j) = cgetg(lc, t_COL);
    for (i = 1; i < lc; i++) {
      x = unpack(gcoeff(M, i, j), p);
      x[1] = sv;
      gcoeff(N, i, j) = Flx_rem(x, T, p);
    }
  }
  return N;
}

static GEN
ZM_unpack_FlxqM_bits(GEN M, long b, GEN T, ulong p)
{
  long i, j, l, lc, sv = get_Flx_var(T);
  GEN N = cgetg_copy(M, &l), x;
  if (l == 1)
    return N;
  lc = lgcols(M);
  if (b < BITS_IN_LONG) {
    for (j = 1; j < l; j++) {
      gel(N, j) = cgetg(lc, t_COL);
      for (i = 1; i < lc; i++) {
        x = kron_unpack_Flx_bits_narrow(gcoeff(M, i, j), b, p);
        x[1] = sv;
        gcoeff(N, i, j) = Flx_rem(x, T, p);
      }
    }
  } else {
    ulong pi = get_Fl_red(p);
    for (j = 1; j < l; j++) {
      gel(N, j) = cgetg(lc, t_COL);
      for (i = 1; i < lc; i++) {
        x = kron_unpack_Flx_bits_wide(gcoeff(M, i, j), b, p, pi);
        x[1] = sv;
        gcoeff(N, i, j) = Flx_rem(x, T, p);
      }
    }
  }
  return N;
}

GEN
FlxqM_mul_Kronecker(GEN A, GEN B, GEN T, ulong p)
{
  pari_sp av = avma;
  long b, d = get_Flx_degree(T), n = lg(A) - 1;
  GEN C, D, z;
  GEN (*pack)(GEN, long), (*unpack)(GEN, ulong);
  int is_sqr = A==B;

  z = muliu(muliu(sqru(p - 1), d), n);
  b = expi(z) + 1;
  /* only do expensive bit-packing if it saves at least 1 limb */
  if (b <= BITS_IN_HALFULONG) {
    if (nbits2nlong(d*b) == (d + 1)/2)
      b = BITS_IN_HALFULONG;
  } else {
    long l = lgefint(z) - 2;
    if (nbits2nlong(d*b) == d*l)
      b = l*BITS_IN_LONG;
  }
  set_avma(av);

  switch (b) {
  case BITS_IN_HALFULONG:
    pack = kron_pack_Flx_spec_half;
    unpack = int_to_Flx_half;
    break;
  case BITS_IN_LONG:
    pack = kron_pack_Flx_spec;
    unpack = kron_unpack_Flx;
    break;
  case 2*BITS_IN_LONG:
    pack = kron_pack_Flx_spec_2;
    unpack = kron_unpack_Flx_2;
    break;
  case 3*BITS_IN_LONG:
    pack = kron_pack_Flx_spec_3;
    unpack = kron_unpack_Flx_3;
    break;
  default:
    A = FlxM_pack_ZM_bits(A, b);
    B = is_sqr? A: FlxM_pack_ZM_bits(B, b);
    C = ZM_mul(A, B);
    D = ZM_unpack_FlxqM_bits(C, b, T, p);
    return gerepilecopy(av, D);
  }
  A = FlxM_pack_ZM(A, pack);
  B = is_sqr? A: FlxM_pack_ZM(B, pack);
  C = ZM_mul(A, B);
  D = ZM_unpack_FlxqM(C, T, p, unpack);
  return gerepilecopy(av, D);
}
