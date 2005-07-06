noinst_PROGRAMS=test_lp_solve

test_lp_solve_SOURCES=\
	test.cpp

test_lp_solve_LDADD= \
	-L../../src/prog -lprog \
	-L../../src/lp_solve -lotawa_lp_solve \
	@LP_SOLVE_LIBS@

test_lp_solve_DEPENDENCIES= \
	../../src/prog/libprog.a \
	../../src/lp_solve/libotawa_lp_solve.a
