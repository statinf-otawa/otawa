bin_PROGRAMS=dumpcfg
dumpcfg_SOURCES= \
	dumpcfg.cpp

CXXFLAGS=-g3

INCLUDES= \
	-I../../include \
	-I@ELM_PATH@/include

dumpcfg_LDADD= \
	-L../../src/prog -lprog \
	-L@ELM_PATH@/src -lelm \
	-L../../src/gliss -lgliss \
	@GLISSPPC_LDADD@

dumpcfg_DEPENDENCIES= \
	../../src/gliss/libgliss.a \
	../../src/prog/libprog.a
