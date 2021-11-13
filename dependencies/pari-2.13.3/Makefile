# This file was created by Configure. All changes made will be lost
# next time Configure is run.
#
SHELL = /bin/sh
VERS = pari-2.13.3 released
TAG=release-2-13-3

dft target::
	@echo "Main targets: we suggest 'make all', then 'make install' as root"
	@echo "    all                    Compilation + Documentation"
	@echo "    gp                     Compilation"
	@echo "    bench                  Compilation + Quick test"
	@echo "    dobench                Quick test only"
	@echo "    doc                    Documentation only"
	@echo "    install                Installation"
	@echo "    clean, cleantest       Clean up"
	@echo "For pari maintainers:"
	@echo "    dbg                    Compile gp binary suitable for debugging"
	@echo "    prf                    Compile gp binary suitable for profiling"
	@echo "    gcov                   Compile gp binary for test coverage reporting"
	@echo "    alpha, beta, release   Tarfile for official source distribution"
	@echo "    snapshot, distrib      Tarfile for source snapshot"
	@echo "    nsis                   Create a NSIS installer for win32"
	@echo "    ctags                  Generate VI/VIM tags file in ./src"
	@echo "    etags                  Generate Emacs  tags file in ./src"
	@echo "    tune                   Generate tuning utility"
	@echo "    test-all               Thorough regression tests (slow)"

all::
	@$(MAKE) gp
	@-cd doc && $(MAKE) doc

