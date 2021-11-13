/* Copyright (C) 2015  The PARI group.

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
/**           Dirichlet series through Euler product               **/
/**                                                                **/
/********************************************************************/
#include "pari.h"
#include "paripriv.h"

static void
err_direuler(GEN x)
{ pari_err_DOMAIN("direuler","constant term","!=", gen_1,x); }

/* s = t_POL (tolerate t_SER of valuation 0) of constant term = 1
 * d = minimal such that p^d > X
 * V indexed by 1..X will contain the a_n
 * v[1..n] contains the indices nj such that V[nj] != 0 */
static long
dirmuleuler_small(GEN V, GEN v, long n, ulong p, GEN s, long d)
{
  long i, j, m = n, D = minss(d+2, lg(s));
  ulong q = 1, X = lg(V)-1;

  for (i = 3, q = p; i < D; i++, q *= p) /* q*p does not overflow */
  {
    GEN aq = gel(s,i);
    if (gequal0(aq)) continue;
    /* j = 1 */
    gel(V,q) = aq;
    v[++n] = q;
    for (j = 2; j <= m; j++)
    {
      ulong nj = umuluu_le(uel(v,j), q, X);
      if (!nj) continue;
      gel(V,nj) = gmul(aq, gel(V,v[j]));
      v[++n] = nj;
    }
  }
  return n;
}

/* ap != 0 for efficiency, p > sqrt(X) */
static void
dirmuleuler_large(GEN V, ulong p, GEN ap)
{
  long j, jp, X = lg(V)-1;
  gel(V,p) = ap;
  for (j = 2, jp = 2*p; jp <= X; j++, jp += p) gel(V,jp) = gmul(ap, gel(V,j));
}

static ulong
direulertou(GEN a, GEN fl(GEN))
{
  if (typ(a) != t_INT)
  {
    a = fl(a);
    if (typ(a) != t_INT) pari_err_TYPE("direuler", a);
  }
  return signe(a)<=0 ? 0: itou(a);
}

static GEN
direuler_Sbad(GEN V, GEN v, GEN Sbad, ulong *n)
{
  long i, l = lg(Sbad);
  ulong X = lg(V)-1;
  GEN pbad = gen_1;
  for (i = 1; i < l; i++)
  {
    GEN ai = gel(Sbad,i);
    ulong q;
    if (typ(ai) != t_VEC || lg(ai) != 3)
      pari_err_TYPE("direuler [bad primes]",ai);
    q = gtou(gel(ai,1));
    if (q <= X)
    {
      long d = ulogint(X, q) + 1;
      GEN s = direuler_factor(gel(ai,2), d);
      *n = dirmuleuler_small(V, v, *n, q, s, d);
      pbad = muliu(pbad, q);
    }
  }
  return pbad;
}

GEN
direuler_bad(void *E, GEN (*eval)(void *,GEN,long), GEN a,GEN b,GEN c, GEN Sbad)
{
  ulong au, bu, X, sqrtX, n, p;
  pari_sp av0 = avma;
  GEN gp, v, V;
  forprime_t T;
  au = direulertou(a, gceil);
  bu = direulertou(b, gfloor);
  X = c ? direulertou(c, gfloor): bu;
  if (X == 0) return cgetg(1,t_VEC);
  if (bu > X) bu = X;
  if (!u_forprime_init(&T, au, bu)) { set_avma(av0); return mkvec(gen_1); }
  v = vecsmall_ei(X, 1);
  V = vec_ei(X, 1);
  n = 1;
  if (Sbad) Sbad = direuler_Sbad(V, v, Sbad, &n);
  p = 1; gp = cgetipos(3); sqrtX = usqrt(X);
  while (p <= sqrtX && (p = u_forprime_next(&T)))
    if (!Sbad || umodiu(Sbad, p))
    {
      long d = ulogint(X, p) + 1; /* minimal d such that p^d > X */
      GEN s;
      gp[2] = p; s = eval(E, gp, d);
      n = dirmuleuler_small(V, v, n, p, s, d);
    }
  while ((p = u_forprime_next(&T))) /* sqrt(X) < p <= X */
    if (!Sbad || umodiu(Sbad, p))
    {
      GEN s;
      gp[2] = p; s = eval(E, gp, 2); /* s either t_POL or t_SER of val 0 */
      if (lg(s) > 3 && !gequal0(gel(s,3)))
        dirmuleuler_large(V, p, gel(s,3));
    }
  return gerepilecopy(av0,V);
}

