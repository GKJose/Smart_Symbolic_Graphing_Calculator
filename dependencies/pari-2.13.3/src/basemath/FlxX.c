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

#include "pari.h"
#include "paripriv.h"

/***********************************************************************/
/**                                                                   **/
/**                               FlxX                                **/
/**                                                                   **/
/***********************************************************************/

/* FlxX are t_POL with Flx coefficients.
 * Normally the variable ordering should be respected.*/

/*Similar to normalizepol, in place*/
/*FlxX_renormalize=zxX_renormalize */
GEN
FlxX_renormalize(GEN /*in place*/ x, long lx)
{
  long i;
  for (i = lx-1; i>1; i--)
    if (lgpol(gel(x,i))) break;
  stackdummy((pari_sp)(x + lg(x)), (pari_sp)(x + i+1));
  setlg(x, i+1); setsigne(x, i!=1); return x;
}

GEN
pol1_FlxX(long v, long sv)
{
  GEN z = cgetg(3, t_POL);
  z[1] = evalsigne(1) | evalvarn(v);
  gel(z,2) = pol1_Flx(sv); return z;
}

GEN
polx_FlxX(long v, long sv)
{
  GEN z = cgetg(4, t_POL);
  z[1] = evalsigne(1) | evalvarn(v);
  gel(z,2) = pol0_Flx(sv);
  gel(z,3) = pol1_Flx(sv); return z;
}

long
FlxY_degreex(GEN b)
{
  long deg = 0, i;
  if (!signe(b)) return -1;
  for (i = 2; i < lg(b); ++i)
    deg = maxss(deg, degpol(gel(b, i)));
  return deg;
}

/*Lift coefficient of B to constant Flx, to give a FlxY*/
GEN
Fly_to_FlxY(GEN B, long sv)
{
  long lb=lg(B);
  long i;
  GEN b=cgetg(lb,t_POL);
  b[1]=evalsigne(1)|(((ulong)B[1])&VARNBITS);
  for (i=2; i<lb; i++)
    gel(b,i) = Fl_to_Flx(B[i], sv);
  return FlxX_renormalize(b, lb);
}

GEN
zxX_to_FlxX(GEN B, ulong p)
{
  long i, lb = lg(B);
  GEN b = cgetg(lb,t_POL);
  for (i=2; i<lb; i++)
    gel(b,i) = zx_to_Flx(gel(B,i), p);
  b[1] = B[1]; return FlxX_renormalize(b, lb);
}

GEN
FlxX_to_ZXX(GEN B)
{
  long i, lb = lg(B);
  GEN b = cgetg(lb,t_POL);
  for (i=2; i<lb; i++)
  {
    GEN c = gel(B,i);
    switch(lgpol(c))
    {
      case 0:  c = gen_0; break;
      case 1:  c = utoi(c[2]); break;
      default: c = Flx_to_ZX(c); break;
    }
    gel(b,i) = c;
  }
  b[1] = B[1]; return b;
}

GEN
FlxXC_to_ZXXC(GEN x)
{ pari_APPLY_type(t_COL, FlxX_to_ZXX(gel(x,i))) }

GEN
FlxXM_to_ZXXM(GEN x)
{ pari_APPLY_same(FlxXC_to_ZXXC(gel(x,i))) }

/* Note: v is used _only_ for the t_INT. It must match
 * the variable of any t_POL coefficients. */
GEN
ZXX_to_FlxX(GEN B, ulong p, long v)
{
  long lb=lg(B);
  long i;
  GEN b=cgetg(lb,t_POL);
  b[1]=evalsigne(1)|(((ulong)B[1])&VARNBITS);
  for (i=2; i<lb; i++)
    switch (typ(gel(B,i)))
    {
    case t_INT:
      gel(b,i) = Z_to_Flx(gel(B,i), p, evalvarn(v));
      break;
    case t_POL:
      gel(b,i) = ZX_to_Flx(gel(B,i), p);
      break;
    }
  return FlxX_renormalize(b, lb);
}

GEN
ZXXV_to_FlxXV(GEN x, ulong p, long v)
{ pari_APPLY_type(t_VEC, ZXX_to_FlxX(gel(x,i), p, v)) }

GEN
ZXXT_to_FlxXT(GEN x, ulong p, long v)
{
  if (typ(x) == t_POL)
    return ZXX_to_FlxX(x, p, v);
  else
    pari_APPLY_type(t_VEC, ZXXT_to_FlxXT(gel(x,i), p, v))
}

GEN
FlxX_to_FlxC(GEN x, long N, long sv)
{
  long i, l;
  GEN z;
  l = lg(x)-1; x++;
  if (l > N+1) l = N+1; /* truncate higher degree terms */
  z = cgetg(N+1,t_COL);
  for (i=1; i<l ; i++) gel(z,i) = gel(x,i);
  for (   ; i<=N; i++) gel(z,i) = pol0_Flx(sv);
  return z;
}

static GEN
FlxXV_to_FlxM_lg(GEN x, long m, long n, long sv)
{
  long i;
  GEN y = cgetg(n+1, t_MAT);
  for (i=1; i<=n; i++) gel(y,i) = FlxX_to_FlxC(gel(x,i), m, sv);
  return y;
}

GEN
FlxXV_to_FlxM(GEN v, long n, long sv)
{ return FlxXV_to_FlxM_lg(v, n, lg(v)-1, sv); }

/* matrix whose entries are given by the coeffs of the polynomial v in
 * two variables (considered as degree n polynomials) */
GEN
FlxX_to_Flm(GEN v, long n)
{
  long j, N = lg(v)-1;
  GEN y = cgetg(N, t_MAT);
  v++;
  for (j=1; j<N; j++) gel(y,j) = Flx_to_Flv(gel(v,j), n);
  return y;
}

GEN
FlxX_to_Flx(GEN f)
{
  long i, l = lg(f);
  GEN V = cgetg(l, t_VECSMALL);
  V[1] = ((ulong)f[1])&VARNBITS;
  for(i=2; i<l; i++)
    V[i] = lgpol(gel(f,i)) ? mael(f,i,2): 0L;
  return V;
}

GEN
Flm_to_FlxX(GEN x, long v,long w)
{
  long j, lx = lg(x);
  GEN y = cgetg(lx+1, t_POL);
  y[1]=evalsigne(1) | v;
  y++;
  for (j=1; j<lx; j++) gel(y,j) = Flv_to_Flx(gel(x,j), w);
  return FlxX_renormalize(--y, lx+1);
}

/* P(X,Y) --> P(Y,X), n is the degree in Y */
GEN
FlxX_swap(GEN x, long n, long ws)
{
  long j, lx = lg(x), ly = n+3;
  GEN y = cgetg(ly, t_POL);
  y[1] = x[1];
  for (j=2; j<ly; j++)
  {
    long k;
    GEN p1 = cgetg(lx, t_VECSMALL);
    p1[1] = ws;
    for (k=2; k<lx; k++)
      if (j<lg(gel(x,k)))
        p1[k] = mael(x,k,j);
      else
        p1[k] = 0;
    gel(y,j) = Flx_renormalize(p1,lx);
  }
  return FlxX_renormalize(y,ly);
}

static GEN
zxX_to_Kronecker_spec(GEN P, long lp, long n)
{ /* P(X) = sum Pi(Y) * X^i, return P( Y^(2n-1) ) */
  long i, j, k, l, N = (n<<1) + 1;
  GEN y = cgetg((N-2)*lp + 2, t_VECSMALL) + 2;
  for (k=i=0; i<lp; i++)
  {
    GEN c = gel(P,i);
    l = lg(c);
    if (l-3 >= n)
      pari_err_BUG("zxX_to_Kronecker, P is not reduced mod Q");
    for (j=2; j < l; j++) y[k++] = c[j];
    if (i == lp-1) break;
    for (   ; j < N; j++) y[k++] = 0;
  }
  y -= 2;
  y[1] = 0; setlg(y, k+2); return y;
}

GEN
zxX_to_Kronecker(GEN P, GEN Q)
{
  GEN z = zxX_to_Kronecker_spec(P+2, lg(P)-2, degpol(Q));
  z[1] = P[1]; return z;
}

GEN
FlxX_add(GEN x, GEN y, ulong p)
{
  long i,lz;
  GEN z;
  long lx=lg(x);
  long ly=lg(y);
  if (ly>lx) swapspec(x,y, lx,ly);
  lz = lx; z = cgetg(lz, t_POL); z[1]=x[1];
  for (i=2; i<ly; i++) gel(z,i) = Flx_add(gel(x,i), gel(y,i), p);
  for (   ; i<lx; i++) gel(z,i) = Flx_copy(gel(x,i));
  return FlxX_renormalize(z, lz);
}

GEN
FlxX_Flx_add(GEN y, GEN x, ulong p)
{
  long i, lz = lg(y);
  GEN z;
  if (signe(y) == 0) return scalarpol(x, varn(y));
  z = cgetg(lz,t_POL); z[1] = y[1];
  gel(z,2) = Flx_add(gel(y,2), x, p);
  if (lz == 3) z = FlxX_renormalize(z,lz);
  else
    for(i=3;i<lz;i++) gel(z,i) = Flx_copy(gel(y,i));
  return z;
}

GEN
FlxX_Flx_sub(GEN y, GEN x, ulong p)
{
  long i, lz = lg(y);
  GEN z;
  if (signe(y) == 0) return scalarpol(x, varn(y));
  z = cgetg(lz,t_POL); z[1] = y[1];
  gel(z,2) = Flx_sub(gel(y,2), x, p);
  if (lz == 3) z = FlxX_renormalize(z,lz);
  else
    for(i=3;i<lz;i++) gel(z,i) = Flx_copy(gel(y,i));
  return z;
}

GEN
FlxX_neg(GEN x, ulong p)
{
  long i, lx=lg(x);
  GEN z = cgetg(lx, t_POL);
  z[1]=x[1];
  for (i=2; i<lx; i++) gel(z,i) = Flx_neg(gel(x,i), p);
  return z;
}

GEN
FlxX_Fl_mul(GEN x, ulong y, ulong p)
{
  long i, lx=lg(x);
  GEN z = cgetg(lx, t_POL);
  z[1]=x[1];
  for (i=2; i<lx; i++) gel(z,i) = Flx_Fl_mul(gel(x,i), y, p);
  return FlxX_renormalize(z, lx);
}

GEN
FlxX_triple(GEN x, ulong p)
{
  long i, lx=lg(x);
  GEN z = cgetg(lx, t_POL);
  z[1]=x[1];
  for (i=2; i<lx; i++) gel(z,i) = Flx_triple(gel(x,i), p);
  return FlxX_renormalize(z, lx);
}

