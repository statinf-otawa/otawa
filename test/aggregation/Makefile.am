noinst_PROGRAMS=test_aggregation

test_aggregation_SOURCES=\
		../delta/BBPath.cpp \
		BBTimeSilumator.h \
		BBTimeSimulator.cpp \
		LoopHeader.h \
		LoopHeader.cpp \
		AggregationGraph.h \
		AggregationGraph.cpp \
		SequenceBasicBlock.h \
		SequenceBasicBlock.cpp \
        test_aggregation.cpp
        

test_aggregation_LDADD = @LOADER_PLUGIN@  @ILP_PLUGIN@ @OGENSIM_LIBS@ @PROG_LIBS@

test_aggregation_DEPENDENCIES = @LOADER_DEP@ @ILP_DEP@ @OGENSIM_DEP@ @PROG_DEP@

AM_CXXFLAGS = @BUILD_GLISS_PPC_CXXFLAGS@ @PROG_CXXFLAGS@ @OGENSIM_CXXFLAGS@