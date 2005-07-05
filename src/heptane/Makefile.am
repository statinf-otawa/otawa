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
	-I../gliss \
	@GLISS_PPC_CXXFLAGS@
AM_YFLAG=-dv
