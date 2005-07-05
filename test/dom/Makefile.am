noinst_PROGRAMS=test_dom

test_dom_SOURCES=\
	test_dom.cpp

test_dom_LDADD= \
	-L../../src/prog -lprog \
	-L../../src/gliss -lgliss \
	@GLISS_PPC_LIBS@

test_dom_DEPENDENCIES= \
	../../src/prog/libprog.a \
	../../src/gliss/libgliss.a
