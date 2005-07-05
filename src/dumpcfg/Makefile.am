bin_PROGRAMS=dumpcfg
dumpcfg_SOURCES= \
	dumpcfg.cpp \
	SimpleDisplayer.cpp \
	DisassemblerDisplayer.cpp \
	DotDisplayer.cpp

dumpcfg_LDADD= \
	-L../gliss -lgliss \
	-L../prog -lprog

dumpcfg_DEPENDENCIES= \
	..//gliss/libgliss.a \
	../prog/libprog.a