gp bench test-kernel test-all install cleanall cleanobj cleantest nsis link-data install-bin install-doc install-docpdf install-nodata install-data install-lib-sta install-bin-sta dobench dyntest-all statest-all tune test-addprimes test-agm test-algebras test-alggroup test-alghasse test-alglattices test-algsplit test-analyz test-apply test-arith test-asymp test-aurifeuille test-bbhnf test-bern test-bessel test-bestappr test-binomial test-bit test-bnf test-bnfisintnorm test-bnflog test-bnfsunit test-bnr test-bnrL1 test-bnrclassfield test-bnrisgalois test-bnrmap test-character test-characteristic test-charpoly test-chinese test-cmp test-combinat test-compat test-concat test-content test-contfrac test-cxtrigo test-cyclo test-debugger test-deriv test-det test-diffop test-digits test-dirmul test-dirpowers test-disc test-div test-divisors test-ell test-ellanal test-ellff test-ellglobalred test-elliptic test-ellisogeny test-ellisomat test-ellissupersingular test-ellmodulareqn test-ellnf test-ellpadic test-ellpadiclambdamu test-ellratpoints test-ellsea test-ellseaJ test-elltors test-ellweilpairing test-equal test-err test-eval test-exact0 test-export test-extract test-factor test-factorff test-factorint test-factormod test-ff test-ffisom test-fflog test-fft test-forperm test-forsubset test-forvec test-galois test-galoischartable test-galoisinit test-galpol test-gamma test-gammamellininv test-gcdext test-genus2red test-graph test-help test-history test-hyperell test-hypergeom test-ideal test-idealappr test-idealramgroups test-incgam test-interpol test-intnum test-ispower test-isprime test-iterator test-krasner test-lambert test-language test-lex test-lfun test-lfunartin test-lfunquad test-lfuntype test-lift test-lindep test-linear test-list test-lll test-log test-logint test-map test-mat test-mathnf test-matpermanent test-matsnf test-member test-memory test-mf test-mfgaloisrep test-minim test-minmax test-modfun test-modpr test-modsym test-modular test-mscosets test-mspadic test-mspolygon test-multiif test-multivar-mul test-nf test-nfcompositum test-nfdiscfactors test-nfeltembed test-nfeltsign test-nffactor test-nfhilbert test-nfields test-nfislocalpower test-nfpolsturm test-nfrootsof1 test-nfsplitting test-norm test-number test-objets test-op test-orthopol test-padic test-partition test-perm test-plotexport test-pol test-polclass test-polmod test-polmodular test-polred test-polygonal test-polylog test-polyser test-pow test-prec test-prime test-print test-printf test-program test-programming test-qf test-qfb test-qfbclassno test-qfbsolve test-qfisom test-qfsolve test-quad test-quadclassunit test-quaddisc test-quadray test-ramanujantau test-random test-ranges test-ratpoints test-real test-resultant test-rfrac test-rnf test-rnfkummer test-rootsof1 test-rootsreal test-round test-round4 test-select test-self test-ser test-set test-size test-solve test-sort test-sqrtn test-stark test-str test-subcyclo test-subfields test-subgroup test-subst test-sumdedekind test-sumdiv test-sumformal test-sumiter test-sumnum test-sumnumrat test-thue test-trans test-trans2 test-valuation test-vararg test-variable test-version test-whatnow test-zeta test-zetahurwitz test-zetamult test-zn test-zncoppersmith test-ploth test-io test-parallel test-install test-time test-env dotest-addprimes dotest-agm dotest-algebras dotest-alggroup dotest-alghasse dotest-alglattices dotest-algsplit dotest-analyz dotest-apply dotest-arith dotest-asymp dotest-aurifeuille dotest-bbhnf dotest-bern dotest-bessel dotest-bestappr dotest-binomial dotest-bit dotest-bnf dotest-bnfisintnorm dotest-bnflog dotest-bnfsunit dotest-bnr dotest-bnrL1 dotest-bnrclassfield dotest-bnrisgalois dotest-bnrmap dotest-character dotest-characteristic dotest-charpoly dotest-chinese dotest-cmp dotest-combinat dotest-compat dotest-concat dotest-content dotest-contfrac dotest-cxtrigo dotest-cyclo dotest-debugger dotest-deriv dotest-det dotest-diffop dotest-digits dotest-dirmul dotest-dirpowers dotest-disc dotest-div dotest-divisors dotest-ell dotest-ellanal dotest-ellff dotest-ellglobalred dotest-elliptic dotest-ellisogeny dotest-ellisomat dotest-ellissupersingular dotest-ellmodulareqn dotest-ellnf dotest-ellpadic dotest-ellpadiclambdamu dotest-ellratpoints dotest-ellsea dotest-ellseaJ dotest-elltors dotest-ellweilpairing dotest-equal dotest-err dotest-eval dotest-exact0 dotest-export dotest-extract dotest-factor dotest-factorff dotest-factorint dotest-factormod dotest-ff dotest-ffisom dotest-fflog dotest-fft dotest-forperm dotest-forsubset dotest-forvec dotest-galois dotest-galoischartable dotest-galoisinit dotest-galpol dotest-gamma dotest-gammamellininv dotest-gcdext dotest-genus2red dotest-graph dotest-help dotest-history dotest-hyperell dotest-hypergeom dotest-ideal dotest-idealappr dotest-idealramgroups dotest-incgam dotest-interpol dotest-intnum dotest-ispower dotest-isprime dotest-iterator dotest-krasner dotest-lambert dotest-language dotest-lex dotest-lfun dotest-lfunartin dotest-lfunquad dotest-lfuntype dotest-lift dotest-lindep dotest-linear dotest-list dotest-lll dotest-log dotest-logint dotest-map dotest-mat dotest-mathnf dotest-matpermanent dotest-matsnf dotest-member dotest-memory dotest-mf dotest-mfgaloisrep dotest-minim dotest-minmax dotest-modfun dotest-modpr dotest-modsym dotest-modular dotest-mscosets dotest-mspadic dotest-mspolygon dotest-multiif dotest-multivar-mul dotest-nf dotest-nfcompositum dotest-nfdiscfactors dotest-nfeltembed dotest-nfeltsign dotest-nffactor dotest-nfhilbert dotest-nfields dotest-nfislocalpower dotest-nfpolsturm dotest-nfrootsof1 dotest-nfsplitting dotest-norm dotest-number dotest-objets dotest-op dotest-orthopol dotest-padic dotest-partition dotest-perm dotest-plotexport dotest-pol dotest-polclass dotest-polmod dotest-polmodular dotest-polred dotest-polygonal dotest-polylog dotest-polyser dotest-pow dotest-prec dotest-prime dotest-print dotest-printf dotest-program dotest-programming dotest-qf dotest-qfb dotest-qfbclassno dotest-qfbsolve dotest-qfisom dotest-qfsolve dotest-quad dotest-quadclassunit dotest-quaddisc dotest-quadray dotest-ramanujantau dotest-random dotest-ranges dotest-ratpoints dotest-real dotest-resultant dotest-rfrac dotest-rnf dotest-rnfkummer dotest-rootsof1 dotest-rootsreal dotest-round dotest-round4 dotest-select dotest-self dotest-ser dotest-set dotest-size dotest-solve dotest-sort dotest-sqrtn dotest-stark dotest-str dotest-subcyclo dotest-subfields dotest-subgroup dotest-subst dotest-sumdedekind dotest-sumdiv dotest-sumformal dotest-sumiter dotest-sumnum dotest-sumnumrat dotest-thue dotest-trans dotest-trans2 dotest-valuation dotest-vararg dotest-variable dotest-version dotest-whatnow dotest-zeta dotest-zetahurwitz dotest-zetamult dotest-zn dotest-zncoppersmith dotest-ploth dotest-io dotest-parallel dotest-install dotest-time dotest-env::
	@dir=`config/objdir`; echo "Making $@ in $$dir";\
	 if test ! -d $$dir; then echo "Please run Configure first!"; exit 1; fi;\
	cd $$dir && $(MAKE) $@

