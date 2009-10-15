AC_PREREQ(2.52)
AC_INIT(src/prog/base.cpp)
AC_CONFIG_AUX_DIR(.config)
AM_INIT_AUTOMAKE(Otawa, 0.5)
AC_LANG(C++)

# ==== Plugin activation ====
AC_ARG_WITH(plugin,
	[  --with-plugin={yes,no}	Activate plugin usage.],,[with_plugin=no])
AS_IF(test "$with_plugin" = "yes",
	[
		AC_DEFINE(HAS_PLUGIN, [], [plugins available])
		HAS_PLUGIN=yes
	],
	[ HAS_PLUGIN=no ])
AM_CONDITIONAL(HAS_PLUGIN, test "$HAS_PLUGIN" = "yes")


# ==== relocation option ====
AC_ARG_WITH(relocation, [  --with-relocation={yes, no}],,[with_relocation=no])
AS_IF(test "$with_relocation" = "yes",
[
	AC_DEFINE(HAS_RELOCATION, [], [support relocation])
	PLUGIN_LIBS='-Wl,-rpath=\$$ORIGIN/../..'
	PROG_LIBS='-Wl,-rpath=\$$ORIGIN/../lib'
	LIB_LIBS='-Wl,-rpath=\$$ORIGIN'
])
AC_SUBST(PLUGIN_LIBS)
AC_SUBST(PROG_LIBS)
AC_SUBST(LIB_LIBS)
AM_CONDITIONAL(HAS_RELOCATION, test "$with_relocation" = "yes")


# === Distribution activation ===
AC_ARG_WITH(makedist, [  --with-makedist={yes, no}],,[with_makedist=yes])


# === Loader Selection ====
AC_ARG_WITH(loader, [  --with-loader={ppc,arm}	Use the given loader],,[with_loader=no])


# === ILP Selection ===
AC_ARG_WITH(ilp, [ --with-ilp={lp_solve,lp_solve5}	Use the given ILP solver],,[with_ilp=no])


# ==== Building mode ====
AC_ARG_WITH(mode,
	[  --with-mode={dev,debug,normal,final}	Activate the given build mode.],,with_mode=dev)
case $with_mode in
dev)
	CXXFLAGS="$CXXFLAGS -g3"
	;;
debug)
	CXXFLAGS="$CXXFLAGS -g3"
	;;
normal)
	;;
final)
	CPPFLAGS="$CPPFLAGS -DNDEBUG"
	CXXFLAGS="$CXXFLAGS -O3"
	;;
*)
	AC_MSG_ERROR([unknown mode $with_mode])
	;;
esac


# ==== Standard tests ====
AM_CONFIG_HEADER(config.h)
AC_PROG_LN_S
AC_PROG_CC
AC_PROG_CXX
AC_PROG_YACC
AM_PROG_LEX
AC_STDC_HEADERS
AC_PATH_PROG(BASH_PATH, bash)
AC_CHECK_FUNCS([strdup])
AC_CHECK_FUNCS([strtol])
AC_C_CONST
AC_C_INLINE
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T
AS_IF(test "$HAS_PLUGIN" = "yes", [ AC_ENABLE_SHARED ], [ AC_DISABLE_SHARED ])
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL


# ==== sed lookup ====
AC_CHECK_PROG(HAS_SED, sed, yes)
AM_CONDITIONAL(HAS_SED, test "$HAS_SED" = "yes")


# ==== Endianness ====
AC_C_BIGENDIAN(
	[IS_BIG="1"],
	[IS_BIG="0"],
	[AC_MSG_ERROR([unsupported endianness])])
AS_AC_EXPAND(IS_BIG, "$IS_BIG")
AC_DEFINE_UNQUOTED(IS_BIG, [$IS_BIG], [Big endian ?])


# ===== GEL check =====
OTAWA_USE_LIB(GEL,
	OTAWA_REQUIRE_LIB(GEL, [GEL library required !],
		OTAWA_CHECK_LTLIB(GEL, gel/gel.h, libgel.la, $PWD/../gel/src, $PWD/../gel/include,
		OTAWA_CHECK_LTLIB(GEL, gel/gel.h, libgel.la, ${prefix}, ,
		OTAWA_CHECK_LTLIB(GEL, gel/gel.h, libgel.la)))))
OTAWA_USE_LIB(GEL_PPC,
		OTAWA_CHECK_LTLIB(GEL_PPC, , libgel_ppc.la, $PWD/../gel/src, $PWD/../gel/include,
		OTAWA_CHECK_LTLIB(GEL_PPC, , libgel_ppc.la, ${prefix}, ,
		OTAWA_CHECK_LTLIB(GEL_PPC, , libgel_ppc.la))))
