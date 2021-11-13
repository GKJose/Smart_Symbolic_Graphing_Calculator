/* Copyright (C) 2018  The PARI group.

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
/**                     BERNOULLI NUMBERS B_2k                     **/
/**                                                                **/
/********************************************************************/
#include "pari.h"
#include "paripriv.h"

/* D = divisorsu(n). Return a/b = \sum_{p-1 | 2n: p prime} 1/p
 * B_2k + a/b in Z [Clausen-von Staudt] */
static GEN
fracB2k(GEN D)
{
  GEN a = utoipos(5), b = utoipos(6); /* 1/2 + 1/3 */
  long i, l = lg(D);
  for (i = 2; i < l; i++) /* skip 1 */
  {
    ulong p = 2*D[i] + 1; /* a/b += 1/p */
    if (uisprime(p)) { a = addii(muliu(a,p), b); b = muliu(b,p); }
  }
  return mkfrac(a,b);
}
/* precision needed to compute B_k for all k <= N */
long
bernbitprec(long N)
{ /* 1.612086 ~ log(8Pi) / 2 */
  const double log2PI = 1.83787706641;
  double logN = log((double)N);
  double t = (N + 4) * logN - N*(1 + log2PI) + 1.612086;
  return (long)ceil(t / M_LN2) + 10;
}
static long
bernprec(long N) { return nbits2prec(bernbitprec(N)); }
/* \sum_{k > M} k^(-n) <= M^(1-n) / (n-1) < 2^-b */
static long
zetamaxpow(long n)
{
  long M = (long)ceil(n / (2 * M_PI * M_E));
  return M | 1; /* make it odd */
}
/* v * zeta(k) using 'max' precomputed odd powers */
static GEN
bern_zeta(GEN v, long k, GEN pow, long r, long p)
{
  GEN z, s = gel(pow, r);
  long j;
  for (j = r - 2; j >= 3; j -= 2) s = addii(s, gel(pow,j));
  z = s = itor(s, nbits2prec(p));
  shiftr_inplace(s, -p); /* zeta(k)(1 - 2^(-k)) - 1*/
  s = addrs(s, 1); shiftr_inplace(s, -k);
  /* divide by 1 - 2^(-k): s + s/2^k + s/2^(2k) + ... */
  for (; k < p; k <<= 1) s = addrr(s, shiftr(s, -k));
  return addrr(v, mulrr(v, addrr(z, s)));
}
#if 0
/* z * j^2 */
static GEN
mulru2(GEN z, ulong j)
{ return (j | HIGHMASK)? mulri(z, sqru(j)): mulru(z, j*j); }
#endif
/* z * j^2 */
static GEN
muliu2(GEN z, ulong j)
{ return (j | HIGHMASK)? mulii(z, sqru(j)): muliu(z, j*j); }
/* 1 <= m <= n, set y[1] = B_{2m}, ... y[n-m+1] = B_{2n} in Q */
static void
bernset(GEN *y, long m, long n)
{
  long i, j, k, p, prec, r, N = n << 1; /* up to B_N */
  GEN u, B, v, t;
  p = bernbitprec(N); prec = nbits2prec(p);
  u = sqrr(Pi2n(1, prec)); /* (2Pi)^2 */
  v = divrr(mpfactr(N, prec), powru(u, n)); shiftr_inplace(v,1);
  r = zetamaxpow(N);
  t = cgetg(r+1, t_VEC); B = int2n(p); /* fixed point */
  for (j = 3; j <= r; j += 2)
  {
    GEN z = cgeti(prec);
    pari_sp av2 = avma;
    affii(divii(B, powuu(j, N)), z);
    gel(t,j) = z; set_avma(av2);
  }
  y += n - m;
  for (i = n, k = N;; i--)
  { /* set B_n, k = 2i */
    pari_sp av2 = avma;
    GEN z = fracB2k(divisorsu(i)), B = bern_zeta(v, k, t, r, p);
    long j;
    /* B = v * zeta(k), v = 2*k! / (2Pi)^k */
    if (!odd(i)) setsigne(B, -1); /* B ~ B_n */
    B = roundr(addrr(B, fractor(z,LOWDEFAULTPREC))); /* B - z = B_n */
    *y-- = gclone(gsub(B, z));
    if (i == m) break;
    affrr(divrunu(mulrr(v,u), k-1), v);
    for (j = r; j >= 3; j -= 2) affii(muliu2(gel(t,j), j), gel(t,j));
    set_avma(av2); k -= 2;
    if ((k & 0xf) == 0)
    { /* reduce precision if possible */
      long p2 = p, prec2 = prec, r2;
      p = bernbitprec(k);
      prec = nbits2prec(p); if (prec2 == prec) continue;
      setprec(v, prec);
      r2 = zetamaxpow(k); if (r2 > r) continue;
      r = r2;
      for (j = 3; j <= r; j += 2) affii(shifti(gel(t,j), p - p2), gel(t,j));
      set_avma(av2);
    }
  }
}
/* need B[2..2*nb] as t_INT or t_FRAC */
void
constbern(long nb)
{
  const pari_sp av = avma;
  long i, l;
  GEN B;
  pari_timer T;

  l = bernzone? lg(bernzone): 0;
  if (l > nb) return;

  nb = maxss(nb, l + 127);
  B = cgetg_block(nb+1, t_VEC);
  if (bernzone)
  { for (i = 1; i < l; i++) gel(B,i) = gel(bernzone,i); }
  else
  {
    gel(B,1) = gclone(mkfracss(1,6));
    gel(B,2) = gclone(mkfracss(-1,30));
    gel(B,3) = gclone(mkfracss(1,42));
    gel(B,4) = gclone(mkfracss(-1,30));
    gel(B,5) = gclone(mkfracss(5,66));
    gel(B,6) = gclone(mkfracss(-691,2730));
    gel(B,7) = gclone(mkfracss(7,6));
    gel(B,8) = gclone(mkfracss(-3617,510));
    gel(B,9) = gclone(mkfracss(43867,798));
    gel(B,10)= gclone(mkfracss(-174611,330));
    gel(B,11)= gclone(mkfracss(854513,138));
    gel(B,12)= gclone(mkfracss(-236364091,2730));
    gel(B,13)= gclone(mkfracss(8553103,6)); /* B_26 */
    l = 14;
  }
  set_avma(av);
  if (DEBUGLEVEL) {
    err_printf("caching Bernoulli numbers 2*%ld to 2*%ld\n", l, nb);
    timer_start(&T);
  }
  bernset((GEN*)B + l, l, nb);
  if (DEBUGLEVEL) timer_printf(&T, "Bernoulli");
  swap(B, bernzone); guncloneNULL(B);
  set_avma(av);
  if (nb > 200000)
  {
    const ulong p = 4294967291UL;
    long n = 2 * nb + 2;
    GEN t = const_vecsmall(n+1, 1);
    t[1] = evalvarn(0); t[2] = 0;
    t = Flx_shift(Flx_invLaplace(t, p), -1); /* t = (exp(x)-1)/x */
    t = Flx_Laplace(Flxn_inv(t, n, p), p);
    for (i = 1; i <= nb; i++)
      if (Rg_to_Fl(bernfrac(2*i), p) != uel(t,2*i+2))
        pari_err_BUG(stack_sprintf("B_{2*%ld}", i));
    set_avma(av);
  }
}
/* Obsolete, kept for backward compatibility */
void
mpbern(long n, long prec) { (void)prec; constbern(n); }

