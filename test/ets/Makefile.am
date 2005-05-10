noinst_PROGRAMS=test_ets

test_ets_SOURCES=\
	test_ets.cpp 
	
INCLUDES=\
	-I../../include \
	@ELM_CFLAGS@
CXXFLAGS=-g3
	
test_ets_LDADD= \
	-L../../src/heptane -lothept \
	@GLISSPPC_LDADD@ \
	-L../../src/gliss -lgliss \
	-L../../src/prog -lprog \
	@ELM_LDADD@ -lelm

test_ets_DEPENDENCIES= \
	../../src/heptane/libothept.a \
	../../src/prog/libprog.a \
	../../src/gliss/libgliss.a 