/* return a t_SER or a truncated t_POL to precision n */
GEN
direuler_factor(GEN s, long n)
{
  long t = typ(s);
  if (is_scalar_t(t))
  {
    if (!gequal1(s)) err_direuler(s);
    return scalarpol_shallow(s,0);
  }
  switch(t)
  {
    case t_POL: break; /* no need to RgXn_red */
    case t_RFRAC:
    {
      GEN p = gel(s,1), q = gel(s,2);
      q = RgXn_red_shallow(q,n);
      s = RgXn_inv(q, n);
      if (typ(p) == t_POL && varn(p) == varn(q))
      {
        p = RgXn_red_shallow(p, n);
        s = RgXn_mul(s, p, n);
      }
      else
        if (!gequal1(p)) s = RgX_Rg_mul(s, p);
      if (!signe(s) || !gequal1(gel(s,2))) err_direuler(s);
      break;
    }
    case t_SER:
      if (!signe(s) || valp(s) || !gequal1(gel(s,2))) err_direuler(s);
      break;
    default: pari_err_TYPE("direuler", s);
  }
  return s;
}

struct eval_bad
{
  void *E;
  GEN (*eval)(void *, GEN);
};
static GEN
eval_bad(void *E, GEN p, long n)
{
  struct eval_bad *d = (struct eval_bad*) E;
  return direuler_factor(d->eval(d->E, p), n);
}
GEN
direuler(void *E, GEN (*eval)(void *, GEN), GEN a, GEN b, GEN c)
{
  struct eval_bad d;
  d.E= E; d.eval = eval;
  return direuler_bad((void*)&d, eval_bad, a, b, c, NULL);
}

static GEN
primelist(forprime_t *T, GEN Sbad, long n, long *running)
{
  GEN P = cgetg(n+1, t_VECSMALL);
  long i, j;
  for (i = 1, j = 1; i <= n; i++)
  {
    ulong p = u_forprime_next(T);
    if (!p) { *running = 0; break; }
    if (Sbad && umodiu(Sbad, p)==0) continue;
    uel(P,j++) = p;
  }
  setlg(P, j);
  return P;
}

GEN
pardireuler(GEN worker, GEN a, GEN b, GEN c, GEN Sbad)
{
  ulong au, bu, X, sqrtX, n, snX, nX;
  pari_sp av0 = avma;
  GEN v, V;
  forprime_t T;
  struct pari_mt pt;
  long running = 1, pending = 0;
  au = direulertou(a, gceil);
  bu = direulertou(b, gfloor);
  X = c ? direulertou(c, gfloor): bu;
  if (X == 0) return cgetg(1,t_VEC);
  if (bu > X) bu = X;
  if (!u_forprime_init(&T, au, bu)) { set_avma(av0); return mkvec(gen_1); }
  v = vecsmall_ei(X, 1);
  V = vec_ei(X, 1);
  n = 1;
  if (Sbad) Sbad = direuler_Sbad(V, v, Sbad, &n);
  sqrtX = usqrt(X); snX = uprimepi(sqrtX); nX = uprimepi(X);
  if (snX)
  {
    GEN P = primelist(&T, Sbad, snX, &running);
    GEN R = gel(closure_callgenvec(worker, mkvec2(P, utoi(X))), 2);
    long i, l = lg(P);
    for (i = 1; i < l; i++)
    {
      GEN s = gel(R,i);
      n = dirmuleuler_small(V, v, n, uel(P,i), s, lg(s));
    }
  } else snX = 1;
  mt_queue_start_lim(&pt, worker, (nX+snX-1)/snX);
  while (running || pending)
  {
    GEN done;
    GEN P = running? primelist(&T, Sbad, snX, &running): NULL;
    mt_queue_submit(&pt, 0, P ? mkvec2(P, utoi(X)): NULL);
    done = mt_queue_get(&pt, NULL, &pending);
    if (done)
    {
      GEN P = gel(done,1), R = gel(done,2);
      long j, l = lg(P);
      for (j=1; j<l; j++)
      {
        GEN F = gel(R,j);
        if (degpol(F) && !gequal0(gel(F,3)))
          dirmuleuler_large(V, uel(P,j), gel(F,3));
      }
    }
  }
  mt_queue_end(&pt);
  return gerepilecopy(av0,V);
}

