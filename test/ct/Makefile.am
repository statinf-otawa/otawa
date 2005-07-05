noinst_PROGRAMS=test_ct

test_ct_SOURCES=\
	test_ct.cpp

test_ct_LDADD= \
	-L../../src/prog -lprog \
	-L../../src/gliss -lgliss \
	@GLISS_PPC_LIBS@

test_ct_DEPENDENCIES= \
	../../src/prog/libprog.a \
	../../src/gliss/libgliss.a
