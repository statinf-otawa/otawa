AM_CXXFLAGS = @OTAWA_CXXFLAGS@ @LP_SOLVE_CXXFLAGS@

ilpdir=$(libdir)/otawa/ilp
ilp_LTLIBRARIES = lp_solve5.la

lp_solve5_la_LDFLAGS = -module
lp_solve5_la_LIBADD = @OTAWA_LIBS@ @LP_SOLVE5_LIBS@
lp_solve5_la_DEPENDENCIES = @OTAWA_DEPS@

lp_solve5_la_SOURCES = lp_solve5.cpp