/********************************************************************/
/**                                                                **/
/**                 DIRPOWERS and DIRPOWERSSUM                     **/
/**                                                                **/
/********************************************************************/

/* [1^B,...,N^B] */
GEN
vecpowuu(long N, ulong B)
{
  GEN v;
  long p, i;
  forprime_t T;

  if (B <= 8000)
  {
    if (!B) return const_vec(N,gen_1);
    v = cgetg(N+1, t_VEC); if (N == 0) return v;
    gel(v,1) = gen_1;
    if (B == 1)
      for (i = 2; i <= N; i++) gel(v,i) = utoipos(i);
    else if (B == 2)
    {
      ulong o, s;
      if (N & HIGHMASK)
        for (i = 2, o = 3; i <= N; i++, o += 2)
          gel(v,i) = addiu(gel(v,i-1), o);
      else
        for (i = 2, s = 1, o = 3; i <= N; i++, s += o, o += 2)
          gel(v,i) = utoipos(s + o);
    }
    else if (B == 3)
      for (i = 2; i <= N; i++) gel(v,i) = powuu(i, B);
    else
    {
      long k, Bk, e = expu(N);
      for (i = 3; i <= N; i += 2) gel(v,i) = powuu(i, B);
      for (k = 1; k <= e; k++)
      {
        N >>= 1; Bk = B * k;
        for (i = 1; i <= N; i += 2) gel(v, i << k) = shifti(gel(v, i), Bk);
      }
    }
    return v;
  }
  v = const_vec(N, NULL);
  u_forprime_init(&T, 3, N);
  while ((p = u_forprime_next(&T)))
  {
    long m, pk, oldpk;
    gel(v,p) = powuu(p, B);
    for (pk = p, oldpk = p; pk; oldpk = pk, pk = umuluu_le(pk,p,N))
    {
      if (pk != p) gel(v,pk) = mulii(gel(v,oldpk), gel(v,p));
      for (m = N/pk; m > 1; m--)
        if (gel(v,m) && m%p) gel(v, m*pk) = mulii(gel(v,m), gel(v,pk));
    }
  }
  gel(v,1) = gen_1;
  for (i = 2; i <= N; i+=2)
  {
    long vi = vals(i);
    gel(v,i) = shifti(gel(v,i >> vi), B * vi);
  }
  return v;
}
/* [1^B,...,N^B] */
GEN
vecpowug(long N, GEN B, long prec)
{
  GEN v, pow = NULL;
  long p, precp = 2, eB, prec0;
  forprime_t T;
  if (N == 1) return mkvec(gen_1);
  eB = gexpo(B);
  prec0 = eB < 5? prec: prec + nbits2extraprec(eB);
  u_forprime_init(&T, 2, N);
  v = const_vec(N, NULL);
  gel(v,1) = gen_1;
  while ((p = u_forprime_next(&T)))
  {
    long m, pk, oldpk;
    if (!pow)
      pow = gpow(utor(p,prec0), B, prec);
    else
    {
      GEN t = gpow(divru(utor(p,prec0), precp), B, prec);
      pow = gmul(pow, t); /* (p / precp)^B * precp^B */
    }
    precp = p;
    gel(v,p) = pow; /* p^B */
    if (prec0 != prec) gel(v,p) = gprec_wtrunc(gel(v,p), prec);
    for (pk = p, oldpk = p; pk; oldpk = pk, pk = umuluu_le(pk,p,N))
    {
      if (pk != p) gel(v,pk) = gmul(gel(v,oldpk), gel(v,p));
      for (m = N/pk; m > 1; m--)
        if (gel(v,m) && m%p) gel(v, m*pk) = gmul(gel(v,m), gel(v,pk));
    }
  }
  return v;
}

GEN
dirpowers(long n, GEN x, long prec)
{
  pari_sp av = avma;
  GEN v;
  if (n <= 0) return cgetg(1, t_VEC);
  if (typ(x) == t_INT && lgefint(x) <= 3 && signe(x) >= 0)
  {
    ulong B = itou(x);
    v = vecpowuu(n, B);
    if (B <= 2) return v;
  }
  else v = vecpowug(n, x, prec);
  return gerepilecopy(av, v);
}

