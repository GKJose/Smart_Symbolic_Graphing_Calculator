# Makefile for Pari/GP -- i386 running darwin (x86-64/GMP kernel) 64-bit version
#
#  This file was created by Configure. Any change made to it will be
#  lost when Configure is run.
#
TOPDIR="/Users/julianalvarez/Documents/VSCODE/Smart_Symbolic_Graphing_Calculator/dependencies/pari-2.13.3"

SHELL      = /bin/sh
PERL       = /usr/local/bin/perl
BISON      = bison
AR         = ar
RANLIB     = ranlib

SIZEOF_LONG= 8

CC_FLAVOR  =
CC         = /usr/bin/gcc $(CC_FLAVOR)
CPPFLAGS   = -I. -I../src/headers
CFLAGS     = -O3 -Wall -fno-strict-aliasing -fomit-frame-pointer   
DLCFLAGS   = -fPIC
KERNELCFLAGS  = -funroll-loops
LD_FLAVOR  = $(CC_FLAVOR)
LD         = /usr/bin/gcc $(LD_FLAVOR)
LDFLAGS    = -O3 -Wall -fno-strict-aliasing -fomit-frame-pointer    -Wl,-search_paths_first 
DLLD_FLAVOR  = $(LD_FLAVOR)
DLLD       = /usr/bin/gcc $(DLLD_FLAVOR)
DLLDFLAGS  = -dynamiclib  $(CFLAGS) $(DLCFLAGS) -Wl,-compatibility_version,2.13.0,-current_version,2.13.3 
EXTRADLLDFLAGS = 
RUNTEST    = 

# HIGHLY EXPERIMENTAL (only tested with gmp-4.0 on ix86 and Ultra).
# If you've configured and compiled GMP and would like to tune PARI using
# the nice cycle counting functions in GMP, uncomment the 4 lines below
# (correct the first one to the path to your gmp source tree).
#
#GMP     = /some/directory/gmp-4.0/tune
#GMPFLAGS= -DGMP_TIMER
#GMPO1 = $(GMP)/time.o $(GMP)/freq.o
#GMPOBJS=$(GMPO1) $(GMPO2)
#
#You may need to add a few object files to GMPOBJS. On UltraSparc, uncomment
#the following line
#GMPO2 = $(GMP)/sparcv9.o

_O	   = .o
_A	   = .a
LIB_PREFIX = lib
DLLIB_PREFIX = lib
LIBPARI_BASE = pari-gmp
LIBPARI      = $(LIB_PREFIX)pari
DLLIBPARI    = $(DLLIB_PREFIX)pari
LIBPARI_STA  = $(LIBPARI)$(_A)
LIBPARI_SO   = $(DLLIBPARI).dylib
SOLIBPARI    = $(DLLIB_PREFIX)$(LIBPARI_BASE).dylib
LIBPARI_DYN   = $(SOLIBPARI)
LIBPARI_SONAME= $(SOLIBPARI)
DL_DFLT_NAME = NULL

LD_INSTALL_NAME  = -Wl,-install_name -Wl,"/Users/julianalvarez/Documents/VSCODE/Smart_Symbolic_Graphing_Calculator/tools/macos/lib"/$(LIBPARI_DYN)

EXPORT_FILE   = 
EXPORT_EXE    = 
EXPORT_CREATE = 
EXPORT_LIB    = 
EXPORT_LIB_CREATE = 
DLLD_IGNORE   = 
DLLTOOL = 

RUNPTH       = 
RUNPTH_FINAL = 
LDDYN        = -lpari
LIBS         = 
GMPLIBS      = -L/Users/julianalvarez/Documents/VSCODE/Smart_Symbolic_Graphing_Calculator/tools/macos/lib -lgmp
MT_LIBS      = 
RT_LIBS      = 
DL_LIBS      = 
DYN_LIBS     = $(GMPLIBS) $(DL_LIBS) $(RT_LIBS) $(MT_LIBS) $(LIBS)
STA_LIBS     = $(GMPLIBS) $(DL_LIBS) $(RT_LIBS) $(MT_LIBS) $(LIBS)

RM = rm -f
MV = mv -f
LN = ln -s
CP_F = cp -f
STRIP      = strip
STRIPFLAGS =

# Change these installation directories to suit your needs.
# DESTDIR is used to install to a false hierachy (to build a Debian package)
INCLUDEDIR= "$(DESTDIR)/Users/julianalvarez/Documents/VSCODE/Smart_Symbolic_Graphing_Calculator/tools/macos/include"
LIBDIR    = "$(DESTDIR)/Users/julianalvarez/Documents/VSCODE/Smart_Symbolic_Graphing_Calculator/tools/macos/lib"
BINDIR    = "$(DESTDIR)/Users/julianalvarez/Documents/VSCODE/Smart_Symbolic_Graphing_Calculator/tools/macos/bin"
MANDIR    = "$(DESTDIR)/Users/julianalvarez/Documents/VSCODE/Smart_Symbolic_Graphing_Calculator/tools/macos/share/man/man1"
DATADIR   = "$(DESTDIR)/Users/julianalvarez/Documents/VSCODE/Smart_Symbolic_Graphing_Calculator/tools/macos/share/pari"
SYSDATADIR= "$(DESTDIR)/Users/julianalvarez/Documents/VSCODE/Smart_Symbolic_Graphing_Calculator/tools/macos/lib/pari"

EXDIR     = $(DATADIR)/examples
MISCDIR   = $(DATADIR)/misc
DOCDIR    = $(DATADIR)/doc

INSTALL = ../config/install
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA = $(INSTALL) -m 644
TAR = tar

# Readline
RLINCLUDE = 
RLLIBS    = 
# GMP
GMPINCLUDE = -I/Users/julianalvarez/Documents/VSCODE/Smart_Symbolic_Graphing_Calculator/tools/macos/include
# Graphic library.
QTMOC      = 
QTINC      = 
QTLIB      = 
PLOTCFLAGS =  -I/usr/local/include
PLOTLIBS   =  -lX11 
CXX        = g++

TOPLDDYN   = "/Users/julianalvarez/Documents/VSCODE/Smart_Symbolic_Graphing_Calculator/dependencies/pari-2.13.3/Odarwin-i386"
# Description system
DESC          = pari.desc
DESC_HELP_GEN = ../src/desc/gen_proto ../src/desc/PARI/822.pm
TMPSUF        = darwin-i386.tmp

DOTEST=env "RUNTEST=$(RUNTEST)" "LD_LIBRARY_PATH=.:$$LD_LIBRARY_PATH" $(SHELL) ../src/test/dotest 64 ""

OBJS   =  mpker$(_O) mpinl$(_O) F2v$(_O) F2x$(_O) F2xqE$(_O) FF$(_O) Fle$(_O) Flv$(_O) Flx$(_O) FlxX$(_O) FlxqE$(_O) Flxq_log$(_O) FpE$(_O) FpV$(_O) FpX$(_O) FpXQX_factor$(_O) FpXX$(_O) FpX_factor$(_O) Hensel$(_O) QX_factor$(_O) Qfb$(_O) RgV$(_O) RgX$(_O) Ser$(_O) ZG$(_O) ZV$(_O) ZX$(_O) alglin1$(_O) alglin2$(_O) alglin3$(_O) aprcl$(_O) arith1$(_O) arith2$(_O) base1$(_O) base2$(_O) base3$(_O) base4$(_O) base5$(_O) bb_group$(_O) bb_hnf$(_O) bern$(_O) bibli1$(_O) bibli2$(_O) bit$(_O) bnflog$(_O) bnfunits$(_O) buch1$(_O) buch2$(_O) buch3$(_O) buch4$(_O) char$(_O) concat$(_O) crvwtors$(_O) dirichlet$(_O) ecpp$(_O) ellanal$(_O) elliptic$(_O) ellisog$(_O) ellpadic$(_O) ellpadiclambdamu$(_O) ellsea$(_O) elltors$(_O) galconj$(_O) gen1$(_O) gen2$(_O) gen3$(_O) hnf_snf$(_O) hyperell$(_O) hypergeom$(_O) ifactor1$(_O) kummer$(_O) lfun$(_O) lfunquad$(_O) lfunutils$(_O) lll$(_O) map$(_O) matperm$(_O) mellininv$(_O) mftrace$(_O) modsym$(_O) msfarey$(_O) nffactor$(_O) perm$(_O) polarit1$(_O) polarit2$(_O) polarit3$(_O) polclass$(_O) polmodular$(_O) prime$(_O) qfisom$(_O) qfsolve$(_O) random$(_O) rootpol$(_O) subcyclo$(_O) subfield$(_O) subgroup$(_O) trans1$(_O) trans2$(_O) trans3$(_O) volcano$(_O) zetamult$(_O) anal$(_O) compile$(_O) default$(_O) es$(_O) eval$(_O) forprime$(_O) gplib$(_O) hash$(_O) init$(_O) intnum$(_O) members$(_O) paricfg$(_O) pariinl$(_O) parse$(_O) readline$(_O) str$(_O) sumiter$(_O) algebras$(_O) elldata$(_O) ellfromeqn$(_O) forperm$(_O) forsubset$(_O) galois$(_O) galpol$(_O) genus2red$(_O) groupid$(_O) krasner$(_O) mpqs$(_O) part$(_O) ratpoints$(_O) stark$(_O) thue$(_O) darwin$(_O) mt$(_O) single$(_O) plotport$(_O) plottty$(_O)
OBJSGP =  emacs$(_O) gp$(_O) gp_rl$(_O) texmacs$(_O) whatnow$(_O) plotX$(_O)

