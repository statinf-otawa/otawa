bin_PROGRAMS=dumpcfg
dumpcfg_SOURCES= \
	dumpcfg.cpp \
	SimpleDisplayer.cpp \
	DisassemblerDisplayer.cpp \
	DotDisplayer.cpp

AM_CXXFLAGS=-g3

INCLUDES= \
	-I../../include \
	@ELM_CFLAGS@

dumpcfg_LDADD= \
	-L../prog -lprog \
	@ELM_LDADD@ \
	-L../gliss -lgliss \
	@GLISSPPC_LDADD@

dumpcfg_DEPENDENCIES= \
	..//gliss/libgliss.a \
	../prog/libprog.a
