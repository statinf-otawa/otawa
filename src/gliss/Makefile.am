lib_LIBRARIES=libgliss.a

libgliss_a_SOURCES = \
	loader.cpp \
	platform.cpp \
	Process.cpp \
	File.cpp \
	CodeSegment.cpp \
	Inst.cpp \
	ControlInst.cpp \
	Symbol.cpp

INCLUDES = \
	-I../../include \
	@ELM_CFLAGS@ \
	@GLISSPPC_CFLAGS@
AM_CXXFLAGS=-g3
