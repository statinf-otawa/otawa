bin_PROGRAMS=dumpcfg
dumpcfg_SOURCES= \
	dumpcfg.cpp

CXXFLAGS=-g3
INCLUDES=-I../../include -I../../elm/include
dumpcfg_LDADD= \
	-L../../elm/src \
	-L../../src/prog \
	-L../../src/gliss \
	-L../../gliss-ppc/lib \
	-lgliss -lprog -lelm -lemul

dumpcfg_DEPENDENCIES= \
	../../src/gliss/libgliss.a \
	../../src/prog/libprog.a