GEN
FlxX_double(GEN x, ulong p)
{
  long i, lx=lg(x);
  GEN z = cgetg(lx, t_POL);
  z[1]=x[1];
  for (i=2; i<lx; i++) gel(z,i) = Flx_double(gel(x,i), p);
  return FlxX_renormalize(z, lx);
}

GEN
FlxX_deriv(GEN z, ulong p)
{
  long i,l = lg(z)-1;
  GEN x;
  if (l < 2) l = 2;
  x = cgetg(l, t_POL); x[1] = z[1];
  for (i=2; i<l; i++) gel(x,i) = Flx_mulu(gel(z,i+1), (ulong) i-1, p);
  return FlxX_renormalize(x,l);
}

GEN
FlxX_translate1(GEN P, long p, long n)
{
  GEN Q;
  long i, l, ws, lP = lgpol(P);
  if (!lP) return gcopy(P);
  ws = mael(P,2,1);
  Q = FlxX_swap(P, n, ws);
  l = lg(Q);
  for (i=2; i<l; i++) gel(Q, i) = Flx_translate1(gel(Q, i), p);
  return FlxX_swap(Q, lP, ws);
}

GEN
zlxX_translate1(GEN P, long p, long e, long n)
{
  GEN Q;
  long i, l, ws, lP = lgpol(P);
  if (!lP) return gcopy(P);
  ws = mael(P,2,1);
  Q = FlxX_swap(P, n, ws);
  l = lg(Q);
  for (i=2; i<l; i++) gel(Q, i) = zlx_translate1(gel(Q, i), p, e);
  return FlxX_swap(Q, lP, ws);
}

static GEN
FlxX_subspec(GEN x, GEN y, ulong p, long lx, long ly)
{
  long i,lz;
  GEN z;

  if (ly <= lx)
  {
    lz = lx+2; z = cgetg(lz, t_POL);
    for (i=0; i<ly; i++) gel(z,i+2) = Flx_sub(gel(x,i),gel(y,i),p);
    for (   ; i<lx; i++) gel(z,i+2) = Flx_copy(gel(x,i));
  }
  else
  {
    lz = ly+2; z = cgetg(lz, t_POL);
    for (i=0; i<lx; i++) gel(z,i+2) = Flx_sub(gel(x,i),gel(y,i),p);
    for (   ; i<ly; i++) gel(z,i+2) = Flx_neg(gel(y,i),p);
  }
  z[1] = 0; return FlxX_renormalize(z, lz);
}

GEN
FlxX_sub(GEN x, GEN y, ulong p)
{
  long lx,ly,i,lz;
  GEN z;
  lx = lg(x); ly = lg(y);
  lz=maxss(lx,ly);
  z = cgetg(lz,t_POL);
  if (lx >= ly)
  {
    z[1] = x[1];
    for (i=2; i<ly; i++) gel(z,i) = Flx_sub(gel(x,i),gel(y,i),p);
    for (   ; i<lx; i++) gel(z,i) = Flx_copy(gel(x,i));
    if (lx==ly) z = FlxX_renormalize(z, lz);
  }
  else
  {
    z[1] = y[1];
    for (i=2; i<lx; i++) gel(z,i) = Flx_sub(gel(x,i),gel(y,i),p);
    for (   ; i<ly; i++) gel(z,i) = Flx_neg(gel(y,i),p);
  }
  if (!lgpol(z)) { set_avma((pari_sp)(z + lz)); z = pol_0(varn(x)); }
  return z;
}

GEN
FlxX_Flx_mul(GEN P, GEN U, ulong p)
{
  long i, lP = lg(P);
  GEN res = cgetg(lP,t_POL);
  res[1] = P[1];
  for(i=2; i<lP; i++)
    gel(res,i) = Flx_mul(U,gel(P,i), p);
  return FlxX_renormalize(res, lP);
}

GEN
FlxY_evalx(GEN Q, ulong x, ulong p)
{
  GEN z;
  long i, lb = lg(Q);
  z = cgetg(lb,t_VECSMALL); z[1] = evalvarn(varn(Q));
  for (i=2; i<lb; i++) z[i] = Flx_eval(gel(Q,i), x, p);
  return Flx_renormalize(z, lb);
}

GEN
FlxY_Flx_translate(GEN P, GEN c, ulong p)
{
  pari_sp av = avma;
  GEN Q;
  long i, k, n;

  if (!signe(P) || gequal0(c)) return RgX_copy(P);
  Q = leafcopy(P); n = degpol(P);
  for (i=1; i<=n; i++)
  {
    for (k=n-i; k<n; k++)
      gel(Q,2+k) = Flx_add(gel(Q,2+k), Flx_mul(gel(Q,2+k+1), c, p), p);
    if (gc_needed(av,2))
    {
      if(DEBUGMEM>1)
        pari_warn(warnmem,"FlxY_Flx_translate, i = %ld/%ld", i,n);
      Q = gerepilecopy(av, Q);
    }
  }
  return gerepilecopy(av, Q);
}

GEN
FlxY_evalx_powers_pre(GEN pol, GEN ypowers, ulong p, ulong pi)
{
  long i, len = lg(pol);
  GEN res = cgetg(len, t_VECSMALL);
  res[1] = pol[1] & VARNBITS;
  for (i = 2; i < len; ++i)
    res[i] = Flx_eval_powers_pre(gel(pol, i), ypowers, p, pi);
  return Flx_renormalize(res, len);
}

ulong
FlxY_eval_powers_pre(GEN pol, GEN ypowers, GEN xpowers, ulong p, ulong pi)
{
  pari_sp av = avma;
  GEN t = FlxY_evalx_powers_pre(pol, ypowers, p, pi);
  return gc_ulong(av, Flx_eval_powers_pre(t, xpowers, p, pi));
}

GEN
FlxY_FlxqV_evalx(GEN P, GEN x, GEN T, ulong p)
{
  long i, lP = lg(P);
  GEN res = cgetg(lP,t_POL);
  res[1] = P[1];
  for(i=2; i<lP; i++)
    gel(res,i) = Flx_FlxqV_eval(gel(P,i), x, T, p);
  return FlxX_renormalize(res, lP);
}

GEN
FlxY_Flxq_evalx(GEN P, GEN x, GEN T, ulong p)
{
  pari_sp av = avma;
  long n = brent_kung_optpow(get_Flx_degree(T)-1,lgpol(P),1);
  GEN xp = Flxq_powers(x, n, T, p);
  return gerepileupto(av, FlxY_FlxqV_evalx(P, xp, T, p));
}

GEN
FlxY_Flx_div(GEN x, GEN y, ulong p)
{
  long i, l;
  GEN z;
  if (degpol(y) == 0)
  {
    ulong t = uel(y,2);
    if (t == 1) return x;
    t = Fl_inv(t, p);
    z = cgetg_copy(x, &l); z[1] = x[1];
    for (i=2; i<l; i++) gel(z,i) = Flx_Fl_mul(gel(x,i),t,p);
  }
  else
  {
    z = cgetg_copy(x, &l); z[1] = x[1];
    for (i=2; i<l; i++) gel(z,i) = Flx_div(gel(x,i),y,p);
  }
  return z;
}

GEN
FlxX_shift(GEN a, long n, long vs)
{
  long i, l = lg(a);
  GEN  b;
  if (l == 2 || !n) return a;
  l += n;
  if (n < 0)
  {
    if (l <= 2) return pol_0(varn(a));
    b = cgetg(l, t_POL); b[1] = a[1];
    a -= n;
    for (i=2; i<l; i++) gel(b,i) = gel(a,i);
  } else {
    b = cgetg(l, t_POL); b[1] = a[1];
    a -= n; n += 2;
    for (i=2; i<n; i++) gel(b,i) = pol0_Flx(vs);
    for (   ; i<l; i++) gel(b,i) = gel(a,i);
  }
  return b;
}

GEN
FlxX_blocks(GEN P, long n, long m, long vs)
{
  GEN z = cgetg(m+1,t_VEC);
  long i,j, k=2, l = lg(P);
  for(i=1; i<=m; i++)
  {
    GEN zi = cgetg(n+2,t_POL);
    zi[1] = P[1];
    gel(z,i) = zi;
    for(j=2; j<n+2; j++)
      gel(zi, j) = k==l ? pol0_Flx(vs) : gel(P,k++);
    zi = FlxX_renormalize(zi, n+2);
  }
  return z;
}

static GEN
FlxX_recipspec(GEN x, long l, long n, long vs)
{
  long i;
  GEN z = cgetg(n+2,t_POL);
  z[1] = 0; z += 2;
  for(i=0; i<l; i++)
    gel(z,n-i-1) = Flx_copy(gel(x,i));
  for(   ; i<n; i++)
    gel(z,n-i-1) = pol0_Flx(vs);
  return FlxX_renormalize(z-2,n+2);
}

GEN
FlxX_invLaplace(GEN x, ulong p)
{
  long i, d = degpol(x);
  GEN y;
  ulong t;
  if (d <= 1) return gcopy(x);
  t = Fl_inv(factorial_Fl(d, p), p);
  y = cgetg(d+3, t_POL);
  y[1] = x[1];
  for (i=d; i>=2; i--)
  {
    gel(y,i+2) = Flx_Fl_mul(gel(x,i+2), t, p);
    t = Fl_mul(t, i, p);
  }
  gel(y,3) = Flx_copy(gel(x,3));
  gel(y,2) = Flx_copy(gel(x,2));
  return FlxX_renormalize(y, d+3);
}

GEN
FlxX_Laplace(GEN x, ulong p)
{
  long i, d = degpol(x);
  ulong t = 1;
  GEN y;
  if (d <= 1) return gcopy(x);
  y = cgetg(d+3, t_POL);
  y[1] = x[1];
  gel(y,2) = Flx_copy(gel(x,2));
  gel(y,3) = Flx_copy(gel(x,3));
  for (i=2; i<=d; i++)
  {
    t = Fl_mul(t, i%p, p);
    gel(y,i+2) = Flx_Fl_mul(gel(x,i+2), t, p);
  }
  return FlxX_renormalize(y, d+3);
}

/***********************************************************************/
/**                                                                   **/
/**                               FlxqX                               **/
/**                                                                   **/
/***********************************************************************/

static GEN
get_FlxqX_red(GEN T, GEN *B)
{
  if (typ(T)!=t_VEC) { *B=NULL; return T; }
  *B = gel(T,1); return gel(T,2);
}

GEN
RgX_to_FlxqX(GEN x, GEN T, ulong p)
{
  long i, l = lg(x);
  GEN z = cgetg(l, t_POL); z[1] = x[1];
  for (i = 2; i < l; i++)
    gel(z,i) = Rg_to_Flxq(gel(x,i), T, p);
  return FlxX_renormalize(z, l);
}