dbg gp.dbg::
	@dir=`config/objdir`.dbg; echo "Making gp in $$dir";\
	 if test ! -d $$dir; then echo "Please run Configure -g first!"; exit 1; fi;\
	cd $$dir && $(MAKE) gp

prf gp.prf::
	@dir=`config/objdir`.prf; echo "Making gp in $$dir";\
	 if test ! -d $$dir; then echo "Please run Configure -pg first!"; exit 1; fi;\
	cd $$dir && $(MAKE) gp

gcov gp.gcov::
	@dir=`config/objdir`.gcov; echo "Making gp in $$dir";\
	 if test ! -d $$dir; then echo "Please run Configure -gcov first!"; exit 1; fi;\
	cd $$dir && $(MAKE) gp

doc docps docpdf gpman cleandoc::
	cd doc && $(MAKE) $@

clean:: cleandoc cleanall

clean.dbg::
	@dir=`config/objdir`.dbg; echo "Making clean in $$dir";\
	 if test ! -d $$dir; then echo "Nothing to be done"; exit 1; fi;\
	cd $$dir && $(MAKE) clean

clean.prf::
	@dir=`config/objdir`.prf; echo "Making clean in $$dir";\
	 if test ! -d $$dir; then echo "Nothing to be done"; exit 1; fi;\
	cd $$dir && $(MAKE) clean

clean.gcov::
	@dir=`config/objdir`.gcov; echo "Making clean in $$dir";\
	 if test ! -d $$dir; then echo "Nothing to be done"; exit 1; fi;\
	cd $$dir && $(MAKE) clean

bench.dbg::
	@dir=`config/objdir`.dbg; echo "Making bench in $$dir";\
	 if test ! -d $$dir; then echo "Please run Configure -g first!"; exit 1; fi;\
	cd $$dir && $(MAKE) bench

bench.prf::
	@dir=`config/objdir`.prf; echo "Making bench in $$dir";\
	 if test ! -d $$dir; then echo "Please run Configure -pg first!"; exit 1; fi;\
	cd $$dir && $(MAKE) bench

bench.gcov::
	@dir=`config/objdir`.gcov; echo "Making bench in $$dir";\
	 if test ! -d $$dir; then echo "Please run Configure -gcov first!"; exit 1; fi;\
	cd $$dir && $(MAKE) bench

distrib:
	config/settar $(VERS) released

setdoc:
	cd doc && $(MAKE) usersch3.tex
alpha: setdoc
	config/settar $(VERS) alpha
beta: setdoc
	config/settar $(VERS) beta
release: setdoc
	config/settar $(VERS) released
snapshot: setdoc
	config/settar $(VERS) snapshot

dosdistrib: gpman
	-zip -kr GPB_2133.ZIP GP.EXE README README.DOS ../EMX/README.DOC ../RSX/README.TXT
	-zip -k GPD_2133.ZIP doc/*.tex doc/gphelp doc/gp.man examples misc/gprc* misc/gpalias

etags:
	config/make_tags --emacs "/Users/julianalvarez/Documents/VSCODE/Smart_Symbolic_Graphing_Calculator/dependencies/pari-2.13.3/src"

ctags:
	config/make_tags --vi "/Users/julianalvarez/Documents/VSCODE/Smart_Symbolic_Graphing_Calculator/dependencies/pari-2.13.3/src"

checkspaces:
	config/checkspaces

.NOTPARALLEL:

