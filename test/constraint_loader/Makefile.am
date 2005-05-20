noinst_PROGRAMS=test_constraint_loader

test_constraint_loader_SOURCES=\
	test_constraint_loader.cpp

INCLUDES=\
	-I../../include \
	@ELM_CFLAGS@
CXXFLAGS=-g3
	
test_constraint_loader_LDADD= \
	-L../../src/prog -lprog \
	@ELM_LDADD@ \
	-L../../src/gliss -lgliss \
	@GLISSPPC_LDADD@ \
	-L../../src/lp_solve -lotawa_lp_solve \
	@LP_SOLVE_LDADD@

test_constraint_loader_DEPENDENCIES= \
	../../src/prog/libprog.a \
	../../src/gliss/libgliss.a \
	../../src/lp_solve/libotawa_lp_solve.a