OTAWA_USE_LIB(GEL_DWARF,
		OTAWA_CHECK_LTLIB(GEL_DWARF, gel/dwarf_line.h, libgel_dwarf.la, $PWD/../gel/src, $PWD/../gel/include,
		OTAWA_CHECK_LTLIB(GEL_DWARF, gel/dwarf_line.h, libgel_dwarf.la, ${prefix}, ,
		OTAWA_CHECK_LTLIB(GEL_DWARF, gel/dwarf_line.h, libgel_dwarf.la))))


# ===== ELM check =====
OTAWA_USE_LIB(ELM,
	OTAWA_REQUIRE_LIB(ELM, [ELM library required !],
		OTAWA_CHECK_LTLIB(ELM, elm/string.h, libelm.la, $PWD/../elm/src, $PWD/../elm/include,
		OTAWA_CHECK_LTLIB(ELM, elm/string.h, libelm.la, ${prefix}, ,
		OTAWA_CHECK_LTLIB(ELM, elm/string.h, libelm.la)))))


# ===== GLISS_PPC check ======
OTAWA_USE_LIB(GLISS_PPC,
	OTAWA_CHECK_LIB(GLISS_PPC, emul.h, emul, $PWD/../ppc/lib, $PWD/../ppc/include))


# ===== GLISS_ARM check ======
OTAWA_USE_LIB(GLISS_ARM,
	OTAWA_CHECK_LIB(GLISS_ARM, emul.h, emul, $PWD/../arm/lib, $PWD/../arm/include))


# ===== GLISS_S12X check ======
OTAWA_USE_LIB(GLISS_S12X,
	OTAWA_CHECK_LIB(GLISS_S12X, emul.h, emul, $PWD/../hcs12/lib, $PWD/../hcs12/include))


# ===== LP_SOLVE check =====
OTAWA_USE_LIB(LP_SOLVE,
	OTAWA_CHECK_LIB(LP_SOLVE, lpkit.h, lpk, $PWD/../lp_solve, $PWD/../lp_solve,
	OTAWA_CHECK_LIB(LP_SOLVE, lpkit.h, lpk)))


# ===== LP_SOLVE5 check =====
OTAWA_USE_LIB(LP_SOLVE5,
	OTAWA_CHECK_LIB(LP_SOLVE5, lp_lib.h, lpsolve55, $PWD/../lp_solve5/lpsolve55, $PWD/../lp_solve5,
	OTAWA_CHECK_LIB(LP_SOLVE5, lpsolve/lp_lib.h, lpsolve55  -lcolamd)))


# ===== GLPK check =====
#OTAWA_USE_LIB(GLPK,
#	OTAWA_CHECK_LIB(GLPK, glpk.h, glpk, $PWD/../glpk/, $PWD/../glpk/,
#	OTAWA_CHECK_LIB(GLPK, glpk.h, glpk))) 


# ===== ILP check =====
HAS_ILP="no"
ILP_DEPS=""
ILP_LIBS=""
ILP_DEFAULT=""
OTAWA_ILP(LP_SOLVE5, lp_solve5)
OTAWA_ILP(LP_SOLVE, lp_solve)
#OTAWA_ILP(GLPK, glpk)
AM_CONDITIONAL(HAS_ILP, test "$HAS_ILP" = "yes")
AC_SUBST(ILP_LIBS)
AC_SUBST(ILP_DEPS)
AC_SUBST(ILP_DEFAULT)


# ===== SYSTEMC check =====
OTAWA_USE_LIB(SYSTEMC,
	OTAWA_CHECK_LIB(SYSTEMC, systemc.h, systemc, $PWD/../systemc/objdir/src, $PWD/../systemc/src,
	OTAWA_CHECK_LIB(SYSTEMC, systemc.h, systemc, /usr/lib-linux, /usr/include,
	OTAWA_CHECK_LIB(SYSTEMC, systemc.h, systemc, $PWD/../systemc/lib-linux, $PWD/../systemc/include)))) 


# ===== dot check =====
AC_PATH_PROG(HAS_DOT, dot, no)
AM_CONDITIONAL(HAS_DOT, test "$HAS_DOT" != no)


# ===== thot check =====
AC_PATH_PROG(THOT_PATH, thot.py, no, "$PATH:$PWD/../thot")
AM_CONDITIONAL(HAS_THOT, test "$THOT_PATH" != no)
AC_SUBST(THOT_PATH)