/* FlxqX are t_POL with Flxq coefficients.
 * Normally the variable ordering should be respected.*/

GEN
random_FlxqX(long d1, long v, GEN T, ulong p)
{
  long dT = get_Flx_degree(T), vT = get_Flx_var(T);
  long i, d = d1+2;
  GEN y = cgetg(d,t_POL); y[1] = evalsigne(1) | evalvarn(v);
  for (i=2; i<d; i++) gel(y,i) = random_Flx(dT, vT, p);
  return FlxX_renormalize(y,d);
}

/*Not stack clean*/
GEN
Kronecker_to_FlxqX(GEN z, GEN T, ulong p)
{
  long i,j,lx,l, N = (get_Flx_degree(T)<<1) + 1;
  GEN x, t = cgetg(N,t_VECSMALL);
  t[1] = get_Flx_var(T);
  l = lg(z); lx = (l-2) / (N-2);
  x = cgetg(lx+3,t_POL);
  x[1] = z[1];
  for (i=2; i<lx+2; i++)
  {
    for (j=2; j<N; j++) t[j] = z[j];
    z += (N-2);
    gel(x,i) = Flx_rem(Flx_renormalize(t,N), T,p);
  }
  N = (l-2) % (N-2) + 2;
  for (j=2; j<N; j++) t[j] = z[j];
  gel(x,i) = Flx_rem(Flx_renormalize(t,N), T,p);
  return FlxX_renormalize(x, i+1);
}

GEN
FlxqX_red(GEN z, GEN T, ulong p)
{
  GEN res;
  long i, l = lg(z);
  res = cgetg(l,t_POL); res[1] = z[1];
  for(i=2;i<l;i++) gel(res,i) = Flx_rem(gel(z,i),T,p);
  return FlxX_renormalize(res,l);
}

static GEN
FlxqX_mulspec(GEN x, GEN y, GEN T, ulong p, long lx, long ly)
{
  pari_sp ltop=avma;
  GEN z,kx,ky;
  long dT =  get_Flx_degree(T);
  kx= zxX_to_Kronecker_spec(x,lx,dT);
  ky= zxX_to_Kronecker_spec(y,ly,dT);
  z = Flx_mul(ky, kx, p);
  z = Kronecker_to_FlxqX(z,T,p);
  return gerepileupto(ltop,z);
}

GEN
FlxqX_mul(GEN x, GEN y, GEN T, ulong p)
{
  pari_sp ltop=avma;
  GEN z, kx, ky, Tm = get_Flx_mod(T);
  kx= zxX_to_Kronecker(x, Tm);
  ky= zxX_to_Kronecker(y, Tm);
  z = Flx_mul(ky, kx, p);
  z = Kronecker_to_FlxqX(z, T, p);
  return gerepileupto(ltop, z);
}

GEN
FlxqX_sqr(GEN x, GEN T, ulong p)
{
  pari_sp ltop=avma;
  GEN z,kx;
  kx= zxX_to_Kronecker(x,get_Flx_mod(T));
  z = Flx_sqr(kx, p);
  z = Kronecker_to_FlxqX(z,T,p);
  return gerepileupto(ltop,z);
}

GEN
FlxqX_Flxq_mul(GEN P, GEN U, GEN T, ulong p)
{
  long i, lP = lg(P);
  GEN res = cgetg(lP,t_POL);
  res[1] = P[1];
  for(i=2; i<lP; i++)
    gel(res,i) = Flxq_mul(U,gel(P,i), T,p);
  return FlxX_renormalize(res, lP);
}
GEN
FlxqX_Flxq_mul_to_monic(GEN P, GEN U, GEN T, ulong p)
{
  long i, lP = lg(P);
  GEN res = cgetg(lP,t_POL);
  res[1] = P[1];
  for(i=2; i<lP-1; i++) gel(res,i) = Flxq_mul(U,gel(P,i), T,p);
  gel(res,lP-1) = pol1_Flx(get_Flx_var(T));
  return FlxX_renormalize(res, lP);
}

GEN
FlxqX_normalize(GEN z, GEN T, ulong p)
{
  GEN p1 = leading_coeff(z);
  if (!lgpol(z) || (!degpol(p1) && p1[1] == 1)) return z;
  return FlxqX_Flxq_mul_to_monic(z, Flxq_inv(p1,T,p), T,p);
}

/* x and y in Z[Y][X]. Assume T irreducible mod p */
static GEN
FlxqX_divrem_basecase(GEN x, GEN y, GEN T, ulong p, GEN *pr)
{
  long vx, dx, dy, dz, i, j, sx, lr;
  pari_sp av0, av, tetpil;
  GEN z,p1,rem,lead;

  if (!signe(y)) pari_err_INV("FlxqX_divrem",y);
  vx=varn(x); dy=degpol(y); dx=degpol(x);
  if (dx < dy)
  {
    if (pr)
    {
      av0 = avma; x = FlxqX_red(x, T, p);
      if (pr == ONLY_DIVIDES) { set_avma(av0); return signe(x)? NULL: pol_0(vx); }
      if (pr == ONLY_REM) return x;
      *pr = x;
    }
    return pol_0(vx);
  }
  lead = leading_coeff(y);
  if (!dy) /* y is constant */
  {
    if (pr && pr != ONLY_DIVIDES)
    {
      if (pr == ONLY_REM) return pol_0(vx);
      *pr = pol_0(vx);
    }
    if (Flx_equal1(lead)) return gcopy(x);
    av0 = avma; x = FlxqX_Flxq_mul(x,Flxq_inv(lead,T,p),T,p);
    return gerepileupto(av0,x);
  }
  av0 = avma; dz = dx-dy;
  lead = Flx_equal1(lead)? NULL: gclone(Flxq_inv(lead,T,p));
  set_avma(av0);
  z = cgetg(dz+3,t_POL); z[1] = x[1];
  x += 2; y += 2; z += 2;

  p1 = gel(x,dx); av = avma;
  gel(z,dz) = lead? gerepileupto(av, Flxq_mul(p1,lead, T, p)): gcopy(p1);
  for (i=dx-1; i>=dy; i--)
  {
    av=avma; p1=gel(x,i);
    for (j=i-dy+1; j<=i && j<=dz; j++)
      p1 = Flx_sub(p1, Flx_mul(gel(z,j),gel(y,i-j),p),p);
    if (lead) p1 = Flx_mul(p1, lead,p);
    tetpil=avma; gel(z,i-dy) = gerepile(av,tetpil,Flx_rem(p1,T,p));
  }
  if (!pr) { guncloneNULL(lead); return z-2; }

  rem = (GEN)avma; av = (pari_sp)new_chunk(dx+3);
  for (sx=0; ; i--)
  {
    p1 = gel(x,i);
    for (j=0; j<=i && j<=dz; j++)
      p1 = Flx_sub(p1, Flx_mul(gel(z,j),gel(y,i-j),p),p);
    tetpil=avma; p1 = Flx_rem(p1, T, p); if (lgpol(p1)) { sx = 1; break; }
    if (!i) break;
    set_avma(av);
  }
  if (pr == ONLY_DIVIDES)
  {
    guncloneNULL(lead);
    if (sx) return gc_NULL(av0);
    return gc_const((pari_sp)rem, z-2);
  }
  lr=i+3; rem -= lr;
  rem[0] = evaltyp(t_POL) | evallg(lr);
  rem[1] = z[-1];
  p1 = gerepile((pari_sp)rem,tetpil,p1);
  rem += 2; gel(rem,i) = p1;
  for (i--; i>=0; i--)
  {
    av=avma; p1 = gel(x,i);
    for (j=0; j<=i && j<=dz; j++)
      p1 = Flx_sub(p1, Flx_mul(gel(z,j),gel(y,i-j),p), p);
    tetpil=avma; gel(rem,i) = gerepile(av,tetpil, Flx_rem(p1, T, p));
  }
  rem -= 2;
  guncloneNULL(lead);
  if (!sx) (void)FlxX_renormalize(rem, lr);
  if (pr == ONLY_REM) return gerepileupto(av0,rem);
  *pr = rem; return z-2;
}

static GEN
FlxqX_invBarrett_basecase(GEN T, GEN Q, ulong p)
{
  long i, l=lg(T)-1, lr = l-1, k;
  long sv=Q[1];
  GEN r=cgetg(lr,t_POL); r[1]=T[1];
  gel(r,2) = pol1_Flx(sv);
  for (i=3;i<lr;i++)
  {
    pari_sp ltop=avma;
    GEN u = Flx_neg(gel(T,l-i+2),p);
    for (k=3;k<i;k++)
      u = Flx_sub(u, Flxq_mul(gel(T,l-i+k),gel(r,k),Q,p),p);
    gel(r,i) = gerepileupto(ltop, u);
  }
  r = FlxX_renormalize(r,lr);
  return r;
}

/* Return new lgpol */
static long
FlxX_lgrenormalizespec(GEN x, long lx)
{
  long i;
  for (i = lx-1; i>=0; i--)
    if (lgpol(gel(x,i))) break;
  return i+1;
}

static GEN
FlxqX_invBarrett_Newton(GEN S, GEN T, ulong p)
{
  pari_sp av = avma;
  long nold, lx, lz, lq, l = degpol(S), i, lQ;
  GEN q, y, z, x = cgetg(l+2, t_POL) + 2;
  long dT = get_Flx_degree(T), vT = get_Flx_var(T);
  ulong mask = quadratic_prec_mask(l-2); /* assume l > 2 */
  for (i=0;i<l;i++) gel(x,i) = pol0_Flx(vT);
  q = FlxX_recipspec(S+2,l+1,l+1,dT);
  lQ = lgpol(q); q+=2;
  /* We work on _spec_ FlxX's, all the l[xzq] below are lgpol's */

  /* initialize */
  gel(x,0) = Flxq_inv(gel(q,0),T, p);
  if (lQ>1 && degpol(gel(q,1)) >= dT)
    gel(q,1) = Flx_rem(gel(q,1), T, p);
  if (lQ>1 && lgpol(gel(q,1)))
  {
    GEN u = gel(q, 1);
    if (!Flx_equal1(gel(x,0))) u = Flxq_mul(u, Flxq_sqr(gel(x,0), T,p), T,p);
    gel(x,1) = Flx_neg(u, p); lx = 2;
  }
  else
    lx = 1;
  nold = 1;
  for (; mask > 1; )
  { /* set x -= x(x*q - 1) + O(t^(nnew + 1)), knowing x*q = 1 + O(t^(nold+1)) */
    long i, lnew, nnew = nold << 1;

    if (mask & 1) nnew--;
    mask >>= 1;

    lnew = nnew + 1;
    lq = FlxX_lgrenormalizespec(q, minss(lQ,lnew));
    z = FlxqX_mulspec(x, q, T, p, lx, lq); /* FIXME: high product */
    lz = lgpol(z); if (lz > lnew) lz = lnew;
    z += 2;
    /* subtract 1 [=>first nold words are 0]: renormalize so that z(0) != 0 */
    for (i = nold; i < lz; i++) if (lgpol(gel(z,i))) break;
    nold = nnew;
    if (i >= lz) continue; /* z-1 = 0(t^(nnew + 1)) */

    /* z + i represents (x*q - 1) / t^i */
    lz = FlxX_lgrenormalizespec (z+i, lz-i);
    z = FlxqX_mulspec(x, z+i, T,p, lx, lz); /* FIXME: low product */
    lz = lgpol(z); z += 2;
    if (lz > lnew-i) lz = FlxX_lgrenormalizespec(z, lnew-i);

    lx = lz+ i;
    y  = x + i; /* x -= z * t^i, in place */
    for (i = 0; i < lz; i++) gel(y,i) = Flx_neg(gel(z,i), p);
  }
  x -= 2; setlg(x, lx + 2); x[1] = S[1];
  return gerepilecopy(av, x);
}