TESTS  = addprimes agm algebras alggroup alghasse alglattices algsplit analyz apply arith asymp aurifeuille bbhnf bern bessel bestappr binomial bit bnf bnfisintnorm bnflog bnfsunit bnr bnrL1 bnrclassfield bnrisgalois bnrmap character characteristic charpoly chinese cmp combinat compat concat content contfrac cxtrigo cyclo debugger deriv det diffop digits dirmul dirpowers disc div divisors ell ellanal ellff ellglobalred elliptic ellisogeny ellisomat ellissupersingular ellmodulareqn ellnf ellpadic ellpadiclambdamu ellratpoints ellsea ellseaJ elltors ellweilpairing equal err eval exact0 export extract factor factorff factorint factormod ff ffisom fflog fft forperm forsubset forvec galois galoischartable galoisinit galpol gamma gammamellininv gcdext genus2red graph help history hyperell hypergeom ideal idealappr idealramgroups incgam interpol intnum ispower isprime iterator krasner lambert language lex lfun lfunartin lfunquad lfuntype lift lindep linear list lll log logint map mat mathnf matpermanent matsnf member memory mf mfgaloisrep minim minmax modfun modpr modsym modular mscosets mspadic mspolygon multiif multivar-mul nf nfcompositum nfdiscfactors nfeltembed nfeltsign nffactor nfhilbert nfields nfislocalpower nfpolsturm nfrootsof1 nfsplitting norm number objets op orthopol padic partition perm plotexport pol polclass polmod polmodular polred polygonal polylog polyser pow prec prime print printf program programming qf qfb qfbclassno qfbsolve qfisom qfsolve quad quadclassunit quaddisc quadray ramanujantau random ranges ratpoints real resultant rfrac rnf rnfkummer rootsof1 rootsreal round round4 select self ser set size solve sort sqrtn stark str subcyclo subfields subgroup subst sumdedekind sumdiv sumformal sumiter sumnum sumnumrat thue trans trans2 valuation vararg variable version whatnow zeta zetahurwitz zetamult zn zncoppersmith

.PHONY: gp dft clean

dft: gp-sta ../gp 

gp: gp-sta ../gp 

../gp: gp-sta
	-$(RM) $@
	-$(LN) Odarwin-i386/gp-sta $@

all: gp-sta gp-dyn lib-sta

lib-sta: $(LIBPARI_STA)
lib-dyn: $(LIBPARI_DYN)

$(LIBPARI_STA): ../src/funclist $(OBJS)
	-$(RM) $@
	$(AR) r $@ $(OBJS)
	-$(RANLIB) $@

kerntest$(_O): ../src/test/kerntest.c
	$(CC) -c -I../src/language $(CPPFLAGS) $(CFLAGS) -o $@ ../src/test/kerntest.c

dummy$(_O): ../src/test/dummy.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ ../src/test/dummy.c

kerntest:  mpker$(_O) mpinl$(_O) dummy$(_O) kerntest$(_O)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@  mpker$(_O) mpinl$(_O) dummy$(_O) kerntest$(_O) $(STA_LIBS)

mpinl$(_O): .headers parilvl0.h parilvl1.h ../src/kernel/none/mpinl.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $(DLCFLAGS) -o mpinl$(_O) ../src/kernel/none/mpinl.c

test-kernel:: kerntest
	@./kerntest > gp.out;	diff -c gp.out ../src/test/32/kernel > kern.dif;	if test -s kern.dif; then echo "KERNEL BUG"; else echo OK; fi

tune$(_O): .headers ../src/test/tune.c
	$(CC) $(GMPFLAGS) $(CPPFLAGS) $(CFLAGS) -o $@ ../src/test/tune.c -c

tune: mpinl.h tune-sta
	-$(RM) tune ../tune
	-$(LN) tune-sta tune
	-$(LN) Odarwin-i386/tune ../tune

tune-dyn: tune$(_O) $(LIBPARI_DYN)
	$(LD) -L. $(LDFLAGS) $(RUNPTH) -o $@ $< $(GMPOBJS) $(LDDYN) $(STA_LIBS)
tune-sta: tune$(_O) $(LIBPARI_STA)
	$(LD) $(LDFLAGS) $(RUNPTH) -o $@ $< $(GMPOBJS) ./$(LIBPARI_STA) $(STA_LIBS)

gp-sta: ../src/funclist $(OBJS) $(OBJSGP) $(EXPORT_EXE)
	$(RM) $@
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(OBJSGP) $(RUNPTH) $(RLLIBS) $(PLOTLIBS) $(STA_LIBS) $(EXPORT_EXE)
	: gp-sta

clean_pari_ps::
	@-$(RM) pari.ps
cleantest:
	$(RM) *.dif gp.out io-testfile pari.ps
cleanobj:
	-$(RM) *$(_O) mpker.c *.s parimt.h mpinl.h parilvl0.h parilvl1.h libpari* gp-sta gp-dyn kerntest
	-$(RM) gmon.out
	-$(RM) *.gcno *.gcda
cleandesc:
	-$(RM) ../src/desc/$(DESC) *$(TMPSUF)
cleantune:
	-$(RM) tune tune-sta tune$(_O)

cleanall: cleanobj cleantune cleantest cleandesc

clean: cleanall


# Use this version to avoid problems with NFS and slightly out of synch
# fileserver/host. We are recompiling everything anyway. Not on by default:
# 3s is slower than the whole compilation on our development server :-)
# .headers: mpinl.h parimt.h ../src/headers/pari.h ../src/headers/paricast.h ../src/headers/paricom.h ../src/headers/parierr.h ../src/headers/parigen.h ../src/headers/pariinl.h ../src/headers/parinf.h ../src/headers/pariold.h ../src/headers/paristio.h ../src/headers/parisys.h ../src/headers/paritune.h
#	@sleep 3; touch $@
.headers: mpinl.h parimt.h ../src/headers/pari.h ../src/headers/paricast.h ../src/headers/paricom.h ../src/headers/parierr.h ../src/headers/parigen.h ../src/headers/pariinl.h ../src/headers/parinf.h ../src/headers/pariold.h ../src/headers/paristio.h ../src/headers/parisys.h ../src/headers/paritune.h
	@touch $@

install-nodata: install-lib-sta install-include install-bin install-man install-misc install-examples install-cfg

install: install-doc install-nodata install-data

install-include:
	-mkdir -p $(INCLUDEDIR)/pari
	-for i in paricfg.h mpinl.h parimt.h; do \
	  $(INSTALL_DATA) $$i $(INCLUDEDIR)/pari; done
	-for i in paridecl paripriv pari paricast paricom parierr parigen pariinl parinf pariold paristio parisys paritune ; do \
	   $(INSTALL_DATA) ../src/headers/$$i.h  $(INCLUDEDIR)/pari; done
	-$(RM) $(INCLUDEDIR)/pari/genpari.h
	-$(LN) pari.h $(INCLUDEDIR)/pari/genpari.h

install-bin: install-bin-sta

install-bin-dyn: gp-dyn install-lib-dyn
	-mkdir -p $(BINDIR)
	-$(RM) $(BINDIR)/gp-2.13 $(BINDIR)/gp
	$(LD) -o $(BINDIR)/gp-2.13 -L$(LIBDIR) $(LDFLAGS) $(OBJSGP) $(RUNPTH_FINAL) $(LDDYN) $(RLLIBS) $(PLOTLIBS) $(LIBS)
	-$(STRIP) $(STRIPFLAGS) $(BINDIR)/gp-2.13
	-cd $(BINDIR); : gp-2.13
	-$(LN) gp-2.13 $(BINDIR)/gp

install-bin-sta: gp-sta
	-mkdir -p $(BINDIR)
	-$(RM) $(BINDIR)/gp-2.13 $(BINDIR)/gp
	$(INSTALL_PROGRAM) gp-sta $(BINDIR)/gp-2.13
	-$(LN) gp-2.13 $(BINDIR)/gp
# Can't strip it if we want install() to work on OSF.
#	-$(STRIP) $(STRIPFLAGS)  $(BINDIR)/gp-2.13
	-cd $(BINDIR); : gp-2.13

install-man::
	-mkdir -p $(MANDIR)
	-$(RM) $(MANDIR)/pari.1 $(MANDIR)/gp.1 $(MANDIR)/gp-2.13.1
	$(INSTALL_DATA) ../doc/gphelp.1 $(MANDIR)
	$(INSTALL_DATA) ../doc/gp.1 $(MANDIR)/gp-2.13.1
	-$(LN) gp.1 $(MANDIR)/pari.1
	-$(LN) gp-2.13.1 $(MANDIR)/gp.1

install-misc:
	-mkdir -p $(MISCDIR) $(BINDIR)
	$(INSTALL_PROGRAM) ../misc/tex2mail $(BINDIR)
	$(INSTALL_DATA) ../doc/tex2mail.1 $(MANDIR)
	$(INSTALL_DATA) ../misc/README    $(MISCDIR)
	$(INSTALL_DATA) ../misc/color.dft $(MISCDIR)
	$(INSTALL_DATA) ../misc/gpalias   $(MISCDIR)
	$(INSTALL_PROGRAM) ../misc/gpflog $(MISCDIR)
	$(INSTALL_DATA) ../misc/gprc.dft  $(MISCDIR)
	$(INSTALL_PROGRAM) ../misc/xgp    $(MISCDIR)

install-cfg::
	-mkdir -p $(SYSDATADIR)
	-$(INSTALL_DATA) pari.cfg $(SYSDATADIR)
	-if test -n "../src/funclist"; then	   mkdir -p $(DATADIR)/PARI;	   $(INSTALL_DATA) ../src/desc/PARI/822.pm $(DATADIR)/PARI;	   $(INSTALL_DATA) ../src/desc/$(DESC) $(DATADIR); fi

install-doc: install-docdvi install-doctex