# ===== ocaml check =====
AC_PATH_PROG(HAS_OCAMLC, ocamlc, no)
AM_CONDITIONAL(HAS_OCAMLC, test "$HAS_OCAMLC" != no)
AC_PATH_PROG(HAS_OCAMLOPT, ocamlopt, no)
AM_CONDITIONAL(HAS_OCAMLOPT, test "$HAS_OCAMLOPT" != no)


# ===== Useful Definitions =====
OTAWA_CXXFLAGS="-I\$(top_srcdir)/include $ELM_CXXFLAGS"
OTAWA_DEPS="\$(top_builddir)/src/prog/libotawa.la $ELM_LIBS"
AS_IF(test "$HAS_PLUGIN" != yes, [ OTAWA_DEPS="$OTAWA_DEPS" ])
OTAWA_LIBS="$OTAWA_DEPS -dlopen force $GEL_LIBS"
ODISPLAY_LIBS="\$(top_builddir)/src/odisplay/libodisplay.la"
ODISPLAY_DEPS="$ODISPLAY_LIBS"
OGENSIM_LIBS="\$(top_builddir)/src/ogensim/libogensim.la"
OGENSIM_DEPS="$OGENSIM_LIBS"
ILP_PATHS=$libdir/otawa/ilp
LOADER_PATHS=$libdir/otawa/loader
PROC_PATHS=$libdir/otawa/proc
SIMULATOR_PATHS=$libdir/otawa/sim
AC_SUBST(OTAWA_CXXFLAGS)
AC_SUBST(OTAWA_LIBS)
AC_SUBST(OTAWA_DEPS)
AC_SUBST(ODISPLAY_LIBS)
AC_SUBST(ODISPLAY_DEPS)
AC_SUBST(OGENSIM_LIBS)
AC_SUBST(OGENSIM_DEPS)

# Configurations
AS_AC_EXPAND(ILP_PATHS, "$ILP_PATHS")
AC_DEFINE_UNQUOTED(ILP_PATHS, "$ILP_PATHS", [Paths for ILP plugins])
AS_AC_EXPAND(LOADER_PATHS,"$LOADER_PATHS")
AC_DEFINE_UNQUOTED(LOADER_PATHS, "$LOADER_PATHS", [Paths for loader plugins])
AS_AC_EXPAND(PROC_PATHS,"$PROC_PATHS")
AC_DEFINE_UNQUOTED(PROC_PATHS, "$PROC_PATHS", [Paths for processor plugins])
AS_AC_EXPAND(SIMULATOR_PATHS, "$SIMULATOR_PATHS")
AC_DEFINE_UNQUOTED(SIMULATOR_PATHS, "$SIMULATOR_PATHS", [Paths for simulator plugins])


# ====== AUTODOC configuration ======
AUTODOC='$(pkgdatadir)/autodoc'
AC_SUBST(AUTODOC)


# ===== Select Loader =====
AS_IF(test "$HAS_PLUGIN" = "yes", [
	HAS_LOADER="no"
	LOADER_LIBS=""
	LOADER_DEPS=""
],
[
	AC_MSG_CHECKING([for a loader])
	AS_IF(test "$with_loader" = "no", [
		AS_IF(test "$HAS_GLISS_PPC" = "yes", [
			HAS_LOADER="yes"
			LOADER_DEPS="\$(top_builddir)/src/ppc/ppc.la"
			LOADER_LIBS="-uppc_plugin $LOADER_DEPS"
		],
		AS_IF(test "$HAS_GLISS_ARM" = "yes", [
			HAS_LOADER="yes"
			LOADER_DEPS="\$(top_builddir)/src/arm/arm.la"
			LOADER_LIBS="-uarm_plugin $LOADER_DEPS"
		],
		AS_IF(test "$HAS_GLISS_S12X" = "yes", [
			HAS_LOADER="yes"
			LOADER_DEPS="\$(top_builddir)/src/s12x/s12x.la"
			LOADER_LIBS="-us12x_plugin $LOADER_DEPS"
		],
		[
			AC_MSG_FAILURE([no loader selected !])
		])))
	],
	[
		AS_IF(test "$with_loader" = "ppc",
		[
			HAS_LOADER="yes"
			LOADER_DEPS="\$(top_builddir)/src/ppc/ppc.la"
			LOADER_LIBS="-uppc_plugin $LOADER_DEPS"
		],
		AS_IF(test "$with_loader" = "arm",
		[
			HAS_LOADER="yes"
			LOADER_DEPS="\$(top_builddir)/src/arm/arm.la"
			LOADER_LIBS="-uarm_plugin $LOADER_DEPS"
		],
		AS_IF(test "$with_loader" = "s12x",
		[
			HAS_LOADER="yes"
			LOADER_DEPS="\$(top_builddir)/src/s12x/s12x.la"
			LOADER_LIBS="-us12x_plugin $LOADER_DEPS"
		],
		[
			AC_MSG_FAILURE([$with_loader loader not available !])
		])))
	])
	AS_IF(test "$HAS_LOADER" = "yes", AC_MSG_RESULT([$LOADER_LIBS]))
])
AM_CONDITIONAL(HAS_LOADER, test "$HAS_LOADER" = "yes")
AC_SUBST(LOADER_LIBS)
AC_SUBST(LOADER_DEPS)


