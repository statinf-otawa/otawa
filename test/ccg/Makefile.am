AM_CXXFLAGS = @OTAWA_CXXFLAGS@

check_PROGRAMS=test_ccg

test_ccg_LDADD = @ILP_LIBS@ @LOADER_LIBS@ @OTAWA_LIBS@
test_ccg_DEPENDENCIES= @LOADER_DEPS@ @ILP_DEPS@ @OTAWA_DEPS@

test_ccg_SOURCES=\
	test-ccg.cpp

TESTS = test.sh