install-doctex:
	-mkdir -p $(BINDIR) $(DOCDIR)
	-$(INSTALL_DATA) ../doc/translations $(DOCDIR)
	-$(INSTALL_PROGRAM) ../doc/gphelp    $(BINDIR)
	-cd ../doc; $(MAKE) usersch3.tex
	-$(INSTALL_DATA) ../doc/appa.tex     $(DOCDIR)
	-$(INSTALL_DATA) ../doc/appb.tex     $(DOCDIR)
	-$(INSTALL_DATA) ../doc/appd.tex     $(DOCDIR)
	-$(INSTALL_DATA) ../doc/parimacro.tex $(DOCDIR)
	-$(INSTALL_DATA) ../doc/pdfmacs.tex  $(DOCDIR)
	-$(INSTALL_DATA) ../doc/develop.tex  $(DOCDIR)
	-$(INSTALL_DATA) ../doc/refcard.tex  $(DOCDIR)
	-$(INSTALL_DATA) ../doc/tutorial.tex $(DOCDIR)
	-$(INSTALL_DATA) ../doc/tutorial-mf.tex $(DOCDIR)
	-$(INSTALL_DATA) ../doc/users.tex    $(DOCDIR)
	-$(INSTALL_DATA) ../doc/usersch1.tex $(DOCDIR)
	-$(INSTALL_DATA) ../doc/usersch2.tex $(DOCDIR)
	-$(INSTALL_DATA) ../doc/usersch3.tex $(DOCDIR)
	-$(INSTALL_DATA) ../doc/usersch4.tex $(DOCDIR)
	-$(INSTALL_DATA) ../doc/usersch5.tex $(DOCDIR)
	-$(INSTALL_DATA) ../doc/usersch6.tex $(DOCDIR)
	-$(INSTALL_DATA) ../doc/usersch7.tex $(DOCDIR)
	-$(INSTALL_DATA) ../doc/usersch8.tex $(DOCDIR)
	-$(INSTALL_DATA) ../doc/paricfg.tex  $(DOCDIR)

install-docdvi::
	-mkdir -p $(DOCDIR)
	-cd ../doc; $(MAKE) doc; for f in *.dvi; do 	  $(INSTALL_DATA) $$f $(DOCDIR);           done

install-docpdf::
	-mkdir -p $(DOCDIR)
	-cd ../doc; $(MAKE) docpdf; for f in *.pdf; do 	  $(INSTALL_DATA) $$f $(DOCDIR);           done

install-examples:
	-mkdir -p $(EXDIR)
	-$(INSTALL_DATA) ../examples/EXPLAIN     $(EXDIR)
	-$(INSTALL_DATA) ../examples/Inputrc     $(EXDIR)
	-$(INSTALL_DATA) Makefile.examples   $(EXDIR)/Makefile
	-$(INSTALL_DATA) ../examples/bench.gp    $(EXDIR)
	-$(INSTALL_DATA) ../examples/cl.gp       $(EXDIR)
	-$(INSTALL_DATA) ../examples/classno.gp  $(EXDIR)
	-$(INSTALL_DATA) ../examples/contfrac.gp $(EXDIR)
	-$(INSTALL_DATA) ../examples/lucas.gp    $(EXDIR)
	-$(INSTALL_DATA) ../examples/extgcd.c    $(EXDIR)
	-$(INSTALL_DATA) ../examples/rho.gp      $(EXDIR)
	-$(INSTALL_DATA) ../examples/squfof.gp   $(EXDIR)
	-$(INSTALL_DATA) ../examples/taylor.gp   $(EXDIR)

install-data:
	mkdir -p $(DATADIR); 	if test -d ../data; then cd ../data; 	  for d in `ls`; do             test -d "$$d" && ($(TAR) c $$d | (cd $(DATADIR) && $(TAR) xo )); 	  done; 	fi;

link-data:
	mkdir -p $(DATADIR); 	if test -d ../data; then cd ../data; 	  for d in `ls`; do 	    $(LN) $(TOPDIR)/data/$$d $(DATADIR)/; 	  done; 	fi;

install-lib-sta: $(LIBPARI_STA)
	-mkdir -p $(LIBDIR)
	-$(RM) $(LIBDIR)/$(LIBPARI_STA)
	$(INSTALL_DATA) $(LIBPARI_STA) $(LIBDIR)/$(LIBPARI_STA)

install-lib-dyn-base:
	-mkdir -p $(LIBDIR)
	-$(RM) $(LIBDIR)/$(LIBPARI_DYN) $(LIBDIR)/$(LIBPARI_SONAME) $(LIBDIR)/$(LIBPARI_SO)
	$(DLLD_IGNORE)$(DLLD) -o $(LIBDIR)/$(LIBPARI_DYN) $(DLLDFLAGS) $(OBJS) $(EXTRADLLDFLAGS) $(DYN_LIBS) $(EXPORT_FILE) $(LD_INSTALL_NAME)
	

install-lib-dyn-link: install-lib-dyn-base
	-if test "$(LIBPARI_DYN)" != "$(LIBDIR)/$(LIBPARI_SO)"; then           $(LN) $(LIBPARI_DYN) $(LIBDIR)/$(LIBPARI_SO); fi
	-if test "$(LIBPARI_SONAME)" != "$(LIBPARI_SO)"; then 	  $(LN) $(LIBPARI_DYN) $(LIBDIR)/$(LIBPARI_SONAME); fi
install-lib-dyn: $(LIBPARI_DYN) install-lib-dyn-link

nsis: gp
	-cd ../doc; $(MAKE) docpdf
	makensis pari.nsi

parimt.h: ../src/mt/single.h
	cat ../src/mt/single.h > parimt.h

mpinl.h: parilvl0.h parilvl1.h
	cat parilvl0.h parilvl1.h > mpinl.h

bench: gp-sta gp-dyn clean_pari_ps
	@$(DOTEST)   objets  analyz  number  polyser  linear  elliptic  sumiter  graph  program  trans  nfields_200
dobench:: clean_pari_ps
	@$(DOTEST)   objets  analyz  number  polyser  linear  elliptic  sumiter  graph  program  trans  nfields_200
test-all: gp-sta gp-dyn clean_pari_ps
	@$(DOTEST) $(TESTS)
dotest-all:: clean_pari_ps
	@$(DOTEST) $(TESTS)
dyntest-all: gp-dyn clean_pari_ps
	@env dotestSUF=dyn $(DOTEST) $(TESTS)
statest-all: gp-sta clean_pari_ps
	@env dotestSUF=sta $(DOTEST) $(TESTS)

dotest-env::
	@export AAA=XXX BBB=YYY; $(DOTEST) env
