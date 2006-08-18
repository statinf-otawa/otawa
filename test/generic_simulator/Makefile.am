noinst_PROGRAMS=simulator

simulator_SOURCES=\
	gen_simulator.cpp \
	GenericSimulator.cpp \
	GenericProcessor.cpp \
	InstructionQueue.cpp \
	PipelineStage.cpp \
	Fetch.cpp \
	Execute.cpp

simulator_LDADD = @LOADER_PLUGIN@  @ILP_PLUGIN@ @PROG_LIBS@ @SYSTEMC_LIBS@

simulator_DEPENDENCIES = @LOADER_DEP@ @ILP_DEP@ @PROG_DEP@

AM_CXXFLAGS = @BUILD_GLISS_PPC_CXXFLAGS@ @PROG_CXXFLAGS@ @SYSTEMC_CXXFLAGS@
