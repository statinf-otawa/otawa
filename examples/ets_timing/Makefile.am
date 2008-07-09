AM_CXXFLAGS = @OTAWA_CXXFLAGS@

noinst_PROGRAMS=ets_timing

ets_timing_LDADD =  \
	@LOADER_LIBS@ \
	@OTAWA_LIBS@ \
	../../src/ast/ast.la \
	../../src/ets/ets.la
ets_timing_DEPENDENCIES = \
	@LOADER_DEPS@ \
	@OTAWA_DEPS@ \
	../../src/ast/ast.la \
	../../src/ets/ets.la

ets_timing_SOURCES=\
	ets_timing.cpp 