test-addprimes: gp-sta gp-dyn dotest-addprimes
test-agm: gp-sta gp-dyn dotest-agm
test-algebras: gp-sta gp-dyn dotest-algebras
test-alggroup: gp-sta gp-dyn dotest-alggroup
test-alghasse: gp-sta gp-dyn dotest-alghasse
test-alglattices: gp-sta gp-dyn dotest-alglattices
test-algsplit: gp-sta gp-dyn dotest-algsplit
test-analyz: gp-sta gp-dyn dotest-analyz
test-apply: gp-sta gp-dyn dotest-apply
test-arith: gp-sta gp-dyn dotest-arith
test-asymp: gp-sta gp-dyn dotest-asymp
test-aurifeuille: gp-sta gp-dyn dotest-aurifeuille
test-bbhnf: gp-sta gp-dyn dotest-bbhnf
test-bern: gp-sta gp-dyn dotest-bern
test-bessel: gp-sta gp-dyn dotest-bessel
test-bestappr: gp-sta gp-dyn dotest-bestappr
test-binomial: gp-sta gp-dyn dotest-binomial
test-bit: gp-sta gp-dyn dotest-bit
test-bnf: gp-sta gp-dyn dotest-bnf
test-bnfisintnorm: gp-sta gp-dyn dotest-bnfisintnorm
test-bnflog: gp-sta gp-dyn dotest-bnflog
test-bnfsunit: gp-sta gp-dyn dotest-bnfsunit
test-bnr: gp-sta gp-dyn dotest-bnr
test-bnrL1: gp-sta gp-dyn dotest-bnrL1
test-bnrclassfield: gp-sta gp-dyn dotest-bnrclassfield
test-bnrisgalois: gp-sta gp-dyn dotest-bnrisgalois
test-bnrmap: gp-sta gp-dyn dotest-bnrmap
test-character: gp-sta gp-dyn dotest-character
test-characteristic: gp-sta gp-dyn dotest-characteristic
test-charpoly: gp-sta gp-dyn dotest-charpoly
test-chinese: gp-sta gp-dyn dotest-chinese
test-cmp: gp-sta gp-dyn dotest-cmp
test-combinat: gp-sta gp-dyn dotest-combinat
test-compat: gp-sta gp-dyn dotest-compat
test-concat: gp-sta gp-dyn dotest-concat
test-content: gp-sta gp-dyn dotest-content
test-contfrac: gp-sta gp-dyn dotest-contfrac
test-cxtrigo: gp-sta gp-dyn dotest-cxtrigo
test-cyclo: gp-sta gp-dyn dotest-cyclo
test-debugger: gp-sta gp-dyn dotest-debugger
test-deriv: gp-sta gp-dyn dotest-deriv
test-det: gp-sta gp-dyn dotest-det
test-diffop: gp-sta gp-dyn dotest-diffop
test-digits: gp-sta gp-dyn dotest-digits
test-dirmul: gp-sta gp-dyn dotest-dirmul
test-dirpowers: gp-sta gp-dyn dotest-dirpowers
test-disc: gp-sta gp-dyn dotest-disc
test-div: gp-sta gp-dyn dotest-div
test-divisors: gp-sta gp-dyn dotest-divisors
test-ell: gp-sta gp-dyn dotest-ell
test-ellanal: gp-sta gp-dyn dotest-ellanal
test-ellff: gp-sta gp-dyn dotest-ellff
test-ellglobalred: gp-sta gp-dyn dotest-ellglobalred
test-elliptic: gp-sta gp-dyn dotest-elliptic
test-ellisogeny: gp-sta gp-dyn dotest-ellisogeny
test-ellisomat: gp-sta gp-dyn dotest-ellisomat
test-ellissupersingular: gp-sta gp-dyn dotest-ellissupersingular
test-ellmodulareqn: gp-sta gp-dyn dotest-ellmodulareqn
test-ellnf: gp-sta gp-dyn dotest-ellnf
test-ellpadic: gp-sta gp-dyn dotest-ellpadic
test-ellpadiclambdamu: gp-sta gp-dyn dotest-ellpadiclambdamu
test-ellratpoints: gp-sta gp-dyn dotest-ellratpoints
test-ellsea: gp-sta gp-dyn dotest-ellsea
test-ellseaJ: gp-sta gp-dyn dotest-ellseaJ
test-elltors: gp-sta gp-dyn dotest-elltors
test-ellweilpairing: gp-sta gp-dyn dotest-ellweilpairing
test-equal: gp-sta gp-dyn dotest-equal
test-err: gp-sta gp-dyn dotest-err
test-eval: gp-sta gp-dyn dotest-eval
test-exact0: gp-sta gp-dyn dotest-exact0
test-export: gp-sta gp-dyn dotest-export
test-extract: gp-sta gp-dyn dotest-extract
test-factor: gp-sta gp-dyn dotest-factor
test-factorff: gp-sta gp-dyn dotest-factorff
test-factorint: gp-sta gp-dyn dotest-factorint
test-factormod: gp-sta gp-dyn dotest-factormod
test-ff: gp-sta gp-dyn dotest-ff
test-ffisom: gp-sta gp-dyn dotest-ffisom
test-fflog: gp-sta gp-dyn dotest-fflog
test-fft: gp-sta gp-dyn dotest-fft
test-forperm: gp-sta gp-dyn dotest-forperm
test-forsubset: gp-sta gp-dyn dotest-forsubset
test-forvec: gp-sta gp-dyn dotest-forvec
test-galois: gp-sta gp-dyn dotest-galois
test-galoischartable: gp-sta gp-dyn dotest-galoischartable
test-galoisinit: gp-sta gp-dyn dotest-galoisinit
test-galpol: gp-sta gp-dyn dotest-galpol
test-gamma: gp-sta gp-dyn dotest-gamma
test-gammamellininv: gp-sta gp-dyn dotest-gammamellininv
test-gcdext: gp-sta gp-dyn dotest-gcdext
test-genus2red: gp-sta gp-dyn dotest-genus2red
test-graph: gp-sta gp-dyn dotest-graph
test-help: gp-sta gp-dyn dotest-help
test-history: gp-sta gp-dyn dotest-history
test-hyperell: gp-sta gp-dyn dotest-hyperell
test-hypergeom: gp-sta gp-dyn dotest-hypergeom
test-ideal: gp-sta gp-dyn dotest-ideal
test-idealappr: gp-sta gp-dyn dotest-idealappr
test-idealramgroups: gp-sta gp-dyn dotest-idealramgroups
test-incgam: gp-sta gp-dyn dotest-incgam
test-interpol: gp-sta gp-dyn dotest-interpol
test-intnum: gp-sta gp-dyn dotest-intnum
test-ispower: gp-sta gp-dyn dotest-ispower
test-isprime: gp-sta gp-dyn dotest-isprime
test-iterator: gp-sta gp-dyn dotest-iterator
test-krasner: gp-sta gp-dyn dotest-krasner
test-lambert: gp-sta gp-dyn dotest-lambert
test-language: gp-sta gp-dyn dotest-language
test-lex: gp-sta gp-dyn dotest-lex
test-lfun: gp-sta gp-dyn dotest-lfun
test-lfunartin: gp-sta gp-dyn dotest-lfunartin
test-lfunquad: gp-sta gp-dyn dotest-lfunquad
test-lfuntype: gp-sta gp-dyn dotest-lfuntype
test-lift: gp-sta gp-dyn dotest-lift
test-lindep: gp-sta gp-dyn dotest-lindep
test-linear: gp-sta gp-dyn dotest-linear
test-list: gp-sta gp-dyn dotest-list
test-lll: gp-sta gp-dyn dotest-lll
test-log: gp-sta gp-dyn dotest-log
test-logint: gp-sta gp-dyn dotest-logint
test-map: gp-sta gp-dyn dotest-map
test-mat: gp-sta gp-dyn dotest-mat
test-mathnf: gp-sta gp-dyn dotest-mathnf
test-matpermanent: gp-sta gp-dyn dotest-matpermanent
test-matsnf: gp-sta gp-dyn dotest-matsnf
test-member: gp-sta gp-dyn dotest-member
test-memory: gp-sta gp-dyn dotest-memory
test-mf: gp-sta gp-dyn dotest-mf
test-mfgaloisrep: gp-sta gp-dyn dotest-mfgaloisrep
test-minim: gp-sta gp-dyn dotest-minim
test-minmax: gp-sta gp-dyn dotest-minmax
test-modfun: gp-sta gp-dyn dotest-modfun
test-modpr: gp-sta gp-dyn dotest-modpr
test-modsym: gp-sta gp-dyn dotest-modsym
test-modular: gp-sta gp-dyn dotest-modular
test-mscosets: gp-sta gp-dyn dotest-mscosets
test-mspadic: gp-sta gp-dyn dotest-mspadic
test-mspolygon: gp-sta gp-dyn dotest-mspolygon
test-multiif: gp-sta gp-dyn dotest-multiif
test-multivar-mul: gp-sta gp-dyn dotest-multivar-mul
test-nf: gp-sta gp-dyn dotest-nf
test-nfcompositum: gp-sta gp-dyn dotest-nfcompositum
test-nfdiscfactors: gp-sta gp-dyn dotest-nfdiscfactors
test-nfeltembed: gp-sta gp-dyn dotest-nfeltembed
test-nfeltsign: gp-sta gp-dyn dotest-nfeltsign
test-nffactor: gp-sta gp-dyn dotest-nffactor
test-nfhilbert: gp-sta gp-dyn dotest-nfhilbert
test-nfields: gp-sta gp-dyn dotest-nfields
test-nfislocalpower: gp-sta gp-dyn dotest-nfislocalpower
test-nfpolsturm: gp-sta gp-dyn dotest-nfpolsturm
test-nfrootsof1: gp-sta gp-dyn dotest-nfrootsof1
test-nfsplitting: gp-sta gp-dyn dotest-nfsplitting
test-norm: gp-sta gp-dyn dotest-norm
test-number: gp-sta gp-dyn dotest-number
test-objets: gp-sta gp-dyn dotest-objets
test-op: gp-sta gp-dyn dotest-op
test-orthopol: gp-sta gp-dyn dotest-orthopol
test-padic: gp-sta gp-dyn dotest-padic
test-partition: gp-sta gp-dyn dotest-partition
test-perm: gp-sta gp-dyn dotest-perm
test-plotexport: gp-sta gp-dyn dotest-plotexport
test-pol: gp-sta gp-dyn dotest-pol
test-polclass: gp-sta gp-dyn dotest-polclass
test-polmod: gp-sta gp-dyn dotest-polmod
test-polmodular: gp-sta gp-dyn dotest-polmodular
test-polred: gp-sta gp-dyn dotest-polred
test-polygonal: gp-sta gp-dyn dotest-polygonal
test-polylog: gp-sta gp-dyn dotest-polylog
test-polyser: gp-sta gp-dyn dotest-polyser
test-pow: gp-sta gp-dyn dotest-pow
test-prec: gp-sta gp-dyn dotest-prec
test-prime: gp-sta gp-dyn dotest-prime
test-print: gp-sta gp-dyn dotest-print
test-printf: gp-sta gp-dyn dotest-printf
test-program: gp-sta gp-dyn dotest-program
test-programming: gp-sta gp-dyn dotest-programming
test-qf: gp-sta gp-dyn dotest-qf
test-qfb: gp-sta gp-dyn dotest-qfb
test-qfbclassno: gp-sta gp-dyn dotest-qfbclassno
test-qfbsolve: gp-sta gp-dyn dotest-qfbsolve
test-qfisom: gp-sta gp-dyn dotest-qfisom
test-qfsolve: gp-sta gp-dyn dotest-qfsolve
test-quad: gp-sta gp-dyn dotest-quad
test-quadclassunit: gp-sta gp-dyn dotest-quadclassunit
test-quaddisc: gp-sta gp-dyn dotest-quaddisc
test-quadray: gp-sta gp-dyn dotest-quadray
test-ramanujantau: gp-sta gp-dyn dotest-ramanujantau
test-random: gp-sta gp-dyn dotest-random
test-ranges: gp-sta gp-dyn dotest-ranges
test-ratpoints: gp-sta gp-dyn dotest-ratpoints
test-real: gp-sta gp-dyn dotest-real
test-resultant: gp-sta gp-dyn dotest-resultant
test-rfrac: gp-sta gp-dyn dotest-rfrac
test-rnf: gp-sta gp-dyn dotest-rnf
test-rnfkummer: gp-sta gp-dyn dotest-rnfkummer
test-rootsof1: gp-sta gp-dyn dotest-rootsof1
test-rootsreal: gp-sta gp-dyn dotest-rootsreal
test-round: gp-sta gp-dyn dotest-round
test-round4: gp-sta gp-dyn dotest-round4
test-select: gp-sta gp-dyn dotest-select
test-self: gp-sta gp-dyn dotest-self
test-ser: gp-sta gp-dyn dotest-ser
test-set: gp-sta gp-dyn dotest-set
test-size: gp-sta gp-dyn dotest-size
test-solve: gp-sta gp-dyn dotest-solve
test-sort: gp-sta gp-dyn dotest-sort
test-sqrtn: gp-sta gp-dyn dotest-sqrtn
test-stark: gp-sta gp-dyn dotest-stark
test-str: gp-sta gp-dyn dotest-str
test-subcyclo: gp-sta gp-dyn dotest-subcyclo
test-subfields: gp-sta gp-dyn dotest-subfields
test-subgroup: gp-sta gp-dyn dotest-subgroup
test-subst: gp-sta gp-dyn dotest-subst
test-sumdedekind: gp-sta gp-dyn dotest-sumdedekind
test-sumdiv: gp-sta gp-dyn dotest-sumdiv
test-sumformal: gp-sta gp-dyn dotest-sumformal
test-sumiter: gp-sta gp-dyn dotest-sumiter
test-sumnum: gp-sta gp-dyn dotest-sumnum
test-sumnumrat: gp-sta gp-dyn dotest-sumnumrat
test-thue: gp-sta gp-dyn dotest-thue
test-trans: gp-sta gp-dyn dotest-trans
test-trans2: gp-sta gp-dyn dotest-trans2
test-valuation: gp-sta gp-dyn dotest-valuation
test-vararg: gp-sta gp-dyn dotest-vararg
test-variable: gp-sta gp-dyn dotest-variable
test-version: gp-sta gp-dyn dotest-version
test-whatnow: gp-sta gp-dyn dotest-whatnow
test-zeta: gp-sta gp-dyn dotest-zeta
test-zetahurwitz: gp-sta gp-dyn dotest-zetahurwitz
test-zetamult: gp-sta gp-dyn dotest-zetamult
test-zn: gp-sta gp-dyn dotest-zn
test-zncoppersmith: gp-sta gp-dyn dotest-zncoppersmith
test-ploth: gp-sta gp-dyn dotest-ploth
test-io: gp-sta gp-dyn dotest-io
test-parallel: gp-sta gp-dyn dotest-parallel
test-install: gp-sta gp-dyn dotest-install
test-time: gp-sta gp-dyn dotest-time
test-env: gp-sta gp-dyn dotest-env
dotest-addprimes::
	@$(DOTEST) addprimes
