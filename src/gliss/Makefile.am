loaderdir=$(libdir)/otawa/loader
loader_LTLIBRARIES=ppc.la

AM_CXXFLAGS = @BUILD_GLISS_PPC_CXXFLAGS@ @PROG_CXXFLAGS@

ppc_la_LDFLAGS = -module
ppc_la_LIBADD = @BUILD_GLISS_PPC_LIBS@ @PROG_LIBS@

ppc_la_SOURCES = \
	loader.cpp \
	platform.cpp \
	Process.cpp \
	File.cpp \
	CodeSegment.cpp \
	Inst.cpp \
	ControlInst.cpp \
	Symbol.cpp \
	MemInst.cpp
