lib_LIBRARIES=libothept.a

libothept_a_SOURCES =  \
	Loader.cpp \
	parser.yy \
	lexer.ll \
	Process.cpp

noinst_HEADERS = \
	HeptLoader.h \
	Loader.h \
	Process.h \
	parser.h

INCLUDES= \
	-I../../include \
	@ELM_CFLAGS@ \
	-I../gliss \
	@GLISSPPC_CFLAGS@
CXXFLAGS=-g3
YFLAG=-d