dotest-agm::
	@$(DOTEST) agm
dotest-algebras::
	@$(DOTEST) algebras
dotest-alggroup::
	@$(DOTEST) alggroup
dotest-alghasse::
	@$(DOTEST) alghasse
dotest-alglattices::
	@$(DOTEST) alglattices
dotest-algsplit::
	@$(DOTEST) algsplit
dotest-analyz::
	@$(DOTEST) analyz
dotest-apply::
	@$(DOTEST) apply
dotest-arith::
	@$(DOTEST) arith
dotest-asymp::
	@$(DOTEST) asymp
dotest-aurifeuille::
	@$(DOTEST) aurifeuille
dotest-bbhnf::
	@$(DOTEST) bbhnf
dotest-bern::
	@$(DOTEST) bern
dotest-bessel::
	@$(DOTEST) bessel
dotest-bestappr::
	@$(DOTEST) bestappr
dotest-binomial::
	@$(DOTEST) binomial
dotest-bit::
	@$(DOTEST) bit
dotest-bnf::
	@$(DOTEST) bnf
dotest-bnfisintnorm::
	@$(DOTEST) bnfisintnorm
dotest-bnflog::
	@$(DOTEST) bnflog
dotest-bnfsunit::
	@$(DOTEST) bnfsunit
dotest-bnr::
	@$(DOTEST) bnr
dotest-bnrL1::
	@$(DOTEST) bnrL1
dotest-bnrclassfield::
	@$(DOTEST) bnrclassfield
dotest-bnrisgalois::
	@$(DOTEST) bnrisgalois
dotest-bnrmap::
	@$(DOTEST) bnrmap
dotest-character::
	@$(DOTEST) character
dotest-characteristic::
	@$(DOTEST) characteristic
dotest-charpoly::
	@$(DOTEST) charpoly
dotest-chinese::
	@$(DOTEST) chinese
dotest-cmp::
	@$(DOTEST) cmp
dotest-combinat::
	@$(DOTEST) combinat
dotest-compat::
	@$(DOTEST) compat
dotest-concat::
	@$(DOTEST) concat
dotest-content::
	@$(DOTEST) content
dotest-contfrac::
	@$(DOTEST) contfrac
dotest-cxtrigo::
	@$(DOTEST) cxtrigo
dotest-cyclo::
	@$(DOTEST) cyclo
dotest-debugger::
	@$(DOTEST) debugger
dotest-deriv::
	@$(DOTEST) deriv
dotest-det::
	@$(DOTEST) det
dotest-diffop::
	@$(DOTEST) diffop
dotest-digits::
	@$(DOTEST) digits
dotest-dirmul::
	@$(DOTEST) dirmul
dotest-dirpowers::
	@$(DOTEST) dirpowers
dotest-disc::
	@$(DOTEST) disc
dotest-div::
	@$(DOTEST) div
dotest-divisors::
	@$(DOTEST) divisors
dotest-ell::
	@$(DOTEST) ell
dotest-ellanal::
	@$(DOTEST) ellanal
dotest-ellff::
	@$(DOTEST) ellff
dotest-ellglobalred::
	@$(DOTEST) ellglobalred
dotest-elliptic::
	@$(DOTEST) elliptic
dotest-ellisogeny::
	@$(DOTEST) ellisogeny
dotest-ellisomat::
	@$(DOTEST) ellisomat
dotest-ellissupersingular::
	@$(DOTEST) ellissupersingular
dotest-ellmodulareqn::
	@$(DOTEST) ellmodulareqn
dotest-ellnf::
	@$(DOTEST) ellnf
dotest-ellpadic::
	@$(DOTEST) ellpadic
dotest-ellpadiclambdamu::
	@$(DOTEST) ellpadiclambdamu
dotest-ellratpoints::
	@$(DOTEST) ellratpoints
dotest-ellsea::
	@$(DOTEST) ellsea
dotest-ellseaJ::
	@$(DOTEST) ellseaJ
dotest-elltors::
	@$(DOTEST) elltors
dotest-ellweilpairing::
	@$(DOTEST) ellweilpairing
dotest-equal::
	@$(DOTEST) equal
dotest-err::
	@$(DOTEST) err
dotest-eval::
	@$(DOTEST) eval
dotest-exact0::
	@$(DOTEST) exact0
dotest-export::
	@$(DOTEST) export
dotest-extract::
	@$(DOTEST) extract
dotest-factor::
	@$(DOTEST) factor
dotest-factorff::
	@$(DOTEST) factorff
dotest-factorint::
	@$(DOTEST) factorint
dotest-factormod::
	@$(DOTEST) factormod
dotest-ff::
	@$(DOTEST) ff
dotest-ffisom::
	@$(DOTEST) ffisom
dotest-fflog::
	@$(DOTEST) fflog
dotest-fft::
	@$(DOTEST) fft
dotest-forperm::
	@$(DOTEST) forperm
dotest-forsubset::
	@$(DOTEST) forsubset
dotest-forvec::
	@$(DOTEST) forvec
dotest-galois::
	@$(DOTEST) galois
dotest-galoischartable::
	@$(DOTEST) galoischartable
dotest-galoisinit::
	@$(DOTEST) galoisinit
dotest-galpol::
	@$(DOTEST) galpol
dotest-gamma::
	@$(DOTEST) gamma
dotest-gammamellininv::
	@$(DOTEST) gammamellininv
dotest-gcdext::
	@$(DOTEST) gcdext
dotest-genus2red::
	@$(DOTEST) genus2red
dotest-graph::
	@$(DOTEST) graph
dotest-help::
	@$(DOTEST) help
dotest-history::
	@$(DOTEST) history
dotest-hyperell::
	@$(DOTEST) hyperell
dotest-hypergeom::
	@$(DOTEST) hypergeom
dotest-ideal::
	@$(DOTEST) ideal
dotest-idealappr::
	@$(DOTEST) idealappr
dotest-idealramgroups::
	@$(DOTEST) idealramgroups
dotest-incgam::
	@$(DOTEST) incgam
dotest-interpol::
	@$(DOTEST) interpol
dotest-intnum::
	@$(DOTEST) intnum
dotest-ispower::
	@$(DOTEST) ispower
dotest-isprime::
	@$(DOTEST) isprime
dotest-iterator::
	@$(DOTEST) iterator
dotest-krasner::
	@$(DOTEST) krasner
dotest-lambert::
	@$(DOTEST) lambert
dotest-language::
	@$(DOTEST) language
dotest-lex::
	@$(DOTEST) lex
dotest-lfun::
	@$(DOTEST) lfun
dotest-lfunartin::
	@$(DOTEST) lfunartin
dotest-lfunquad::
	@$(DOTEST) lfunquad
dotest-lfuntype::
	@$(DOTEST) lfuntype
dotest-lift::
	@$(DOTEST) lift
dotest-lindep::
	@$(DOTEST) lindep
dotest-linear::
	@$(DOTEST) linear
dotest-list::
	@$(DOTEST) list
dotest-lll::
	@$(DOTEST) lll
dotest-log::
	@$(DOTEST) log
dotest-logint::
	@$(DOTEST) logint
dotest-map::
	@$(DOTEST) map
dotest-mat::
	@$(DOTEST) mat
dotest-mathnf::
	@$(DOTEST) mathnf