GEN
FlxqX_invBarrett(GEN T, GEN Q, ulong p)
{
  pari_sp ltop=avma;
  long l=lg(T), v = varn(T);
  GEN r;
  GEN c = gel(T,l-1);
  if (l<5) return pol_0(v);
  if (l<=FlxqX_INVBARRETT_LIMIT)
  {
    if (!Flx_equal1(c))
    {
      GEN ci = Flxq_inv(c,Q,p);
      T = FlxqX_Flxq_mul(T, ci, Q, p);
      r = FlxqX_invBarrett_basecase(T,Q,p);
      r = FlxqX_Flxq_mul(r,ci,Q,p);
    } else
      r = FlxqX_invBarrett_basecase(T,Q,p);
  } else
    r = FlxqX_invBarrett_Newton(T,Q,p);
  return gerepileupto(ltop, r);
}

GEN
FlxqX_get_red(GEN S, GEN T, ulong p)
{
  if (typ(S)==t_POL && lg(S)>FlxqX_BARRETT_LIMIT)
    retmkvec2(FlxqX_invBarrett(S, T, p), S);
  return S;
}

/* Compute x mod S where 2 <= degpol(S) <= l+1 <= 2*(degpol(S)-1)
 *  * and mg is the Barrett inverse of S. */
static GEN
FlxqX_divrem_Barrettspec(GEN x, long l, GEN mg, GEN S, GEN T, ulong p, GEN *pr)
{
  GEN q, r;
  long lt = degpol(S); /*We discard the leading term*/
  long ld, lm, lT, lmg;
  ld = l-lt;
  lm = minss(ld, lgpol(mg));
  lT  = FlxX_lgrenormalizespec(S+2,lt);
  lmg = FlxX_lgrenormalizespec(mg+2,lm);
  q = FlxX_recipspec(x+lt,ld,ld,0);               /* q = rec(x)     lq<=ld*/
  q = FlxqX_mulspec(q+2,mg+2,T,p,lgpol(q),lmg);   /* q = rec(x) * mg lq<=ld+lm*/
  q = FlxX_recipspec(q+2,minss(ld,lgpol(q)),ld,0);/* q = rec (rec(x) * mg) lq<=ld*/
  if (!pr) return q;
  r = FlxqX_mulspec(q+2,S+2,T,p,lgpol(q),lT);     /* r = q*pol        lr<=ld+lt*/
  r = FlxX_subspec(x,r+2,p,lt,minss(lt,lgpol(r)));/* r = x - r   lr<=lt */
  if (pr == ONLY_REM) return r;
  *pr = r; return q;
}

static GEN
FlxqX_divrem_Barrett(GEN x, GEN mg, GEN S, GEN T, ulong p, GEN *pr)
{
  GEN q = NULL, r = FlxqX_red(x, T, p);
  long l = lgpol(r), lt = degpol(S), lm = 2*lt-1, v = varn(S);
  long i;
  if (l <= lt)
  {
    if (pr == ONLY_REM) return r;
    if (pr == ONLY_DIVIDES) return signe(r)? NULL: pol_0(v);
    if (pr) *pr = r;
    return pol_0(v);
  }
  if (lt <= 1)
    return FlxqX_divrem_basecase(x,S,T,p,pr);
  if (pr != ONLY_REM && l>lm)
  {
    long vT = get_Flx_var(T);
    q = cgetg(l-lt+2, t_POL); q[1] = S[1];
    for (i=0;i<l-lt;i++) gel(q+2,i) = pol0_Flx(vT);
  }
  while (l>lm)
  {
    GEN zr, zq = FlxqX_divrem_Barrettspec(r+2+l-lm,lm,mg,S,T,p,&zr);
    long lz = lgpol(zr);
    if (pr != ONLY_REM)
    {
      long lq = lgpol(zq);
      for(i=0; i<lq; i++) gel(q+2+l-lm,i) = gel(zq,2+i);
    }
    for(i=0; i<lz; i++) gel(r+2+l-lm,i) = gel(zr,2+i);
    l = l-lm+lz;
  }
  if (pr == ONLY_REM)
  {
    if (l > lt)
      r = FlxqX_divrem_Barrettspec(r+2,l,mg,S,T,p,ONLY_REM);
    else
      r = FlxX_renormalize(r, l+2);
    setvarn(r, v); return r;
  }
  if (l > lt)
  {
    GEN zq = FlxqX_divrem_Barrettspec(r+2,l,mg,S,T,p,pr? &r: NULL);
    if (!q) q = zq;
    else
    {
      long lq = lgpol(zq);
      for(i=0; i<lq; i++) gel(q+2,i) = gel(zq,2+i);
    }
  }
  else if (pr)
    r = FlxX_renormalize(r, l+2);
  setvarn(q, v); q = FlxX_renormalize(q, lg(q));
  if (pr == ONLY_DIVIDES) return signe(r)? NULL: q;
  if (pr) { setvarn(r, v); *pr = r; }
  return q;
}

GEN
FlxqX_divrem(GEN x, GEN S, GEN T, ulong p, GEN *pr)
{
  GEN B, y;
  long dy, dx, d;
  if (pr==ONLY_REM) return FlxqX_rem(x, S, T, p);
  y = get_FlxqX_red(S, &B);
  dy = degpol(y); dx = degpol(x); d = dx-dy;
  if (!B && d+3 < FlxqX_DIVREM_BARRETT_LIMIT)
    return FlxqX_divrem_basecase(x,y,T,p,pr);
  else
  {
    pari_sp av = avma;
    GEN mg = B? B: FlxqX_invBarrett(y, T, p);
    GEN q = FlxqX_divrem_Barrett(x,mg,y,T,p,pr);
    if (!q) return gc_NULL(av);
    if (!pr || pr==ONLY_DIVIDES) return gerepilecopy(av, q);
    gerepileall(av,2,&q,pr);
    return q;
  }
}

GEN
FlxqX_rem(GEN x, GEN S, GEN T, ulong p)
{
  GEN B, y = get_FlxqX_red(S, &B);
  long dy = degpol(y), dx = degpol(x), d = dx-dy;
  if (d < 0) return FlxqX_red(x, T, p);
  if (!B && d+3 < FlxqX_REM_BARRETT_LIMIT)
    return FlxqX_divrem_basecase(x,y, T, p, ONLY_REM);
  else
  {
    pari_sp av=avma;
    GEN mg = B? B: FlxqX_invBarrett(y, T, p);
    GEN r = FlxqX_divrem_Barrett(x, mg, y, T, p, ONLY_REM);
    return gerepileupto(av, r);
  }
}

static GEN
FlxqX_halfgcd_basecase(GEN a, GEN b, GEN T, ulong p)
{
  pari_sp av=avma;
  GEN u,u1,v,v1;
  long vx = varn(a);
  long n = lgpol(a)>>1;
  u1 = v = pol_0(vx);
  u = v1 = pol1_FlxX(vx, get_Flx_var(T));
  while (lgpol(b)>n)
  {
    GEN r, q = FlxqX_divrem(a,b, T, p, &r);
    a = b; b = r; swap(u,u1); swap(v,v1);
    u1 = FlxX_sub(u1, FlxqX_mul(u, q, T, p), p);
    v1 = FlxX_sub(v1, FlxqX_mul(v, q ,T, p), p);
    if (gc_needed(av,2))
    {
      if (DEBUGMEM>1) pari_warn(warnmem,"FlxqX_halfgcd (d = %ld)",degpol(b));
      gerepileall(av,6, &a,&b,&u1,&v1,&u,&v);
    }
  }
  return gerepilecopy(av, mkmat2(mkcol2(u,u1), mkcol2(v,v1)));
}
static GEN
FlxqX_addmulmul(GEN u, GEN v, GEN x, GEN y, GEN T, ulong p)
{
  return FlxX_add(FlxqX_mul(u, x, T, p),FlxqX_mul(v, y, T, p), p);
}

static GEN
FlxqXM_FlxqX_mul2(GEN M, GEN x, GEN y, GEN T, ulong p)
{
  GEN res = cgetg(3, t_COL);
  gel(res, 1) = FlxqX_addmulmul(gcoeff(M,1,1), gcoeff(M,1,2), x, y, T, p);
  gel(res, 2) = FlxqX_addmulmul(gcoeff(M,2,1), gcoeff(M,2,2), x, y, T, p);
  return res;
}

static GEN
FlxqXM_mul2(GEN A, GEN B, GEN T, ulong p)
{
  GEN A11=gcoeff(A,1,1),A12=gcoeff(A,1,2), B11=gcoeff(B,1,1),B12=gcoeff(B,1,2);
  GEN A21=gcoeff(A,2,1),A22=gcoeff(A,2,2), B21=gcoeff(B,2,1),B22=gcoeff(B,2,2);
  GEN M1 = FlxqX_mul(FlxX_add(A11,A22, p), FlxX_add(B11,B22, p), T, p);
  GEN M2 = FlxqX_mul(FlxX_add(A21,A22, p), B11, T, p);
  GEN M3 = FlxqX_mul(A11, FlxX_sub(B12,B22, p), T, p);
  GEN M4 = FlxqX_mul(A22, FlxX_sub(B21,B11, p), T, p);
  GEN M5 = FlxqX_mul(FlxX_add(A11,A12, p), B22, T, p);
  GEN M6 = FlxqX_mul(FlxX_sub(A21,A11, p), FlxX_add(B11,B12, p), T, p);
  GEN M7 = FlxqX_mul(FlxX_sub(A12,A22, p), FlxX_add(B21,B22, p), T, p);
  GEN T1 = FlxX_add(M1,M4, p), T2 = FlxX_sub(M7,M5, p);
  GEN T3 = FlxX_sub(M1,M2, p), T4 = FlxX_add(M3,M6, p);
  retmkmat2(mkcol2(FlxX_add(T1,T2, p), FlxX_add(M2,M4, p)),
            mkcol2(FlxX_add(M3,M5, p), FlxX_add(T3,T4, p)));
}

