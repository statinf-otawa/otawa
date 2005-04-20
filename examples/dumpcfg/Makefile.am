bin_PROGRAMS=dumpcfg
dumpcfg_SOURCES= \
	dumpcfg.cpp \
	SimpleDisplayer.cpp \
	DiassemblerDisplayer.cpp

CXXFLAGS=-g3

INCLUDES= \
	-I../../include \
	@ELM_CFLAGS@

dumpcfg_LDADD= \
	-L../../src/prog -lprog \
	@ELM_LDADD@ \
	-L../../src/gliss -lgliss \
	@GLISSPPC_LDADD@

dumpcfg_DEPENDENCIES= \
	../../src/gliss/libgliss.a \
	../../src/prog/libprog.a