/* assume n even > 0, if iz != NULL, assume iz = 1/zeta(n) */
static GEN
bernreal_using_zeta(long n, long prec)
{
  GEN pi2 = Pi2n(1, prec+EXTRAPRECWORD);
  GEN iz = inv_szeta_euler(n, prec);
  GEN z = divrr(mpfactr(n, prec), mulrr(powru(pi2, n), iz));
  shiftr_inplace(z, 1); /* 2 * n! * zeta(n) / (2Pi)^n */
  if ((n & 3) == 0) setsigne(z, -1);
  return z;
}
/* assume n even > 0, B = NULL or good approximation to B_n */
static GEN
bernfrac_i(long n, GEN B)
{
  pari_sp av = avma;
  GEN z = fracB2k(divisorsu(n >> 1));
  if (!B) B = bernreal_using_zeta(n, bernprec(n));
  B = roundr( gadd(B, fractor(z,LOWDEFAULTPREC)) );
  return gerepileupto(av, gsub(B, z));
}
GEN
bernfrac(long n)
{
  long k;
  if (n <= 1)
  {
    if (n < 0) pari_err_DOMAIN("bernfrac", "index", "<", gen_0, stoi(n));
    return n? mkfrac(gen_m1,gen_2): gen_1;
  }
  if (odd(n)) return gen_0;
  k = n >> 1;
  if (!bernzone) constbern(0);
  if (bernzone && k < lg(bernzone)) return gel(bernzone, k);
  return bernfrac_i(n, NULL);
}
GEN
bernvec(long n)
{
  long i, l;
  GEN y;
  if (n < 0) return cgetg(1, t_VEC);
  constbern(n);
  l = n+2; y = cgetg(l, t_VEC); gel(y,1) = gen_1;
  for (i = 2; i < l; i++) gel(y,i) = gel(bernzone,i-1);
  return y;
}