/* Return [0,1;1,-q]*M */
static GEN
FlxqX_FlxqXM_qmul(GEN q, GEN M, GEN T, ulong p)
{
  GEN u, v, res = cgetg(3, t_MAT);
  u = FlxX_sub(gcoeff(M,1,1), FlxqX_mul(gcoeff(M,2,1), q, T, p), p);
  gel(res,1) = mkcol2(gcoeff(M,2,1), u);
  v = FlxX_sub(gcoeff(M,1,2), FlxqX_mul(gcoeff(M,2,2), q, T, p), p);
  gel(res,2) = mkcol2(gcoeff(M,2,2), v);
  return res;
}

static GEN
matid2_FlxXM(long v, long sv)
{
  retmkmat2(mkcol2(pol1_FlxX(v, sv),pol_0(v)),
            mkcol2(pol_0(v),pol1_FlxX(v, sv)));
}

static GEN
FlxqX_halfgcd_split(GEN x, GEN y, GEN T, ulong p)
{
  pari_sp av=avma;
  GEN R, S, V;
  GEN y1, r, q;
  long l = lgpol(x), n = l>>1, k;
  long vT = get_Flx_var(T);
  if (lgpol(y)<=n) return matid2_FlxXM(varn(x),vT);
  R = FlxqX_halfgcd(FlxX_shift(x,-n,vT),FlxX_shift(y,-n,vT), T, p);
  V = FlxqXM_FlxqX_mul2(R,x,y, T, p); y1 = gel(V,2);
  if (lgpol(y1)<=n) return gerepilecopy(av, R);
  q = FlxqX_divrem(gel(V,1), y1, T, p, &r);
  k = 2*n-degpol(y1);
  S = FlxqX_halfgcd(FlxX_shift(y1,-k,vT), FlxX_shift(r,-k,vT), T, p);
  return gerepileupto(av, FlxqXM_mul2(S,FlxqX_FlxqXM_qmul(q,R, T, p), T, p));
}

/* Return M in GL_2(Fp[X]) such that:
if [a',b']~=M*[a,b]~ then degpol(a')>= (lgpol(a)>>1) >degpol(b')
*/

static GEN
FlxqX_halfgcd_i(GEN x, GEN y, GEN T, ulong p)
{
  if (lg(x)<=FlxqX_HALFGCD_LIMIT) return FlxqX_halfgcd_basecase(x, y, T, p);
  return FlxqX_halfgcd_split(x, y, T, p);
}

GEN
FlxqX_halfgcd(GEN x, GEN y, GEN T, ulong p)
{
  pari_sp av = avma;
  GEN M,q,r;
  if (!signe(x))
  {
    long v = varn(x), vT = get_Flx_var(T);
    retmkmat2(mkcol2(pol_0(v),pol1_FlxX(v,vT)),
        mkcol2(pol1_FlxX(v,vT),pol_0(v)));
  }
  if (degpol(y)<degpol(x)) return FlxqX_halfgcd_i(x, y, T, p);
  q = FlxqX_divrem(y, x, T, p, &r);
  M = FlxqX_halfgcd_i(x, r, T, p);
  gcoeff(M,1,1) = FlxX_sub(gcoeff(M,1,1), FlxqX_mul(q, gcoeff(M,1,2), T, p), p);
  gcoeff(M,2,1) = FlxX_sub(gcoeff(M,2,1), FlxqX_mul(q, gcoeff(M,2,2), T, p), p);
  return gerepilecopy(av, M);
}

static GEN
FlxqX_gcd_basecase(GEN a, GEN b, GEN T, ulong p)
{
  pari_sp av = avma, av0=avma;
  while (signe(b))
  {
    GEN c;
    if (gc_needed(av0,2))
    {
      if (DEBUGMEM>1) pari_warn(warnmem,"FlxqX_gcd (d = %ld)",degpol(b));
      gerepileall(av0,2, &a,&b);
    }
    av = avma; c = FlxqX_rem(a, b, T, p); a=b; b=c;
  }
  return gc_const(av, a);
}

GEN
FlxqX_gcd(GEN x, GEN y, GEN T, ulong p)
{
  pari_sp av = avma;
  x = FlxqX_red(x, T, p);
  y = FlxqX_red(y, T, p);
  if (!signe(x)) return gerepileupto(av, y);
  while (lg(y)>FlxqX_GCD_LIMIT)
  {
    GEN c;
    if (lgpol(y)<=(lgpol(x)>>1))
    {
      GEN r = FlxqX_rem(x, y, T, p);
      x = y; y = r;
    }
    c = FlxqXM_FlxqX_mul2(FlxqX_halfgcd(x,y, T, p), x, y, T, p);
    x = gel(c,1); y = gel(c,2);
    gerepileall(av,2,&x,&y);
  }
  return gerepileupto(av, FlxqX_gcd_basecase(x, y, T, p));
}

static GEN
FlxqX_extgcd_basecase(GEN a, GEN b, GEN T, ulong p, GEN *ptu, GEN *ptv)
{
  pari_sp av=avma;
  GEN u,v,d,d1,v1;
  long vx = varn(a);
  d = a; d1 = b;
  v = pol_0(vx); v1 = pol1_FlxX(vx, get_Flx_var(T));
  while (signe(d1))
  {
    GEN r, q = FlxqX_divrem(d, d1, T, p, &r);
    v = FlxX_sub(v,FlxqX_mul(q,v1,T, p),p);
    u=v; v=v1; v1=u;
    u=r; d=d1; d1=u;
    if (gc_needed(av,2))
    {
      if (DEBUGMEM>1) pari_warn(warnmem,"FlxqX_extgcd (d = %ld)",degpol(d));
      gerepileall(av,5, &d,&d1,&u,&v,&v1);
    }
  }
  if (ptu) *ptu = FlxqX_div(FlxX_sub(d,FlxqX_mul(b,v, T, p), p), a, T, p);
  *ptv = v; return d;
}

static GEN
FlxqX_extgcd_halfgcd(GEN x, GEN y, GEN T, ulong p, GEN *ptu, GEN *ptv)
{
  pari_sp av=avma;
  GEN u,v,R = matid2_FlxXM(varn(x), get_Flx_var(T));
  while (lg(y)>FlxqX_EXTGCD_LIMIT)
  {
    GEN M, c;
    if (lgpol(y)<=(lgpol(x)>>1))
    {
      GEN r, q = FlxqX_divrem(x, y, T, p, &r);
      x = y; y = r;
      R = FlxqX_FlxqXM_qmul(q, R, T, p);
    }
    M = FlxqX_halfgcd(x,y, T, p);
    c = FlxqXM_FlxqX_mul2(M, x,y, T, p);
    R = FlxqXM_mul2(M, R, T, p);
    x = gel(c,1); y = gel(c,2);
    gerepileall(av,3,&x,&y,&R);
  }
  y = FlxqX_extgcd_basecase(x,y, T, p, &u,&v);
  if (ptu) *ptu = FlxqX_addmulmul(u,v,gcoeff(R,1,1),gcoeff(R,2,1), T, p);
  *ptv = FlxqX_addmulmul(u,v,gcoeff(R,1,2),gcoeff(R,2,2), T, p);
  return y;
}

/* x and y in Z[Y][X], return lift(gcd(x mod T,p, y mod T,p)). Set u and v st
 * ux + vy = gcd (mod T,p) */
GEN
FlxqX_extgcd(GEN x, GEN y, GEN T, ulong p, GEN *ptu, GEN *ptv)
{
  GEN d;
  pari_sp ltop=avma;
  x = FlxqX_red(x, T, p);
  y = FlxqX_red(y, T, p);
  if (lg(y)>FlxqX_EXTGCD_LIMIT)
    d = FlxqX_extgcd_halfgcd(x, y, T, p, ptu, ptv);
  else
    d = FlxqX_extgcd_basecase(x, y, T, p, ptu, ptv);
  gerepileall(ltop,ptu?3:2,&d,ptv,ptu);
  return d;
}

static GEN
FlxqX_saferem(GEN P, GEN Q, GEN T, ulong p)
{
  GEN U = Flxq_invsafe(leading_coeff(Q), T, p);
  if (!U) return NULL;
  Q = FlxqX_Flxq_mul_to_monic(Q,U,T,p);
  return FlxqX_rem(P,Q,T,p);
}

GEN
FlxqX_safegcd(GEN P, GEN Q, GEN T, ulong p)
{
  pari_sp av = avma;
  GEN U;
  if (!signe(P)) return gcopy(Q);
  if (!signe(Q)) return gcopy(P);
  T = Flx_get_red(T,p);
  for(;;)
  {
    P = FlxqX_saferem(P,Q,T,p);
    if (!P) return gc_NULL(av);
    if (!signe(P)) break;
    if (gc_needed(av, 1))
    {
      if (DEBUGMEM>1) pari_warn(warnmem,"FlxqX_safegcd");
      gerepileall(av, 2, &P,&Q);
    }
    swap(P, Q);
  }
  U = Flxq_invsafe(leading_coeff(Q), T, p);
  if (!U) return gc_NULL(av);
  Q = FlxqX_Flxq_mul_to_monic(Q,U,T,p);
  return gerepileupto(av, Q);
}

struct _FlxqX {ulong p; GEN T;};
static GEN _FlxqX_mul(void *data,GEN a,GEN b)
{
  struct _FlxqX *d=(struct _FlxqX*)data;
  return FlxqX_mul(a,b,d->T,d->p);
}
static GEN _FlxqX_sqr(void *data,GEN a)
{
  struct _FlxqX *d=(struct _FlxqX*)data;
  return FlxqX_sqr(a,d->T,d->p);
}

GEN
FlxqX_powu(GEN V, ulong n, GEN T, ulong p)
{
  struct _FlxqX d; d.p=p; d.T=T;
  return gen_powu(V, n, (void*)&d, &_FlxqX_sqr, &_FlxqX_mul);
}

