loaderdir=$(libdir)/otawa/loader
loader_LTLIBRARIES=ppc.la

AM_CXXFLAGS = @GLISS_PPC_CXXFLAGS@ @GEL_CXXFLAGS@ @OTAWA_CXXFLAGS@

ppc_la_LDFLAGS = -module

ppc_la_LIBADD = @GLISS_PPC_LIBS@ @GEL_PPC_LIBS@ @OTAWA_LIBS@

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