dotest-matpermanent::
	@$(DOTEST) matpermanent
dotest-matsnf::
	@$(DOTEST) matsnf
dotest-member::
	@$(DOTEST) member
dotest-memory::
	@$(DOTEST) memory
dotest-mf::
	@$(DOTEST) mf
dotest-mfgaloisrep::
	@$(DOTEST) mfgaloisrep
dotest-minim::
	@$(DOTEST) minim
dotest-minmax::
	@$(DOTEST) minmax
dotest-modfun::
	@$(DOTEST) modfun
dotest-modpr::
	@$(DOTEST) modpr
dotest-modsym::
	@$(DOTEST) modsym
dotest-modular::
	@$(DOTEST) modular
dotest-mscosets::
	@$(DOTEST) mscosets
dotest-mspadic::
	@$(DOTEST) mspadic
dotest-mspolygon::
	@$(DOTEST) mspolygon
dotest-multiif::
	@$(DOTEST) multiif
dotest-multivar-mul::
	@$(DOTEST) multivar-mul
dotest-nf::
	@$(DOTEST) nf
dotest-nfcompositum::
	@$(DOTEST) nfcompositum
dotest-nfdiscfactors::
	@$(DOTEST) nfdiscfactors
dotest-nfeltembed::
	@$(DOTEST) nfeltembed
dotest-nfeltsign::
	@$(DOTEST) nfeltsign
dotest-nffactor::
	@$(DOTEST) nffactor
dotest-nfhilbert::
	@$(DOTEST) nfhilbert
dotest-nfields::
	@$(DOTEST) nfields
dotest-nfislocalpower::
	@$(DOTEST) nfislocalpower
dotest-nfpolsturm::
	@$(DOTEST) nfpolsturm
dotest-nfrootsof1::
	@$(DOTEST) nfrootsof1
dotest-nfsplitting::
	@$(DOTEST) nfsplitting
dotest-norm::
	@$(DOTEST) norm
dotest-number::
	@$(DOTEST) number
dotest-objets::
	@$(DOTEST) objets
dotest-op::
	@$(DOTEST) op
dotest-orthopol::
	@$(DOTEST) orthopol
dotest-padic::
	@$(DOTEST) padic
dotest-partition::
	@$(DOTEST) partition
dotest-perm::
	@$(DOTEST) perm
dotest-plotexport::
	@$(DOTEST) plotexport
dotest-pol::
	@$(DOTEST) pol
dotest-polclass::
	@$(DOTEST) polclass
dotest-polmod::
	@$(DOTEST) polmod
dotest-polmodular::
	@$(DOTEST) polmodular
dotest-polred::
	@$(DOTEST) polred
dotest-polygonal::
	@$(DOTEST) polygonal
dotest-polylog::
	@$(DOTEST) polylog
dotest-polyser::
	@$(DOTEST) polyser
dotest-pow::
	@$(DOTEST) pow
dotest-prec::
	@$(DOTEST) prec
dotest-prime::
	@$(DOTEST) prime
dotest-print::
	@$(DOTEST) print
dotest-printf::
	@$(DOTEST) printf
dotest-program::
	@$(DOTEST) program
dotest-programming::
	@$(DOTEST) programming
dotest-qf::
	@$(DOTEST) qf
dotest-qfb::
	@$(DOTEST) qfb
dotest-qfbclassno::
	@$(DOTEST) qfbclassno
dotest-qfbsolve::
	@$(DOTEST) qfbsolve
dotest-qfisom::
	@$(DOTEST) qfisom
dotest-qfsolve::
	@$(DOTEST) qfsolve
dotest-quad::
	@$(DOTEST) quad
dotest-quadclassunit::
	@$(DOTEST) quadclassunit
dotest-quaddisc::
	@$(DOTEST) quaddisc
dotest-quadray::
	@$(DOTEST) quadray
dotest-ramanujantau::
	@$(DOTEST) ramanujantau
dotest-random::
	@$(DOTEST) random
dotest-ranges::
	@$(DOTEST) ranges
dotest-ratpoints::
	@$(DOTEST) ratpoints
dotest-real::
	@$(DOTEST) real
dotest-resultant::
	@$(DOTEST) resultant
dotest-rfrac::
	@$(DOTEST) rfrac
dotest-rnf::
	@$(DOTEST) rnf
dotest-rnfkummer::
	@$(DOTEST) rnfkummer
dotest-rootsof1::
	@$(DOTEST) rootsof1
dotest-rootsreal::
	@$(DOTEST) rootsreal
dotest-round::
	@$(DOTEST) round
dotest-round4::
	@$(DOTEST) round4
dotest-select::
	@$(DOTEST) select
dotest-self::
	@$(DOTEST) self
dotest-ser::
	@$(DOTEST) ser
dotest-set::
	@$(DOTEST) set
dotest-size::
	@$(DOTEST) size
dotest-solve::
	@$(DOTEST) solve
dotest-sort::
	@$(DOTEST) sort
dotest-sqrtn::
	@$(DOTEST) sqrtn
dotest-stark::
	@$(DOTEST) stark
dotest-str::
	@$(DOTEST) str
dotest-subcyclo::
	@$(DOTEST) subcyclo
dotest-subfields::
	@$(DOTEST) subfields
dotest-subgroup::
	@$(DOTEST) subgroup
dotest-subst::
	@$(DOTEST) subst
dotest-sumdedekind::
	@$(DOTEST) sumdedekind
dotest-sumdiv::
	@$(DOTEST) sumdiv
dotest-sumformal::
	@$(DOTEST) sumformal
dotest-sumiter::
	@$(DOTEST) sumiter
dotest-sumnum::
	@$(DOTEST) sumnum
dotest-sumnumrat::
	@$(DOTEST) sumnumrat
dotest-thue::
	@$(DOTEST) thue
dotest-trans::
	@$(DOTEST) trans
dotest-trans2::
	@$(DOTEST) trans2
dotest-valuation::
	@$(DOTEST) valuation
dotest-vararg::
	@$(DOTEST) vararg
dotest-variable::
	@$(DOTEST) variable
dotest-version::
	@$(DOTEST) version
dotest-whatnow::
	@$(DOTEST) whatnow
dotest-zeta::
	@$(DOTEST) zeta
dotest-zetahurwitz::
	@$(DOTEST) zetahurwitz
dotest-zetamult::
	@$(DOTEST) zetamult
dotest-zn::
	@$(DOTEST) zn
dotest-zncoppersmith::
	@$(DOTEST) zncoppersmith
dotest-ploth::
	@$(DOTEST) ploth
dotest-io::
	@$(DOTEST) io
dotest-parallel::
	@$(DOTEST) parallel
dotest-install::
	@$(DOTEST) install
dotest-time::
	@$(DOTEST) time

gp-dyn: ../src/funclist $(OBJSGP) $(LIBPARI_DYN) $(EXPORT_LIB)
	$(RM) $@
	$(LD) -o $@ -L$(TOPLDDYN) $(LDFLAGS) $(OBJSGP) $(RUNPTH) $(RLLIBS) $(LDDYN) $(PLOTLIBS) $(LIBS)
	: gp-dyn

$(LIBPARI_DYN): ../src/funclist $(OBJS) $(EXPORT_FILE)
	-$(RM) $(LIBPARI_DYN)
	$(DLLD_IGNORE)$(DLLD) -o $(TOPLDDYN)/$(LIBPARI_DYN) $(DLLDFLAGS) $(OBJS) $(EXTRADLLDFLAGS) $(DYN_LIBS) $(EXPORT_FILE)
	-if test "$(LIBPARI_DYN)" != "$(LIBPARI_SO)"; then 	  $(RM) $(LIBPARI_SO);	  $(LN) $(LIBPARI_DYN) $(LIBPARI_SO); fi
	-if test "$(LIBPARI_DYN)" != "$(LIBPARI_SONAME)"; then 	  $(RM) $(LIBPARI_SONAME);	  $(LN) $(LIBPARI_DYN) $(LIBPARI_SONAME); fi
../src/language/parse.c ../src/language/parse.h: ../src/language/parse.y
	$(BISON) -d ../src/language/parse.y -o ../src/language/parse.c

../src/funclist::
	@-../config/genfunclist ../src/funclist ../src/desc
../src/desc/$(DESC): ../src/funclist ../src/desc/merge_822 ../src/desc/PARI/822.pm
	f=funclist-$$$$-$(TMPSUF); (cd ../src/desc && $(PERL) merge_822 ../../src/funclist > $$f) && mv ../src/desc/$$f $@
../src/language/init.h: ../src/desc/$(DESC) $(DESC_HELP_GEN)
	f=init-$$$$-$(TMPSUF); (cd ../src/desc && $(PERL) gen_proto basic $(DESC) > $$f) && mv ../src/desc/$$f $@
../src/language/default.h: ../src/desc/$(DESC) $(DESC_HELP_GEN)
	f=default-$$$$-$(TMPSUF); (cd ../src/desc && $(PERL) gen_proto default $(DESC) > $$f) && mv ../src/desc/$$f $@
../src/gp/gp_init.h: ../src/desc/$(DESC) $(DESC_HELP_GEN)
	f=gp_init-$$$$-$(TMPSUF); (cd ../src/desc && $(PERL) gen_proto gp $(DESC) > $$f) && mv ../src/desc/$$f $@
L0MODS=../src/kernel/x86_64/asm0.h ../src/kernel/none/addll.h ../src/kernel/none/mulll.h ../src/kernel/none/bfffo.h ../src/kernel/none/divll.h ../src/kernel/none/divll_pre.h
parilvl0.h: $(L0MODS)
	../config/genkernel ../src ../src/kernel/x86_64/asm0.h > parilvl0.h
