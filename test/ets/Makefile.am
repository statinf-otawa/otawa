noinst_PROGRAMS=test_ets

test_ets_SOURCES=\
	test_ets.cpp 
	
AM_CXXFLAGS=-g3
	
test_ets_LDADD= \
	-L../../src/gliss -lgliss \
	-L../../src/prog -lprog \
	@GLISS_PPC_LIBS@

test_ets_DEPENDENCIES= \
	../../src/prog/libprog.a \
	../../src/gliss/libgliss.a 
