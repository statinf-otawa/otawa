noinst_PROGRAMS=test_lp_solve

test_lp_solve_SOURCES=\
	test.cpp

test_lp_solve_LDADD = ../../src/lp_solve/*.o @BUILD_LP_SOLVE_LIBS@ @PROG_LIBS@

test_lp_solve_DEPENDENCIES = @LP_SOLVE_DEP@ @PROG_DEP@

AM_CXXFLAGS = @PROG_CXXFLAGS@
