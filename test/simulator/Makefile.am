noinst_PROGRAMS=simulator

simulator_SOURCES=\
	simulator.cpp \
	ARMSimulator.cpp \
	ARMProcessor.cpp

SYSTEMC_DIR = /home/rochange/systemc-2.1.v1

simulator_LDADD = @LOADER_PLUGIN@  @ILP_PLUGIN@ @PROG_LIBS@ -L$(SYSTEMC_DIR)/lib-linux -lsystemc

simulator_DEPENDENCIES = @LOADER_DEP@ @ILP_DEP@ @PROG_DEP@

AM_CXXFLAGS = @BUILD_GLISS_PPC_CXXFLAGS@ @PROG_CXXFLAGS@ -I$(SYSTEMC_DIR)/include
