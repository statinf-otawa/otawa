AM_CXXFLAGS = @OTAWA_CXXFLAGS@ -Wall

check_PROGRAMS=test_delta

test_delta_LDADD = @LOADER_LIBS@  @ILP_LIBS@ @OGENSIM_LIBS@ @ODISPLAY_LIBS@ @OTAWA_LIBS@
test_delta_DEPENDENCIES = @LOADER_DEPS@ @ILP_DEPS@ @OGENSIM_DEPS@ @ODISPLAY_DEPS@ @OTAWA_DEPS@

test_delta_SOURCES=\
	DeltaCFGDrawer.cpp \
	DeltaCFGDrawer.h \
	test_delta.cpp

TESTS = test.sh
