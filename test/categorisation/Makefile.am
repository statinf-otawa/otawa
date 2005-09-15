noinst_PROGRAMS=test-categorisation

test_categorisation_SOURCES=\
	test-categorisation.cpp

test_categorisation_LDADD= \
	-L../../src/prog -lprog \
	-L../../src/gliss -lgliss \
	-L../../src/lp_solve -lotawa_lp_solve \
	@LP_SOLVE_LIBS@

test_categorisation_DEPENDENCIES= \
	../../src/prog/libprog.a \
	../../src/gliss/libgliss.a \
	../../src/lp_solve/libotawa_lp_solve.a
