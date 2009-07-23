bin_PROGRAMS=dumpcfg
dumpcfg_SOURCES= \
	dumpcfg.cpp \
	SimpleDisplayer.cpp \
	DisassemblerDisplayer.cpp \
	DotDisplayer.cpp

dumpcfg_LDADD = @LOADER_LIBS@ @OTAWA_LIBS@ @PROG_LIBS@
dumpcfg_DEPENDENCIES = @LOADER_DEPS@ @OTAWA_DEPS@

AM_CXXFLAGS = @OTAWA_CXXFLAGS@ -Wall
