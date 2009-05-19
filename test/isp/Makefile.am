AM_CXXFLAGS = @OTAWA_CXXFLAGS@ -Wall -DTRACE_FOR_CHECKING

check_PROGRAMS=isp

isp_LDADD = @LOADER_LIBS@  @ILP_LIBS@ @OTAWA_LIBS@ 
isp_DEPENDENCIES = @LOADER_DEPS@ @ILP_DEPS@ @OTAWA_DEPS@

isp_SOURCES=\
	isp.cpp \
	FunctionBlockBuilder.cpp \
	FunctionBlock.cpp