L1OBJS=../src/kernel/gmp/int.h ../src/kernel/none/level1.h
parilvl1.h: $(L1OBJS) ../src/headers/paritune.h
	if test -r ./tune.h; then d=.; else d=../src/kernel/gmp; fi;          cat $$d/tune.h $(L1OBJS) > parilvl1.h
MP_C=../src/kernel/gmp/mp.c ../src/kernel/none/cmp.c ../src/kernel/none/gcdll.c ../src/kernel/none/halfgcd.c  ../src/kernel/none/ratlift.c  ../src/kernel/none/invmod.c ../src/kernel/gmp/gcd.c ../src/kernel/gmp/gcdext.c ../src/kernel/none/mp_indep.c ../src/kernel/none/add.c
mpker.c: $(MP_C)
	cat $(MP_C) > mpker.c
mpker$(_O): .headers mpker.c
	$(CC) -c $(CFLAGS) $(KERNELCFLAGS) $(DLCFLAGS) $(CPPFLAGS) $(GMPINCLUDE) -o mpker$(_O) mpker.c

F2v$(_O): .headers  ../src/basemath/F2v.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o F2v$(_O) ../src/basemath/F2v.c
F2x$(_O): .headers  ../src/basemath/F2x.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o F2x$(_O) ../src/basemath/F2x.c
F2xqE$(_O): .headers  ../src/basemath/F2xqE.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o F2xqE$(_O) ../src/basemath/F2xqE.c
FF$(_O): .headers  ../src/basemath/FF.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o FF$(_O) ../src/basemath/FF.c
Fle$(_O): .headers  ../src/basemath/Fle.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o Fle$(_O) ../src/basemath/Fle.c
Flv$(_O): .headers  ../src/basemath/Flv.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o Flv$(_O) ../src/basemath/Flv.c
Flx$(_O): .headers  ../src/basemath/Flx.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o Flx$(_O) ../src/basemath/Flx.c
FlxX$(_O): .headers  ../src/basemath/FlxX.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o FlxX$(_O) ../src/basemath/FlxX.c
FlxqE$(_O): .headers  ../src/basemath/FlxqE.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o FlxqE$(_O) ../src/basemath/FlxqE.c
Flxq_log$(_O): .headers  ../src/basemath/Flxq_log.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o Flxq_log$(_O) ../src/basemath/Flxq_log.c
FpE$(_O): .headers  ../src/basemath/FpE.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o FpE$(_O) ../src/basemath/FpE.c
FpV$(_O): .headers  ../src/basemath/FpV.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o FpV$(_O) ../src/basemath/FpV.c
FpX$(_O): .headers  ../src/basemath/FpX.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o FpX$(_O) ../src/basemath/FpX.c
FpXQX_factor$(_O): .headers  ../src/basemath/FpXQX_factor.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o FpXQX_factor$(_O) ../src/basemath/FpXQX_factor.c
FpXX$(_O): .headers  ../src/basemath/FpXX.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o FpXX$(_O) ../src/basemath/FpXX.c
FpX_factor$(_O): .headers  ../src/basemath/FpX_factor.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o FpX_factor$(_O) ../src/basemath/FpX_factor.c
Hensel$(_O): .headers  ../src/basemath/Hensel.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o Hensel$(_O) ../src/basemath/Hensel.c
QX_factor$(_O): .headers  ../src/basemath/QX_factor.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o QX_factor$(_O) ../src/basemath/QX_factor.c
Qfb$(_O): .headers  ../src/basemath/Qfb.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o Qfb$(_O) ../src/basemath/Qfb.c
RgV$(_O): .headers  ../src/basemath/RgV.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o RgV$(_O) ../src/basemath/RgV.c
RgX$(_O): .headers  ../src/basemath/RgX.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o RgX$(_O) ../src/basemath/RgX.c
Ser$(_O): .headers  ../src/basemath/Ser.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o Ser$(_O) ../src/basemath/Ser.c
ZG$(_O): .headers  ../src/basemath/ZG.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o ZG$(_O) ../src/basemath/ZG.c
ZV$(_O): .headers  ../src/basemath/ZV.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o ZV$(_O) ../src/basemath/ZV.c
ZX$(_O): .headers  ../src/basemath/ZX.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o ZX$(_O) ../src/basemath/ZX.c
alglin1$(_O): .headers  ../src/basemath/alglin1.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o alglin1$(_O) ../src/basemath/alglin1.c
alglin2$(_O): .headers  ../src/basemath/alglin2.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o alglin2$(_O) ../src/basemath/alglin2.c
alglin3$(_O): .headers  ../src/basemath/alglin3.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o alglin3$(_O) ../src/basemath/alglin3.c
aprcl$(_O): .headers  ../src/basemath/aprcl.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o aprcl$(_O) ../src/basemath/aprcl.c
arith1$(_O): .headers  ../src/basemath/arith1.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o arith1$(_O) ../src/basemath/arith1.c
arith2$(_O): .headers  ../src/basemath/arith2.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o arith2$(_O) ../src/basemath/arith2.c
base1$(_O): .headers  ../src/basemath/base1.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o base1$(_O) ../src/basemath/base1.c
base2$(_O): .headers  ../src/basemath/base2.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o base2$(_O) ../src/basemath/base2.c
base3$(_O): .headers  ../src/basemath/base3.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o base3$(_O) ../src/basemath/base3.c
base4$(_O): .headers  ../src/basemath/base4.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o base4$(_O) ../src/basemath/base4.c
base5$(_O): .headers  ../src/basemath/base5.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o base5$(_O) ../src/basemath/base5.c
bb_group$(_O): .headers  ../src/basemath/bb_group.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o bb_group$(_O) ../src/basemath/bb_group.c
bb_hnf$(_O): .headers  ../src/basemath/bb_hnf.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o bb_hnf$(_O) ../src/basemath/bb_hnf.c
bern$(_O): .headers  ../src/basemath/bern.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o bern$(_O) ../src/basemath/bern.c
bibli1$(_O): .headers  ../src/basemath/bibli1.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o bibli1$(_O) ../src/basemath/bibli1.c
bibli2$(_O): .headers  ../src/basemath/bibli2.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o bibli2$(_O) ../src/basemath/bibli2.c
bit$(_O): .headers  ../src/basemath/bit.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o bit$(_O) ../src/basemath/bit.c
bnflog$(_O): .headers  ../src/basemath/bnflog.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o bnflog$(_O) ../src/basemath/bnflog.c
bnfunits$(_O): .headers  ../src/basemath/bnfunits.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o bnfunits$(_O) ../src/basemath/bnfunits.c
buch1$(_O): .headers  ../src/basemath/buch1.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o buch1$(_O) ../src/basemath/buch1.c
buch2$(_O): .headers  ../src/basemath/buch2.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o buch2$(_O) ../src/basemath/buch2.c
buch3$(_O): .headers  ../src/basemath/buch3.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o buch3$(_O) ../src/basemath/buch3.c
buch4$(_O): .headers  ../src/basemath/buch4.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o buch4$(_O) ../src/basemath/buch4.c
char$(_O): .headers  ../src/basemath/char.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o char$(_O) ../src/basemath/char.c
concat$(_O): .headers  ../src/basemath/concat.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o concat$(_O) ../src/basemath/concat.c
crvwtors$(_O): .headers  ../src/basemath/crvwtors.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o crvwtors$(_O) ../src/basemath/crvwtors.c
dirichlet$(_O): .headers  ../src/basemath/dirichlet.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o dirichlet$(_O) ../src/basemath/dirichlet.c
ecpp$(_O): .headers  ../src/basemath/ecpp.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o ecpp$(_O) ../src/basemath/ecpp.c
ellanal$(_O): .headers  ../src/basemath/ellanal.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o ellanal$(_O) ../src/basemath/ellanal.c
elliptic$(_O): .headers  ../src/basemath/elliptic.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o elliptic$(_O) ../src/basemath/elliptic.c
ellisog$(_O): .headers  ../src/basemath/ellisog.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o ellisog$(_O) ../src/basemath/ellisog.c
ellpadic$(_O): .headers  ../src/basemath/ellpadic.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o ellpadic$(_O) ../src/basemath/ellpadic.c
ellpadiclambdamu$(_O): .headers  ../src/basemath/ellpadiclambdamu.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o ellpadiclambdamu$(_O) ../src/basemath/ellpadiclambdamu.c
ellsea$(_O): .headers  ../src/basemath/ellsea.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o ellsea$(_O) ../src/basemath/ellsea.c
elltors$(_O): .headers  ../src/basemath/elltors.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o elltors$(_O) ../src/basemath/elltors.c
galconj$(_O): .headers  ../src/basemath/galconj.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o galconj$(_O) ../src/basemath/galconj.c
gen1$(_O): .headers  ../src/basemath/gen1.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o gen1$(_O) ../src/basemath/gen1.c
gen2$(_O): .headers  ../src/basemath/gen2.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o gen2$(_O) ../src/basemath/gen2.c
gen3$(_O): .headers  ../src/basemath/gen3.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o gen3$(_O) ../src/basemath/gen3.c
hnf_snf$(_O): .headers  ../src/basemath/hnf_snf.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o hnf_snf$(_O) ../src/basemath/hnf_snf.c
hyperell$(_O): .headers  ../src/basemath/hyperell.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o hyperell$(_O) ../src/basemath/hyperell.c
hypergeom$(_O): .headers  ../src/basemath/hypergeom.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o hypergeom$(_O) ../src/basemath/hypergeom.c
ifactor1$(_O): .headers  ../src/basemath/ifactor1.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o ifactor1$(_O) ../src/basemath/ifactor1.c
kummer$(_O): .headers  ../src/basemath/kummer.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o kummer$(_O) ../src/basemath/kummer.c
lfun$(_O): .headers  ../src/basemath/lfun.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o lfun$(_O) ../src/basemath/lfun.c
lfunquad$(_O): .headers  ../src/basemath/lfunquad.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o lfunquad$(_O) ../src/basemath/lfunquad.c
lfunutils$(_O): .headers  ../src/basemath/lfunutils.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o lfunutils$(_O) ../src/basemath/lfunutils.c
lll$(_O): .headers  ../src/basemath/lll.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o lll$(_O) ../src/basemath/lll.c
map$(_O): .headers  ../src/basemath/map.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o map$(_O) ../src/basemath/map.c
matperm$(_O): .headers  ../src/basemath/matperm.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o matperm$(_O) ../src/basemath/matperm.c
mellininv$(_O): .headers  ../src/basemath/mellininv.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o mellininv$(_O) ../src/basemath/mellininv.c
mftrace$(_O): .headers  ../src/basemath/mftrace.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o mftrace$(_O) ../src/basemath/mftrace.c
modsym$(_O): .headers  ../src/basemath/modsym.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o modsym$(_O) ../src/basemath/modsym.c
msfarey$(_O): .headers  ../src/basemath/msfarey.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o msfarey$(_O) ../src/basemath/msfarey.c
nffactor$(_O): .headers  ../src/basemath/nffactor.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o nffactor$(_O) ../src/basemath/nffactor.c
perm$(_O): .headers  ../src/basemath/perm.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o perm$(_O) ../src/basemath/perm.c
polarit1$(_O): .headers  ../src/basemath/polarit1.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o polarit1$(_O) ../src/basemath/polarit1.c
polarit2$(_O): .headers  ../src/basemath/polarit2.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o polarit2$(_O) ../src/basemath/polarit2.c
polarit3$(_O): .headers  ../src/basemath/polarit3.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o polarit3$(_O) ../src/basemath/polarit3.c
polclass$(_O): .headers  ../src/basemath/polclass.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o polclass$(_O) ../src/basemath/polclass.c
polmodular$(_O): .headers  ../src/basemath/polmodular.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o polmodular$(_O) ../src/basemath/polmodular.c
prime$(_O): .headers  ../src/basemath/prime.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o prime$(_O) ../src/basemath/prime.c
qfisom$(_O): .headers  ../src/basemath/qfisom.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o qfisom$(_O) ../src/basemath/qfisom.c
qfsolve$(_O): .headers  ../src/basemath/qfsolve.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o qfsolve$(_O) ../src/basemath/qfsolve.c
random$(_O): .headers  ../src/basemath/random.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o random$(_O) ../src/basemath/random.c
rootpol$(_O): .headers  ../src/basemath/rootpol.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o rootpol$(_O) ../src/basemath/rootpol.c
subcyclo$(_O): .headers  ../src/basemath/subcyclo.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o subcyclo$(_O) ../src/basemath/subcyclo.c
subfield$(_O): .headers  ../src/basemath/subfield.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o subfield$(_O) ../src/basemath/subfield.c
subgroup$(_O): .headers  ../src/basemath/subgroup.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o subgroup$(_O) ../src/basemath/subgroup.c
trans1$(_O): .headers  ../src/basemath/trans1.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o trans1$(_O) ../src/basemath/trans1.c
trans2$(_O): .headers  ../src/basemath/trans2.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o trans2$(_O) ../src/basemath/trans2.c
trans3$(_O): .headers  ../src/basemath/trans3.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o trans3$(_O) ../src/basemath/trans3.c
volcano$(_O): .headers  ../src/basemath/volcano.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o volcano$(_O) ../src/basemath/volcano.c
zetamult$(_O): .headers  ../src/basemath/zetamult.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o zetamult$(_O) ../src/basemath/zetamult.c
algebras$(_O): .headers  ../src/modules/algebras.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o algebras$(_O) ../src/modules/algebras.c
elldata$(_O): .headers  ../src/modules/elldata.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o elldata$(_O) ../src/modules/elldata.c
ellfromeqn$(_O): .headers  ../src/modules/ellfromeqn.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o ellfromeqn$(_O) ../src/modules/ellfromeqn.c
forperm$(_O): .headers  ../src/modules/forperm.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o forperm$(_O) ../src/modules/forperm.c
forsubset$(_O): .headers  ../src/modules/forsubset.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o forsubset$(_O) ../src/modules/forsubset.c
galois$(_O): .headers  ../src/modules/galois.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o galois$(_O) ../src/modules/galois.c
galpol$(_O): .headers  ../src/modules/galpol.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o galpol$(_O) ../src/modules/galpol.c
genus2red$(_O): .headers  ../src/modules/genus2red.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o genus2red$(_O) ../src/modules/genus2red.c
groupid$(_O): .headers  ../src/modules/groupid.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o groupid$(_O) ../src/modules/groupid.c
krasner$(_O): .headers  ../src/modules/krasner.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o krasner$(_O) ../src/modules/krasner.c
mpqs$(_O): .headers ../src/modules/mpqs.h ../src/modules/mpqs.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o mpqs$(_O) ../src/modules/mpqs.c
part$(_O): .headers  ../src/modules/part.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o part$(_O) ../src/modules/part.c
ratpoints$(_O): .headers  ../src/modules/ratpoints.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(KERNELCFLAGS) $(CFLAGS) -o ratpoints$(_O) ../src/modules/ratpoints.c
stark$(_O): .headers  ../src/modules/stark.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o stark$(_O) ../src/modules/stark.c
thue$(_O): .headers  ../src/modules/thue.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o thue$(_O) ../src/modules/thue.c
anal$(_O): .headers ../src/language/anal.h ../src/language/parse.h ../src/language/anal.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o anal$(_O) ../src/language/anal.c
compile$(_O): .headers ../src/language/anal.h ../src/language/tree.h ../src/language/opcode.h ../src/language/compile.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o compile$(_O) ../src/language/compile.c
default$(_O): .headers  ../src/language/default.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o default$(_O) ../src/language/default.c
es$(_O): .headers ../src/language/anal.h ../src/language/es.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o es$(_O) ../src/language/es.c
eval$(_O): .headers ../src/language/anal.h ../src/language/opcode.h ../src/language/eval.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o eval$(_O) ../src/language/eval.c
forprime$(_O): .headers  ../src/language/forprime.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o forprime$(_O) ../src/language/forprime.c
gplib$(_O): .headers  ../src/language/gplib.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o gplib$(_O) ../src/language/gplib.c
hash$(_O): .headers  ../src/language/hash.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o hash$(_O) ../src/language/hash.c
init$(_O): .headers ../src/language/anal.h ../src/language/init.h ../src/language/default.h ../src/language/init.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o init$(_O) ../src/language/init.c
intnum$(_O): .headers  ../src/language/intnum.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o intnum$(_O) ../src/language/intnum.c
members$(_O): .headers  ../src/language/members.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o members$(_O) ../src/language/members.c
paricfg$(_O): .headers ./paricfg.h ../src/language/paricfg.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o paricfg$(_O) ../src/language/paricfg.c
pariinl$(_O): .headers  ../src/language/pariinl.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o pariinl$(_O) ../src/language/pariinl.c
parse$(_O): .headers ../src/language/anal.h ../src/language/parse.h ../src/language/parsec.h ../src/language/tree.h ../src/language/parse.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o parse$(_O) ../src/language/parse.c
readline$(_O): .headers  ../src/language/readline.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o readline$(_O) ../src/language/readline.c
str$(_O): .headers  ../src/language/str.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o str$(_O) ../src/language/str.c
sumiter$(_O): .headers  ../src/language/sumiter.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o sumiter$(_O) ../src/language/sumiter.c
emacs$(_O): .headers  ../src/gp/emacs.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o emacs$(_O) ../src/gp/emacs.c
gp$(_O): .headers ../src/gp/gp.h ../src/gp/gp_init.h ../src/gp/gp.c
	$(CC) -c $(CPPFLAGS) -I../src/language -DDL_DFLT_NAME=$(DL_DFLT_NAME) $(CFLAGS) -o gp$(_O) ../src/gp/gp.c
