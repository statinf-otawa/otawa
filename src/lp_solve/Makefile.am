AM_CXXFLAGS = @OTAWA_CXXFLAGS@ @LP_SOLVE_CXXFLAGS@ -Wall

ilpdir=$(libdir)/otawa/ilp
ilp_LTLIBRARIES = lp_solve.la

lp_solve_la_LDFLAGS = -module
lp_solve_la_LIBADD = @OTAWA_LIBS@ @LP_SOLVE_LIBS@ @PLUGIN_LIBS@
lp_solve_la_DEPENDENCIES = @OTAWA_DEPS@

lp_solve_la_SOURCES = \
	lp_solve_System.cpp \
	lp_solve_Constraint.cpp \
	lp_solve_Plugin.cpp
