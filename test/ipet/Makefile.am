noinst_PROGRAMS=test_ipet

test_ipet_SOURCES=\
	test_ipet.cpp

INCLUDES=\
	-I../../include \
	@ELM_CFLAGS@
CXXFLAGS=-g3
	
test_ipet_LDADD= \
	-L../../src/prog -lprog \
	@ELM_LDADD@ \
	-L../../src/gliss -lgliss \
	@GLISSPPC_LDADD@ \
	-L../../src/lp_solve -lotawa_lp_solve \
	@LP_SOLVE_LDADD@

test_ipet_DEPENDENCIES= \
	../../src/prog/libprog.a \
	../../src/gliss/libgliss.a \
	../../src/lp_solve/libotawa_lp_solve.a
