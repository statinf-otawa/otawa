AM_CXXFLAGS=@OTAWA_CXXFLAGS@ @SYSTEMC_CXXFLAGS@ -Wall -O2

lib_LTLIBRARIES=libogensim.la

libogensim_la_LDFLAGS=
libogensim_la_LIBADD= @OTAWA_LIBS@ @SYSTEMC_LIBS@ 

libogensim_la_SOURCES = \
	Cache.cpp \
	Execute.cpp \
	Fetch.cpp \
	FullSimulationDriver.cpp \
	GenericProcessor.cpp \
	GenericSimulator.cpp \
	InstructionQueue.cpp \
	LazyStage.cpp \
	Memory.cpp \
	MemoryDriver.cpp \
	PipelineStage.cpp \
	GenericState.cpp
