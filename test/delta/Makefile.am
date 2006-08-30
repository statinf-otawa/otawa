noinst_PROGRAMS=test_delta

test_delta_SOURCES=\
	BBTimeSimulator.h \
	BBTimeSimulator.cpp \
	DeltaCFGDrawer.cpp \
	DeltaCFGDrawer.h \
	BBPath.cpp \
	test_delta.cpp
        

test_delta_LDADD = @LOADER_PLUGIN@  @ILP_PLUGIN@ @OGENSIM_LIBS@ @ODISPLAY_LIBS@ @PROG_LIBS@

test_delta_DEPENDENCIES = @LOADER_DEP@ @ILP_DEP@ @OGENSIM_DEP@ @ODISPLAY_DEP@ @PROG_DEP@

AM_CXXFLAGS = @BUILD_GLISS_PPC_CXXFLAGS@ @PROG_CXXFLAGS@ @OGENSIM_CXXFLAGS@
