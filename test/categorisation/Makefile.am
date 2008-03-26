AM_CXXFLAGS = @OTAWA_CXXFLAGS@ -Wall

check_PROGRAMS=test_categorisation

test_categorisation_LDADD= @LOADER_LIBS@ @ILP_LIBS@ @OTAWA_LIBS@
test_categorisation_DEPENDENCIES= @LOADER_DEPS@ @ILP_DEPS@ @OTAWA_DEPS@

test_categorisation_SOURCES=\
	test-categorisation.cpp

TESTS = test.sh
