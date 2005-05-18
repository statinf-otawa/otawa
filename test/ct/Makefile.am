noinst_PROGRAMS=test_ct

test_ct_SOURCES=\
	test_ct.cpp

INCLUDES=\
	-I../../include \
	@ELM_CFLAGS@
CXXFLAGS=-g3
	
test_ct_LDADD= \
	-L../../src/prog -lprog \
	@ELM_LDADD@ \
	-L../../src/gliss -lgliss \
	@GLISSPPC_LDADD@

test_ct_DEPENDENCIES= \
	../../src/prog/libprog.a \
	../../src/gliss/libgliss.a
