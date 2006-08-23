lib_LTLIBRARIES=libogensim.la

AM_CXXFLAGS=@PROG_CXXFLAGS@ @SYSTEMC_CXXFLAGS@ @BUILD_GLISS_PPC_CXXFLAGS@

libogensim_la_LDFLAGS=
libogensim_la_LIBADD= @PROG_LIBS@ @SYSTEMC_LIBS@

libogensim_la_SOURCES = \
	Execute.cpp \
	Fetch.cpp \
	GenericProcessor.cpp \
	GenericSimulator.cpp \
	InstructionQueue.cpp \
	PipelineStage.cpp