/* Res(A,B) = Res(B,R) * lc(B)^(a-r) * (-1)^(ab), with R=A%B, a=deg(A) ...*/
GEN
FlxqX_saferesultant(GEN a, GEN b, GEN T, ulong p)
{
  long vT = get_Flx_var(T);
  long da,db,dc;
  pari_sp av;
  GEN c,lb, res = pol1_Flx(vT);

  if (!signe(a) || !signe(b)) return pol0_Flx(vT);

  da = degpol(a);
  db = degpol(b);
  if (db > da)
  {
    swapspec(a,b, da,db);
    if (both_odd(da,db)) res = Flx_neg(res, p);
  }
  if (!da) return pol1_Flx(vT); /* = res * a[2] ^ db, since 0 <= db <= da = 0 */
  av = avma;
  while (db)
  {
    lb = gel(b,db+2);
    c = FlxqX_saferem(a,b, T,p);
    if (!c) return gc_NULL(av);
    a = b; b = c; dc = degpol(c);
    if (dc < 0) { set_avma(av); return pol0_Flx(vT); }

    if (both_odd(da,db)) res = Flx_neg(res, p);
    if (!equali1(lb)) res = Flxq_mul(res, Flxq_powu(lb, da - dc, T, p), T, p);
    if (gc_needed(av,2))
    {
      if (DEBUGMEM>1) pari_warn(warnmem,"FlxqX_resultant (da = %ld)",da);
      gerepileall(av,3, &a,&b,&res);
    }
    da = db; /* = degpol(a) */
    db = dc; /* = degpol(b) */
  }
  res = Flxq_mul(res, Flxq_powu(gel(b,2), da, T, p), T, p);
  return gerepileupto(av, res);
}

/* Res(A,B) = Res(B,R) * lc(B)^(a-r) * (-1)^(ab), with R=A%B, a=deg(A) ...*/
GEN
FlxqX_resultant(GEN a, GEN b, GEN T, ulong p)
{
  long vT = get_Flx_var(T);
  long da,db,dc;
  pari_sp av;
  GEN c,lb, res = pol1_Flx(vT);

  if (!signe(a) || !signe(b)) return pol0_Flx(vT);

  da = degpol(a);
  db = degpol(b);
  if (db > da)
  {
    swapspec(a,b, da,db);
    if (both_odd(da,db)) res = Flx_neg(res, p);
  }
  if (!da) return pol1_Flx(vT); /* = res * a[2] ^ db, since 0 <= db <= da = 0 */
  av = avma;
  while (db)
  {
    lb = gel(b,db+2);
    c = FlxqX_rem(a,b, T,p);
    a = b; b = c; dc = degpol(c);
    if (dc < 0) { set_avma(av); return pol0_Flx(vT); }

    if (both_odd(da,db)) res = Flx_neg(res, p);
    if (!equali1(lb)) res = Flxq_mul(res, Flxq_powu(lb, da - dc, T, p), T, p);
    if (gc_needed(av,2))
    {
      if (DEBUGMEM>1) pari_warn(warnmem,"FlxqX_resultant (da = %ld)",da);
      gerepileall(av,3, &a,&b,&res);
    }
    da = db; /* = degpol(a) */
    db = dc; /* = degpol(b) */
  }
  res = Flxq_mul(res, Flxq_powu(gel(b,2), da, T, p), T, p);
  return gerepileupto(av, res);
}

/* disc P = (-1)^(n(n-1)/2) lc(P)^(n - deg P' - 2) Res(P,P'), n = deg P */
GEN
FlxqX_disc(GEN P, GEN T, ulong p)
{
  pari_sp av = avma;
  GEN L, dP = FlxX_deriv(P, p), D = FlxqX_resultant(P, dP, T, p);
  long dd;
  if (!lgpol(D)) return pol0_Flx(get_Flx_var(T));
  dd = degpol(P) - 2 - degpol(dP); /* >= -1; > -1 iff p | deg(P) */
  L = leading_coeff(P);
  if (dd && !Flx_equal1(L))
    D = (dd == -1)? Flxq_div(D,L,T,p): Flxq_mul(D, Flxq_powu(L, dd, T, p), T, p);
  if (degpol(P) & 2) D = Flx_neg(D, p);
  return gerepileupto(av, D);
}

INLINE GEN
FlxXn_recip(GEN x, long n, long v)
{
  return FlxX_recipspec(x+2, minss(lgpol(x), n), n, v);
}

GEN
FlxqX_Newton(GEN P, long n, GEN T, ulong p)
{
  pari_sp av = avma;
  long d = degpol(P), vT = get_Flx_var(T);
  GEN dP = FlxXn_recip(FlxX_deriv(P, p), d, vT);
  GEN Q = FlxqXn_mul(FlxqXn_inv(FlxXn_recip(P, d+1, vT), n, T, p), dP, n, T, p);
  return gerepilecopy(av, Q);
}

GEN
FlxqX_fromNewton(GEN P, GEN T, ulong p)
{
  pari_sp av = avma;
  long vT = get_Flx_var(T);
  long n = Flx_constant(constant_coeff(P))+1;
  GEN z = FlxX_neg(FlxX_shift(P, -1, vT), p);
  GEN Q = FlxXn_recip(FlxqXn_expint(z, n, T, p), n, vT);
  return gerepilecopy(av, Q);
}

static GEN
FlxqX_composedsum(GEN P, GEN Q, GEN T, ulong p)
{
  long n = 1+ degpol(P)*degpol(Q);
  GEN Pl = FlxX_invLaplace(FlxqX_Newton(P,n, T,p), p);
  GEN Ql = FlxX_invLaplace(FlxqX_Newton(Q,n, T,p), p);
  GEN L = FlxX_Laplace(FlxqXn_mul(Pl, Ql, n, T,p), p);
  GEN R = FlxqX_fromNewton(L, T, p);
  GEN lead = Flxq_mul(Flxq_powu(leading_coeff(P),degpol(Q), T, p),
                      Flxq_powu(leading_coeff(Q),degpol(P), T, p), T, p);
  return FlxqX_Flxq_mul(R, lead, T, p);
}

GEN
FlxqX_direct_compositum(GEN P, GEN Q, GEN T, ulong p)
{
  return FlxqX_composedsum(P, Q, T, p);
}

GEN
FlxqXV_prod(GEN V, GEN T, ulong p)
{
  struct _FlxqX d; d.p=p; d.T=T;
  return gen_product(V, (void*)&d, &_FlxqX_mul);
}

static GEN
FlxqV_roots_to_deg1(GEN x, GEN T, ulong p, long v)
{
  long sv = get_Flx_var(T);
  pari_APPLY_same(deg1pol_shallow(pol1_Flx(sv),Flx_neg(gel(x,i),p),v))
}

GEN
FlxqV_roots_to_pol(GEN V, GEN T, ulong p, long v)
{
  pari_sp ltop = avma;
  GEN W = FlxqV_roots_to_deg1(V, T, p, v);
  return gerepileupto(ltop, FlxqXV_prod(W, T, p));
}

/*******************************************************************/
/*                                                                 */
/*                       (Fl[X]/T(X))[Y] / S(Y)                    */
/*                                                                 */
/*******************************************************************/

GEN
FlxqXQ_mul(GEN x, GEN y, GEN S, GEN T, ulong p) {
  return FlxqX_rem(FlxqX_mul(x,y,T,p),S,T,p);
}

GEN
FlxqXQ_sqr(GEN x, GEN S, GEN T, ulong p) {
  return FlxqX_rem(FlxqX_sqr(x,T,p),S,T,p);
}

GEN
FlxqXQ_invsafe(GEN x, GEN S, GEN T, ulong p)
{
  GEN V, z = FlxqX_extgcd(get_FlxqX_mod(S), x, T, p, NULL, &V);
  if (degpol(z)) return NULL;
  z = Flxq_invsafe(gel(z,2),T,p);
  if (!z) return NULL;
  return FlxqX_Flxq_mul(V, z, T, p);
}

GEN
FlxqXQ_inv(GEN x, GEN S, GEN T,ulong p)
{
  pari_sp av = avma;
  GEN U = FlxqXQ_invsafe(x, S, T, p);
  if (!U) pari_err_INV("FlxqXQ_inv",x);
  return gerepileupto(av, U);
}

GEN
FlxqXQ_div(GEN x, GEN y, GEN S, GEN T, ulong p) {
  return FlxqXQ_mul(x, FlxqXQ_inv(y,S,T,p),S,T,p);
}

struct _FlxqXQ {
  GEN T, S;
  ulong p;
};
static GEN
_FlxqXQ_add(void *data, GEN x, GEN y) {
  struct _FlxqXQ *d = (struct _FlxqXQ*) data;
  return FlxX_add(x,y, d->p);
}
static GEN
_FlxqXQ_sub(void *data, GEN x, GEN y) {
  struct _FlxqXQ *d = (struct _FlxqXQ*) data;
  return FlxX_sub(x,y, d->p);
}
#if 0
static GEN
_FlxqXQ_cmul(void *data, GEN P, long a, GEN x) {
  struct _FlxqXQ *d = (struct _FlxqXQ*) data;
  return FlxX_Flx_mul(x,gel(P,a+2), d->p);
}
#endif
static GEN
_FlxqXQ_red(void *data, GEN x) {
  struct _FlxqXQ *d = (struct _FlxqXQ*) data;
  return FlxqX_red(x, d->T, d->p);
}
static GEN
_FlxqXQ_mul(void *data, GEN x, GEN y) {
  struct _FlxqXQ *d = (struct _FlxqXQ*) data;
  return FlxqXQ_mul(x,y, d->S,d->T, d->p);
}
static GEN
_FlxqXQ_sqr(void *data, GEN x) {
  struct _FlxqXQ *d = (struct _FlxqXQ*) data;
  return FlxqXQ_sqr(x, d->S,d->T, d->p);
}

static GEN
_FlxqXQ_one(void *data) {
  struct _FlxqXQ *d = (struct _FlxqXQ*) data;
  return pol1_FlxX(get_FlxqX_var(d->S),get_Flx_var(d->T));
}

static GEN
_FlxqXQ_zero(void *data) {
  struct _FlxqXQ *d = (struct _FlxqXQ*) data;
  return pol_0(get_FlxqX_var(d->S));
}

static struct bb_algebra FlxqXQ_algebra = { _FlxqXQ_red, _FlxqXQ_add,
       _FlxqXQ_sub, _FlxqXQ_mul, _FlxqXQ_sqr, _FlxqXQ_one, _FlxqXQ_zero };

const struct bb_algebra *
get_FlxqXQ_algebra(void **E, GEN S, GEN T, ulong p)
{
  GEN z = new_chunk(sizeof(struct _FlxqXQ));
  struct _FlxqXQ *e = (struct _FlxqXQ *) z;
  e->T = Flx_get_red(T, p);
  e->S = FlxqX_get_red(S, e->T, p);
  e->p  = p; *E = (void*)e;
  return &FlxqXQ_algebra;
}

