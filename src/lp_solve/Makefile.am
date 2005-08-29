if WITH_PLUGIN

ilpdir=$(libdir)/otawa/ilp
ilp_PROGRAMS = lp_solve.so
lp_solve_so_SOURCES = \
	lp_solve_System.cpp \
	lp_solve_Constraint.cpp \
	lp_solve_Plugin.cpp
lp_solve_so_LDFLAGS	= -shared
lp_solve_so_LDADD = @PROG_LIBS@ @BUILD_LP_SOLVE_LIBS@
AM_CXXFLAGS=@PROG_CXXFLAGS@ @BUILD_LP_SOLVE_CXXFLAGS@ -fPIC

else

lib_LIBRARIES = libotawa_lp_solve.a
libotawa_lp_solve_a_SOURCES = \
	lp_solve_System.cpp \
	lp_solve_Constraint.cpp
AM_CXXFLAGS=@PROG_CXXFLAGS@ @BUILD_LP_SOLVE_CXXFLAGS@

endif

