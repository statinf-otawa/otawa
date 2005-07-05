noinst_PROGRAMS=test_ipet

test_ipet_SOURCES=\
	test_ipet.cpp

test_ipet_LDADD= \
	-L../../src/prog -lprog \
	-L../../src/gliss -lgliss \
	-L../../src/lp_solve -lotawa_lp_solve \
	@LP_SOLVE_LIBS@

test_ipet_DEPENDENCIES= \
	../../src/prog/libprog.a \
	../../src/gliss/libgliss.a \
	../../src/lp_solve/libotawa_lp_solve.a