/* P = prime divisors of (squarefree) n */
static GEN
smallfact(ulong n, GEN P, ulong sq, GEN V)
{
  long i, l = lg(P);
  ulong p;
  GEN c;
  if (l == 1) return gen_1;
  p = uel(P,l - 1); if (p > sq) return NULL;
  c = gel(V, p);
  for (i = l-2; i > 0; i--)
  {
    n /= p; if (n <= sq) return gmul(c, gel(V,n));
    p = uel(P, i); c = gmul(c, gel(V,p));
  }
  return c;
}
/* sum_{n <= N} n^s. */
GEN
dirpowerssum(ulong N, GEN s, long prec)
{
  const ulong step = 2048;
  pari_sp av = avma, av2;
  GEN P, V, W, F, c2, c3, c6, c12, c123, c1234, tmp, S;
  forprime_t T;
  ulong x1, n, sq, p, precp;
  long prec0;

  if (!N) return gen_0;
  if (N < 9UL) return gerepileupto(av, RgV_sum(dirpowers(N, s, prec)));
  sq = usqrt(N);
  V = cgetg(sq+1, t_VEC);
  W = cgetg(sq+1, t_VEC);
  F = cgetg(sq+1, t_VEC);
  prec0 = prec + EXTRAPRECWORD;
  s = gprec_w(s, prec0);
  gel(V,1) = gel(W,1) = gel(F,1) = gen_1;
  for (n = 2; n <= sq; n++)
  {
    GEN t = divru(utor(n, prec0), n-1);
    gel(V,n) = gmul(gel(V,n-1), gpow(t, s, prec0));
    gel(W,n) = gadd(gel(W,n-1), gel(V,n));
    gel(F,n) = gadd(gel(F,n-1), gsqr(gel(V,n)));
  }
  c2 = gel(V,2); c3 = gel(V,3); c6 = gmul(c2, c3);
  c12 = gaddgs(c2, 1);
  c123 = gadd(c12, c3);
  c1234 = gadd(gel(F,2), gadd(c2,c3));
  precp = 0; tmp = NULL; S = gen_0;
  u_forprime_init(&T, sq + 1, N);
  av2 = avma;
  while ((p = u_forprime_next(&T)))
  {
    GEN t = utor(p, prec0);
    if (precp) t = divru(t, precp);
    t = gpow(t, s, prec0);
    tmp = precp? gmul(tmp, t): t; /* p^s */
    S = gadd(S, gmul(gel(W, N / p), tmp));
    precp = p;
    if ((p & 0x1ff) == 1) S = gerepileupto(av2, S);
  }
  P = mkvecsmall2(2, 3);
  av2 = avma;
  for(x1 = 1;; x1 += step)
  { /* beware overflow, fuse last two bins (avoid a tiny remainder) */
    ulong j, lv, x2 = (N >= 2*step && N - 2*step >= x1)? x1-1 + step: N;
    GEN v = vecfactorsquarefreeu_coprime(x1, x2, P);
    lv = lg(v);
    for (j = 1; j < lv; j++) if (gel(v,j))
    {
      ulong d = x1-1 + j; /* squarefree, coprime to 6 */
      ulong q;
      tmp = smallfact(d, gel(v,j), sq, V);
      if (!tmp) continue;
      q = N / d;
      switch(q = N / d)
      {
        case 1: break;
        case 2: tmp = gmul(tmp, c12); break;
        case 3: tmp = gmul(tmp, c123); break;
        case 4:
        case 5: tmp = gmul(tmp, c1234); break;
        default:
        {
          GEN a = gel(F, usqrt(q)), b = gel(F, usqrt(q / 2));
          GEN c = gel(F, usqrt(q / 3)), d = gel(F, usqrt(q / 6));
          tmp = gmul(tmp, gadd(gadd(a, gmul(c2, b)),
                               gadd(gmul(c3, c), gmul(c6, d))));
        }
      }
      S = gadd(S, tmp);
    }
    if (x2 == N) break;
    S = gerepileupto(av2, S);
  }
  return gerepilecopy(av, S);
}
GEN
dirpowerssum0(GEN N, GEN s, long prec)
{
  if (typ(N) != t_INT) pari_err_TYPE("dirpowerssum", N);
  if (signe(N) <= 0) return gen_0;
  return dirpowerssum(itou(N), s, prec);
}
