noinst_PROGRAMS=test_dom

test_dom_SOURCES=\
	test_dom.cpp

INCLUDES=\
	-I../../include \
	@ELM_CFLAGS@
CXXFLAGS=-g3
	
test_dom_LDADD= \
	-L../../src/prog -lprog \
	@ELM_LDADD@ \
	-L../../src/gliss -lgliss \
	@GLISSPPC_LDADD@

test_dom_DEPENDENCIES= \
	../../src/prog/libprog.a \
	../../src/gliss/libgliss.a