/* x over Fq, return lift(x^n) mod S */
GEN
FlxqXQ_pow(GEN x, GEN n, GEN S, GEN T, ulong p)
{
  pari_sp av = avma;
  struct _FlxqXQ D;
  long s = signe(n);
  if (!s) return pol1_FlxX(get_FlxqX_var(S),get_Flx_var(T));
  if (s < 0) x = FlxqXQ_inv(x,S,T,p);
  if (is_pm1(n)) return s < 0 ? x : gcopy(x);
  if (degpol(x) >= get_FlxqX_degree(S)) x = FlxqX_rem(x,S,T,p);
  T = Flx_get_red(T, p);
  S = FlxqX_get_red(S, T, p);
  D.S = S;
  D.T = T;
  D.p = p;
  x = gen_pow_i(x, n, (void*)&D, &_FlxqXQ_sqr, &_FlxqXQ_mul);
  return gerepilecopy(av, x);
}

/* x over Fq, return lift(x^n) mod S */
GEN
FlxqXQ_powu(GEN x, ulong n, GEN S, GEN T, ulong p)
{
  pari_sp av = avma;
  struct _FlxqXQ D;
  switch(n)
  {
    case 0: return pol1_FlxX(get_FlxqX_var(S),get_Flx_var(T));
    case 1: return gcopy(x);
    case 2: return FlxqXQ_sqr(x, S, T, p);
  }
  T = Flx_get_red(T, p);
  S = FlxqX_get_red(S, T, p);
  D.S = S; D.T = T; D.p = p;
  x = gen_powu_i(x, n, (void*)&D, &_FlxqXQ_sqr, &_FlxqXQ_mul);
  return gerepilecopy(av, x);
}

GEN
FlxqXQ_powers(GEN x, long l, GEN S, GEN T, ulong p)
{
  struct _FlxqXQ D;
  int use_sqr = 2*degpol(x) >= get_FlxqX_degree(S);
  T = Flx_get_red(T, p);
  S = FlxqX_get_red(S, T, p);
  D.S = S; D.T = T; D.p = p;
  return gen_powers(x, l, use_sqr, (void*)&D, &_FlxqXQ_sqr, &_FlxqXQ_mul,&_FlxqXQ_one);
}

/* Let v a linear form, return the linear form z->v(tau*z)
   that is, v*(M_tau) */

static GEN
FlxqXQ_transmul_init(GEN tau, GEN S, GEN T, ulong p)
{
  GEN bht;
  GEN h, Sp = get_FlxqX_red(S, &h);
  long n = degpol(Sp), vS = varn(Sp), vT = get_Flx_var(T);
  GEN ft = FlxX_recipspec(Sp+2, n+1, n+1, vT);
  GEN bt = FlxX_recipspec(tau+2, lgpol(tau), n, vT);
  setvarn(ft, vS); setvarn(bt, vS);
  if (h)
    bht = FlxqXn_mul(bt, h, n-1, T, p);
  else
  {
    GEN bh = FlxqX_div(FlxX_shift(tau, n-1, vT), S, T, p);
    bht = FlxX_recipspec(bh+2, lgpol(bh), n-1, vT);
    setvarn(bht, vS);
  }
  return mkvec3(bt, bht, ft);
}

static GEN
FlxqXQ_transmul(GEN tau, GEN a, long n, GEN T, ulong p)
{
  pari_sp ltop = avma;
  GEN t1, t2, t3, vec;
  GEN bt = gel(tau, 1), bht = gel(tau, 2), ft = gel(tau, 3);
  long vT = get_Flx_var(T);
  if (signe(a)==0) return pol_0(varn(a));
  t2 = FlxX_shift(FlxqX_mul(bt, a, T, p),1-n,vT);
  if (signe(bht)==0) return gerepilecopy(ltop, t2);
  t1 = FlxX_shift(FlxqX_mul(ft, a, T, p),-n,vT);
  t3 = FlxqXn_mul(t1, bht, n-1, T, p);
  vec = FlxX_sub(t2, FlxX_shift(t3, 1, vT), p);
  return gerepileupto(ltop, vec);
}

static GEN
polxn_FlxX(long n, long v, long vT)
{
  long i, a = n+2;
  GEN p = cgetg(a+1, t_POL);
  p[1] = evalsigne(1)|evalvarn(v);
  for (i = 2; i < a; i++) gel(p,i) = pol0_Flx(vT);
  gel(p,a) = pol1_Flx(vT); return p;
}

GEN
FlxqXQ_minpoly(GEN x, GEN S, GEN T, ulong p)
{
  pari_sp ltop = avma;
  long vS, vT, n;
  GEN v_x, g, tau;
  vS = get_FlxqX_var(S);
  vT = get_Flx_var(T);
  n = get_FlxqX_degree(S);
  g = pol1_FlxX(vS,vT);
  tau = pol1_FlxX(vS,vT);
  S = FlxqX_get_red(S, T, p);
  v_x = FlxqXQ_powers(x, usqrt(2*n), S, T, p);
  while(signe(tau) != 0)
  {
    long i, j, m, k1;
    GEN M, v, tr;
    GEN g_prime, c;
    if (degpol(g) == n) { tau = pol1_FlxX(vS, vT); g = pol1_FlxX(vS, vT); }
    v = random_FlxqX(n, vS, T, p);
    tr = FlxqXQ_transmul_init(tau, S, T, p);
    v = FlxqXQ_transmul(tr, v, n, T, p);
    m = 2*(n-degpol(g));
    k1 = usqrt(m);
    tr = FlxqXQ_transmul_init(gel(v_x,k1+1), S, T, p);
    c = cgetg(m+2,t_POL);
    c[1] = evalsigne(1)|evalvarn(vS);
    for (i=0; i<m; i+=k1)
    {
      long mj = minss(m-i, k1);
      for (j=0; j<mj; j++)
        gel(c,m+1-(i+j)) = FlxqX_dotproduct(v, gel(v_x,j+1), T, p);
      v = FlxqXQ_transmul(tr, v, n, T, p);
    }
    c = FlxX_renormalize(c, m+2);
    /* now c contains <v,x^i> , i = 0..m-1  */
    M = FlxqX_halfgcd(polxn_FlxX(m, vS, vT), c, T, p);
    g_prime = gmael(M, 2, 2);
    if (degpol(g_prime) < 1) continue;
    g = FlxqX_mul(g, g_prime, T, p);
    tau = FlxqXQ_mul(tau, FlxqX_FlxqXQV_eval(g_prime, v_x, S, T, p), S, T, p);
  }
  g = FlxqX_normalize(g,T, p);
  return gerepilecopy(ltop,g);
}

GEN
FlxqXQ_matrix_pow(GEN y, long n, long m, GEN S, GEN T, ulong p)
{
  return FlxXV_to_FlxM(FlxqXQ_powers(y,m-1,S,T,p), n, get_Flx_var(T));
}

static GEN
FlxX_blocks_FlxM(GEN P, long n, long m, long v)
{
  GEN z = cgetg(m+1,t_MAT);
  long i,j, k=2, l = lg(P);
  for(i=1; i<=m; i++)
  {
    GEN zi = cgetg(n+1,t_COL);
    gel(z,i) = zi;
    for(j=1; j<=n; j++)
      gel(zi, j) = k==l ? pol0_Flx(v) : gel(P,k++);
  }
  return z;
}

GEN
FlxqX_FlxqXQV_eval(GEN Q, GEN x, GEN S, GEN T, ulong p)
{
  pari_sp btop, av = avma;
  long v = get_FlxqX_var(S), m = get_FlxqX_degree(S);
  long vT = get_Flx_var(T);
  long i, l = lg(x)-1, lQ = lgpol(Q), n,  d;
  GEN A, B, C, R, g;
  if (lQ == 0) return pol_0(v);
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
  A = FlxXV_to_FlxM_lg(x, m, n, vT);
  B = FlxX_blocks_FlxM(Q, n, d, vT);
  C = gerepileupto(av, FlxqM_mul(A, B, T, p));
  g = gel(x, l);
  T = Flx_get_red(T, p);
  S = FlxqX_get_red(S, T, p);
  btop = avma;
  R = FlxV_to_FlxX(gel(C, d), v);
  for (i = d-1; i>0; i--)
  {
    R = FlxX_add(FlxqXQ_mul(R, g, S, T, p), FlxV_to_FlxX(gel(C,i), v), p);
    if (gc_needed(btop,1))
      R = gerepileupto(btop, R);
  }
  return gerepilecopy(av, R);
}

GEN
FlxqX_FlxqXQ_eval(GEN Q, GEN x, GEN S, GEN T, ulong p)
{
  pari_sp av = avma;
  GEN z, V;
  long d = degpol(Q), rtd;
  if (d < 0) return pol_0(get_FlxqX_var(S));
  rtd = (long) sqrt((double)d);
  T = Flx_get_red(T, p);
  S = FlxqX_get_red(S, T, p);
  V = FlxqXQ_powers(x, rtd, S, T, p);
  z = FlxqX_FlxqXQV_eval(Q, V, S, T, p);
  return gerepileupto(av, z);
}

static GEN
FlxqXQ_autpow_sqr(void * E, GEN x)
{
  struct _FlxqXQ *D = (struct _FlxqXQ *)E;
  GEN S = D->S, T = D->T;
  ulong p = D->p;
  GEN phi = gel(x,1), S1 = gel(x,2);
  long n = brent_kung_optpow(get_Flx_degree(T)-1,lgpol(S1)+1,1);
  GEN V = Flxq_powers(phi, n, T, p);
  GEN phi2 = Flx_FlxqV_eval(phi, V, T, p);
  GEN Sphi = FlxY_FlxqV_evalx(S1, V, T, p);
  GEN S2 = FlxqX_FlxqXQ_eval(Sphi, S1, S, T, p);
  return mkvec2(phi2, S2);
}

static GEN
FlxqXQ_autpow_mul(void * E, GEN x, GEN y)
{
  struct _FlxqXQ *D = (struct _FlxqXQ *)E;
  GEN S = D->S, T = D->T;
  ulong p = D->p;
  GEN phi1 = gel(x,1), S1 = gel(x,2);
  GEN phi2 = gel(y,1), S2 = gel(y,2);
  long n = brent_kung_optpow(get_Flx_degree(T)-1,lgpol(S1)+1,1);
  GEN V = Flxq_powers(phi2, n, T, p);
  GEN phi3 = Flx_FlxqV_eval(phi1, V, T, p);
  GEN Sphi = FlxY_FlxqV_evalx(S1, V, T, p);
  GEN S3 = FlxqX_FlxqXQ_eval(Sphi, S2, S, T, p);
  return mkvec2(phi3, S3);
}

