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
