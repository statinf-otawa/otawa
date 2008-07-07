AM_CXXFLAGS = @OTAWA_CXXFLAGS@ -Wall

check_PROGRAMS=test_ets

test_ets_LDADD = @LOADER_LIBS@ @OTAWA_LIBS@ ../../src/ets/ets.la
test_ets_DEPENDENCIES = @LOADER_DEPS@ @OTAWA_DEPS@ ../../src/ets/ets.la

test_ets_SOURCES=\
	test_ets.cpp 
