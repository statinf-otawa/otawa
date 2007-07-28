AM_CXXFLAGS = @OTAWA_CXXFLAGS@

check_PROGRAMS=test_constraint_loader

test_constraint_loader_LDADD= @LOADER_LIBS@ @ILP_LIBS@ @OTAWA_LIBS@
test_constraint_loader_DEPENDENCIES= @LOADER_DEPS@ @ILP_DEPS@ @OTAWA_DEPS@

test_constraint_loader_SOURCES=\
	test_constraint_loader.cpp