/* x := pol_x(v); B_k(x) = \sum_{i=0}^k binomial(k, i) B_i x^{k-i} */
static GEN
bernpol_i(long k, long v)
{
  GEN B, C;
  long i;
  if (v < 0) v = 0;
  constbern(k >> 1); /* cache B_2, ..., B_2[k/2] */
  C = vecbinomial(k);
  B = cgetg(k + 3, t_POL);
  for (i = 0; i <= k; ++i) gel(B, k-i+2) = gmul(gel(C,i+1), bernfrac(i));
  B[1] = evalsigne(1) | evalvarn(v);
  return B;
}
GEN
bernpol(long k, long v)
{
  pari_sp av = avma;
  if (k < 0) pari_err_DOMAIN("bernpol", "index", "<", gen_0, stoi(k));
  return gerepileupto(av, bernpol_i(k, v));
}
/* x := pol_x(v); return 1^e + ... + x^e = x^e + (B_{e+1}(x) - B_{e+1})/(e+1) */
static GEN
faulhaber(long e, long v)
{
  GEN B;
  if (e == 0) return pol_x(v);
  B = RgX_integ(bernpol_i(e, v)); /* (B_{e+1}(x) - B_{e+1}) / (e+1) */
  gel(B,e+2) = gaddgs(gel(B,e+2), 1); /* add x^e, in place */
  return B;
}
/* sum_v T(v), T a polynomial expression in v */
GEN
sumformal(GEN T, long v)
{
  pari_sp av = avma, av2;
  long i, t, d;
  GEN R;

  T = simplify_shallow(T);
  t = typ(T);
  if (is_scalar_t(t))
    return gerepileupto(av, monomialcopy(T, 1, v < 0? 0: v));
  if (t != t_POL) pari_err_TYPE("sumformal [not a t_POL]", T);
  if (v < 0) v = varn(T);
  av2 = avma;
  R = gen_0;
  d = poldegree(T,v);
  for (i = d; i >= 0; i--)
  {
    GEN c = polcoef(T, i, v);
    if (gequal0(c)) continue;
    R = gadd(R, gmul(c, faulhaber(i, v)));
    if (gc_needed(av2,3))
    {
      if(DEBUGMEM>1) pari_warn(warnmem,"sumformal, i = %ld/%ld", i,d);
      R = gerepileupto(av2, R);
    }
  }
  return gerepileupto(av, R);
}

/* 1/zeta(n) using Euler product. Assume n > 0. */
GEN
inv_szeta_euler(long n, long prec)
{
  GEN z, res;
  pari_sp av, av2;
  double A, D, lba;
  ulong p, lim;
  forprime_t S;

  if (n > prec2nbits(prec)) return real_1(prec);

  lba = prec2nbits_mul(prec, M_LN2);
  D = exp((lba - log((double)(n-1))) / (n-1));
  lim = 1 + (ulong)ceil(D);
  if (lim < 3) return subir(gen_1,real2n(-n,prec));
  res = cgetr(prec); incrprec(prec);
  av = avma;
  z = subir(gen_1, real2n(-n, prec));

  (void)u_forprime_init(&S, 3, lim);
  av2 = avma; A = n / M_LN2;
  while ((p = u_forprime_next(&S)))
  {
    long l = prec2nbits(prec) - (long)floor(A * log((double)p)) - BITS_IN_LONG;
    GEN h;

    if (l < BITS_IN_LONG) l = BITS_IN_LONG;
    l = minss(prec, nbits2prec(l));
    h = divrr(z, rpowuu(p, (ulong)n, l));
    z = subrr(z, h);
    if (gc_needed(av,1))
    {
      if (DEBUGMEM>1) pari_warn(warnmem,"inv_szeta_euler, p = %lu/%lu", p,lim);
      z = gerepileuptoleaf(av2, z);
    }
  }
  affrr(z, res); set_avma(av); return res;
}

/* Return B_n */
GEN
bernreal(long n, long prec)
{
  GEN B;
  long p, k;
  if (n < 0) pari_err_DOMAIN("bernreal", "index", "<", gen_0, stoi(n));
  if (n == 0) return real_1(prec);
  if (n == 1) return real_m2n(-1,prec); /*-1/2*/
  if (odd(n)) return real_0(prec);

  k = n >> 1;
  if (!bernzone) constbern(0);
  if (k < lg(bernzone)) return fractor(gel(bernzone,k), prec);
  p = bernprec(n);
  B = bernreal_using_zeta(n, minss(p, prec));
  if (p < prec) B = fractor(bernfrac_i(n, B), prec);
  return B;
}

GEN
eulerpol(long k, long v)
{
  pari_sp av = avma;
  GEN B, E;
  if (k < 0) pari_err_DOMAIN("eulerpol", "index", "<", gen_0, stoi(k));
  k++; B = bernpol_i(k, v);
  E = RgX_Rg_mul(RgX_sub(B, RgX_rescale(B, gen_2)), sstoQ(2,k));
  return gerepileupto(av, E);
}
GEN
eulervec(long n)
{
  pari_sp av;
  GEN v, E, C;
  long k;
  if (n < 0) return cgetg(1, t_VEC);
  C = vecbinomial(2*n);
  E = ZX_translate(RgX_rescale(eulerpol(2*n, 0), gen_2), gen_1);
  av = avma; v = cgetg(n + 2, t_VEC); gel(v,1) = gen_1;
  /* 2^(2n) E_2n(x/2 + 1) = sum_k binomial(2n,k) E_k x^(n-k) */
  for (k = 1; k <= n; k++)
    gel(v,k+1) = diviiexact(gel(E,2*n-2*k+2), gel(C,2*k+1));
  return gerepileupto(av, v);
}
