ilpdir=$(libdir)/otawa/ilp
ilp_LTLIBRARIES = lp_solve.la

AM_CXXFLAGS = @PROG_CXXFLAGS@ @BUILD_LP_SOLVE_CXXFLAGS@

lp_solve_la_LDFLAGS = -module
lp_solve_la_LIBADD = @PROG_LIBS@ @BUILD_LP_SOLVE_LIBS@

lp_solve_la_SOURCES = \
	lp_solve_System.cpp \
	lp_solve_Constraint.cpp \
	lp_solve_Plugin.cpp