# ===== for otawa-config =====
AS_IF( test "$HAS_PLUGIN" = "yes",
	[ HAS_SO="0" ],
	[ HAS_SO="1" ])
AC_SUBST(HAS_SO)


# ===== for date =====
DATE=`date +%Y.%m.%d-%H:%M:%S `
AS_AC_EXPAND(DATE, "$DATE")
AC_DEFINE_UNQUOTED(DATE, "$DATE", [Current date])


# ==== Output ====
#	src/glpk/Makefile
AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/prog/Makefile
	src/ets/Makefile
	src/ast/Makefile
	src/oshell/Makefile
	src/lp_solve/Makefile
	src/lp_solve5/Makefile
	src/dumpcfg/Makefile
	src/mkff/Makefile
	src/ostat/Makefile
	src/oipet/Makefile
	src/odisplay/Makefile
	src/ogensim/Makefile
	src/oloader/Makefile
	src/owcet/Makefile
	src/arm/Makefile
	src/ppc/Makefile
	src/s12x/Makefile
	src/bpred/Makefile
	src/opcg/Makefile
	examples/Makefile
	bin/Makefile
	test/Makefile
	test/checksum/Makefile
	test/ipet/Makefile
	test/ets/Makefile
	test/dom/Makefile
	test/dynproc/Makefile
	test/display/Makefile
	test/delta/Makefile
	test/ct/Makefile
	test/ccg/Makefile
	test/generic_simulator/Makefile
	test/graph/Makefile
	test/categorisation/Makefile
	test/hard/Makefile
	test/ilp/Makefile
	test/props/Makefile
	test/pcg/Makefile
	test/pfg/Makefile
	test/exegraph/Makefile
	test/reg/Makefile
	test/script/Makefile
	test/simulator/Makefile
	test/source/Makefile
	test/isp/Makefile
	examples/ets_timing/Makefile
	include/Makefile
	include/otawa/Makefile
	include/otawa/app/Makefile
	include/otawa/ast/Makefile
	include/otawa/cache/Makefile
	include/otawa/cache/categorisation/Makefile
	include/otawa/cache/cat2/Makefile
	include/otawa/cache/ccg/Makefile
	include/otawa/dcache/Makefile
	include/otawa/cfg/Makefile
	include/otawa/dfa/Makefile
	include/otawa/display/Makefile
	include/otawa/ets/Makefile
	include/otawa/exegraph/Makefile
	include/otawa/parexegraph/Makefile
	include/otawa/flowfact/Makefile
	include/otawa/gensim/Makefile
	include/otawa/graph/Makefile
	include/otawa/hard/Makefile
	include/otawa/hardware/Makefile
	include/otawa/ilp/Makefile
	include/otawa/ipet/Makefile
	include/otawa/loader/Makefile
	include/otawa/loader/new_gliss/Makefile
	include/otawa/loader/old_gliss/Makefile
	include/otawa/pcg/Makefile
	include/otawa/proc/Makefile
	include/otawa/prog/Makefile
	include/otawa/prop/Makefile
	include/otawa/script/Makefile
	include/otawa/sim/Makefile
	include/otawa/util/Makefile
	data/Makefile
	data/caches/Makefile
	data/makefiles/Makefile
	data/procs/Makefile
	data/scripts/Makefile
	doc/Makefile
])
AC_CONFIG_FILES([
	bin/otawa-config
	bin/otawa-new
], [], [
	echo "setting +x to bin/otawa-config"
	chmod +x bin/otawa-config
	echo "setting +x to bin/otawa-new"
	chmod +x bin/otawa-new
])
AC_OUTPUT()



