noinst_PROGRAMS=test

test_SOURCES=\
	test.cpp

INCLUDES=\
	-I../../include \
	@ELM_CFLAGS@
CXXFLAGS=-g3
	
test_LDADD= \
	-L../../src/prog -lprog \
	@ELM_LDADD@ \
	-L../../src/lp_solve -lotawa_lp_solve \
	@LP_SOLVE_LDADD@

test_DEPENDENCIES= \
	../../src/prog/libprog.a \
	../../src/lp_solve/libotawa_lp_solve.a

	