gp_rl$(_O): .headers ../src/gp/gp.h ../src/gp/gp_rl.c
	$(CC) -c $(CPPFLAGS) -I../src/language $(RLINCLUDE) $(CFLAGS) -o gp_rl$(_O) ../src/gp/gp_rl.c
texmacs$(_O): .headers ../src/gp/gp.h ../src/gp/texmacs.c
	$(CC) -c $(CPPFLAGS) -I../src/language $(RLINCLUDE) $(CFLAGS) -o texmacs$(_O) ../src/gp/texmacs.c
whatnow$(_O): .headers ../src/gp/whatnow.h ../src/gp/whatnow.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -o whatnow$(_O) ../src/gp/whatnow.c
plotX$(_O): .headers ../src/graph/rect.h ../src/graph/plotX.c
	$(CC) -c $(CPPFLAGS) $(PLOTCFLAGS) $(CFLAGS) -o plotX$(_O) ../src/graph/plotX.c
plotport$(_O): .headers ../src/graph/rect.h ../src/graph/plotport.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o plotport$(_O) ../src/graph/plotport.c
plottty$(_O): .headers  ../src/graph/plottty.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o plottty$(_O) ../src/graph/plottty.c
darwin$(_O): .headers  ../src/systems/darwin/darwin.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o darwin$(_O) ../src/systems/darwin/darwin.c
mt$(_O): .headers  ../src/mt/mt.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o mt$(_O) ../src/mt/mt.c
single$(_O): .headers  ../src/mt/single.c
	$(CC) -c $(CPPFLAGS) $(DLCFLAGS) $(CFLAGS) -o single$(_O) ../src/mt/single.c
