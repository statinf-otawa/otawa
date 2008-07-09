AM_CXXFLAGS = @OTAWA_CXXFLAGS@ -Wall

check_PROGRAMS=test_ets

test_ets_LDADD = \
	 ../../src/ets/ets.la \
	 ../../src/ast/ast.la \
	@LOADER_LIBS@ \
	@OTAWA_LIBS@
test_ets_DEPENDENCIES = \
	../../src/ets/ets.la \
	 ../../src/ast/ast.la \
	@LOADER_DEPS@ \
	@OTAWA_DEPS@

test_ets_SOURCES=\
	test_ets.cpp 