GEN
FlxqXQ_autpow(GEN aut, long n, GEN S, GEN T, ulong p)
{
  pari_sp av = avma;
  struct _FlxqXQ D;
  T = Flx_get_red(T, p);
  S = FlxqX_get_red(S, T, p);
  D.S=S; D.T=T; D.p=p;
  aut = gen_powu_i(aut,n,&D,FlxqXQ_autpow_sqr,FlxqXQ_autpow_mul);
  return gerepilecopy(av, aut);
}

static GEN
FlxqXQ_autsum_mul(void *E, GEN x, GEN y)
{
  struct _FlxqXQ *D = (struct _FlxqXQ *)E;
  GEN S = D->S, T = D->T;
  ulong p = D->p;
  GEN phi1 = gel(x,1), S1 = gel(x,2), a1 = gel(x,3);
  GEN phi2 = gel(y,1), S2 = gel(y,2), a2 = gel(y,3);
  long n2 = brent_kung_optpow(get_Flx_degree(T)-1, lgpol(S1)+lgpol(a1)+1,1);
  GEN V2 = Flxq_powers(phi2, n2, T, p);
  GEN phi3 = Flx_FlxqV_eval(phi1, V2, T, p);
  GEN Sphi = FlxY_FlxqV_evalx(S1, V2, T, p);
  GEN aphi = FlxY_FlxqV_evalx(a1, V2, T, p);
  long n = brent_kung_optpow(maxss(degpol(Sphi),degpol(aphi)),2,1);
  GEN V = FlxqXQ_powers(S2, n, S, T, p);
  GEN S3 = FlxqX_FlxqXQV_eval(Sphi, V, S, T, p);
  GEN aS = FlxqX_FlxqXQV_eval(aphi, V, S, T, p);
  GEN a3 = FlxqXQ_mul(aS, a2, S, T, p);
  return mkvec3(phi3, S3, a3);
}

static GEN
FlxqXQ_autsum_sqr(void * T, GEN x)
{ return FlxqXQ_autsum_mul(T, x, x); }

GEN
FlxqXQ_autsum(GEN aut, long n, GEN S, GEN T, ulong p)
{
  pari_sp av = avma;
  struct _FlxqXQ D;
  T = Flx_get_red(T, p);
  S = FlxqX_get_red(S, T, p);
  D.S=S; D.T=T; D.p=p;
  aut = gen_powu_i(aut,n,&D,FlxqXQ_autsum_sqr,FlxqXQ_autsum_mul);
  return gerepilecopy(av, aut);
}

static GEN
FlxqXQ_auttrace_mul(void *E, GEN x, GEN y)
{
  struct _FlxqXQ *D = (struct _FlxqXQ *)E;
  GEN S = D->S, T = D->T;
  ulong p = D->p;
  GEN S1 = gel(x,1), a1 = gel(x,2);
  GEN S2 = gel(y,1), a2 = gel(y,2);
  long n = brent_kung_optpow(maxss(degpol(S1),degpol(a1)),2,1);
  GEN V = FlxqXQ_powers(S2, n, S, T, p);
  GEN S3 = FlxqX_FlxqXQV_eval(S1, V, S, T, p);
  GEN aS = FlxqX_FlxqXQV_eval(a1, V, S, T, p);
  GEN a3 = FlxX_add(aS, a2, p);
  return mkvec2(S3, a3);
}

static GEN
FlxqXQ_auttrace_sqr(void *E, GEN x)
{ return FlxqXQ_auttrace_mul(E, x, x); }

GEN
FlxqXQ_auttrace(GEN x, ulong n, GEN S, GEN T, ulong p)
{
  pari_sp av = avma;
  struct _FlxqXQ D;
  T = Flx_get_red(T, p);
  S = FlxqX_get_red(S, T, p);
  D.S=S; D.T=T; D.p=p;
  x = gen_powu_i(x,n,(void*)&D,FlxqXQ_auttrace_sqr,FlxqXQ_auttrace_mul);
  return gerepilecopy(av, x);
}

/*******************************************************************/
/*                                                                 */
/*                      FlxYqQ                                     */
/*                                                                 */
/*******************************************************************/

/*Preliminary implementation to speed up FpX_ffisom*/
typedef struct {
  GEN S, T;
  ulong p;
} FlxYqq_muldata;

/* reduce x in Fl[X, Y] in the algebra Fl[X, Y]/ (P(X),Q(Y)) */
static GEN
FlxYqq_redswap(GEN x, GEN S, GEN T, ulong p)
{
  pari_sp ltop=avma;
  long n = get_Flx_degree(S);
  long m = get_Flx_degree(T);
  long w = get_Flx_var(T);
  GEN V = FlxX_swap(x,m,w);
  V = FlxqX_red(V,S,p);
  V = FlxX_swap(V,n,w);
  return gerepilecopy(ltop,V);
}
static GEN
FlxYqq_sqr(void *data, GEN x)
{
  FlxYqq_muldata *D = (FlxYqq_muldata*)data;
  return FlxYqq_redswap(FlxqX_sqr(x, D->T, D->p),D->S,D->T,D->p);
}

static GEN
FlxYqq_mul(void *data, GEN x, GEN y)
{
  FlxYqq_muldata *D = (FlxYqq_muldata*)data;
  return FlxYqq_redswap(FlxqX_mul(x,y, D->T, D->p),D->S,D->T,D->p);
}

/* x in Z[X,Y], S in Z[X] over Fq = Z[Y]/(p,T); compute lift(x^n mod (S,T,p)) */
GEN
FlxYqq_pow(GEN x, GEN n, GEN S, GEN T, ulong p)
{
  FlxYqq_muldata D;
  D.S = S;
  D.T = T;
  D.p = p;
  return gen_pow(x, n, (void*)&D, &FlxYqq_sqr, &FlxYqq_mul);
}

/*******************************************************************/
/*                                                                 */
/*                      FlxqXn                                     */
/*                                                                 */
/*******************************************************************/

GEN
FlxXn_red(GEN a, long n)
{
  long i, L = n+2, l = lg(a);
  GEN  b;
  if (L >= l) return a; /* deg(x) < n */
  b = cgetg(L, t_POL); b[1] = a[1];
  for (i=2; i<L; i++) gel(b,i) = gel(a,i);
  return FlxX_renormalize(b,L);
}

GEN
FlxqXn_mul(GEN a, GEN b, long n, GEN T, ulong p)
{ return FlxXn_red(FlxqX_mul(a, b, T, p), n); }

GEN
FlxqXn_sqr(GEN a, long n, GEN T, ulong p)
{ return FlxXn_red(FlxqX_sqr(a, T, p), n); }

/* (f*g) \/ x^n */
static GEN
FlxqX_mulhigh_i(GEN f, GEN g, long n, GEN T, ulong p)
{
  return FlxX_shift(FlxqX_mul(f, g, T, p), -n , get_Flx_var(T));
}

static GEN
FlxqXn_mulhigh(GEN f, GEN g, long n2, long n, GEN T, ulong p)
{
  long vT = get_Flx_var(T);
  GEN F = FlxX_blocks(f, n2, 2, vT), fl = gel(F,1), fh = gel(F,2);
  return FlxX_add(FlxqX_mulhigh_i(fl, g, n2, T, p),
                  FlxqXn_mul(fh, g, n - n2, T, p), p);
}

GEN
FlxqXn_inv(GEN f, long e, GEN T, ulong p)
{
  pari_sp av = avma, av2;
  ulong mask;
  GEN W, a;
  long v = varn(f), n = 1, vT = get_Flx_var(T);

  if (!signe(f)) pari_err_INV("FlxqXn_inv",f);
  a = Flxq_inv(gel(f,2), T, p);
  if (e == 1) return scalarpol(a, v);
  else if (e == 2)
  {
    GEN b;
    if (degpol(f) <= 0) return scalarpol(a, v);
    b = Flx_neg(gel(f,3), p);
    if (lgpol(b)==0) return scalarpol(a, v);
    b = Flxq_mul(b, Flxq_sqr(a, T, p), T, p);
    W = deg1pol_shallow(b, a, v);
    return gerepilecopy(av, W);
  }
  W = scalarpol_shallow(Flxq_inv(gel(f,2), T, p),v);
  mask = quadratic_prec_mask(e);
  av2 = avma;
  for (;mask>1;)
  {
    GEN u, fr;
    long n2 = n;
    n<<=1; if (mask & 1) n--;
    mask >>= 1;
    fr = FlxXn_red(f, n);
    u = FlxqXn_mul(W, FlxqXn_mulhigh(fr, W, n2, n, T, p), n-n2, T, p);
    W = FlxX_sub(W, FlxX_shift(u, n2, vT), p);
    if (gc_needed(av2,2))
    {
      if(DEBUGMEM>1) pari_warn(warnmem,"FlxqXn_inv, e = %ld", n);
      W = gerepileupto(av2, W);
    }
  }
  return gerepileupto(av, W);
}

/* Compute intformal(x^n*S)/x^(n+1) */
static GEN
FlxX_integXn(GEN x, long n, ulong p)
{
  long i, lx = lg(x);
  GEN y;
  if (lx == 2) return gcopy(x);
  y = cgetg(lx, t_POL); y[1] = x[1];
  for (i=2; i<lx; i++)
  {
    GEN xi = gel(x,i);
    gel(y,i) = Flx_Fl_mul(xi, Fl_inv((n+i-1)%p, p), p);
  }
  return FlxX_renormalize(y, lx);;
}

GEN
FlxqXn_expint(GEN h, long e, GEN T, ulong p)
{
  pari_sp av = avma, av2;
  long v = varn(h), n = 1, vT = get_Flx_var(T);
  GEN f = pol1_FlxX(v, vT), g = pol1_FlxX(v, vT);
  ulong mask = quadratic_prec_mask(e);
  av2 = avma;
  for (;mask>1;)
  {
    GEN u, w;
    long n2 = n;
    n<<=1; if (mask & 1) n--;
    mask >>= 1;
    u = FlxqXn_mul(g, FlxqX_mulhigh_i(f, FlxXn_red(h, n2-1), n2-1, T, p), n-n2, T, p);
    u = FlxX_add(u, FlxX_shift(FlxXn_red(h, n-1), 1-n2, vT), p);
    w = FlxqXn_mul(f, FlxX_integXn(u, n2-1, p), n-n2, T, p);
    f = FlxX_add(f, FlxX_shift(w, n2, vT), p);
    if (mask<=1) break;
    u = FlxqXn_mul(g, FlxqXn_mulhigh(f, g, n2, n, T, p), n-n2, T, p);
    g = FlxX_sub(g, FlxX_shift(u, n2, vT), p);
    if (gc_needed(av2,2))
    {
      if (DEBUGMEM>1) pari_warn(warnmem,"FlxqXn_exp, e = %ld", n);
      gerepileall(av2, 2, &f, &g);
    }
  }
  return gerepileupto(av, f);
}
