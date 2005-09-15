noinst_PROGRAMS=test-ccg

test_ccg_SOURCES=\
	test-ccg.cpp

test_ccg_LDADD= \
	-L../../src/prog -lprog \
	-L../../src/gliss -lgliss \
	-L../../src/lp_solve -lotawa_lp_solve \
	@LP_SOLVE_LIBS@

test_ccg_DEPENDENCIES= \
	../../src/prog/libprog.a \
	../../src/gliss/libgliss.a \
	../../src/lp_solve/libotawa_lp_solve.a
