AM_CXXFLAGS = @OGENSIM_CXXFLAGS@ @OTAWA_CXXFLAGS@

noinst_PROGRAMS=test_aggregation

test_aggregation_LDADD = @LOADER_LIBS@ @ILP_LIBS@ @OGENSIM_LIBS@ @OTAWA_LIBS@
test_aggregation_DEPENDENCIES = @LOADER_DEPS@ @ILP_DEPS@ @OGENSIM_DEPS@ @OTAWA_DEP@

test_aggregation_SOURCES=\
	../delta/BBPath.cpp \
	BBTimeSilumator.h \
	BBTimeSimulator.cpp \
	Header.h \
	Header.cpp \
	AggregationGraph.h \
	AggregationGraph.cpp \
	EnhacedAggregationGraph.h \
	EnhacedAggregationGraph.cpp \
	SequenceBasicBlock.h \
	SequenceBasicBlock.cpp \
	test_aggregation.cpp
        
