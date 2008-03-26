AM_CXXFLAGS = @OTAWA_CXXFLAGS@ @GLISS_PPC_CXXFLAGS@ -Wall

check_PROGRAMS=simulator

simulator_LDADD = @LOADER_LIBS@ @OGENSIM_LIBS@ @OTAWA_LIBS@ ../../src/ppc/ppc.la
simulator_DEPENDENCIES = @LOADER_DEPS@ @OGENSIM_DEPS@ @OTAWA_DEPS@ ../../src/ppc/ppc.la

simulator_SOURCES=\
	gen_simulator.cpp \
	FullSimulationDriver.cpp
