lib_LIBRARIES=libothept.a

libothept_a_SOURCES =  \
	Loader.cpp \
	parser.yy \
	lexer.ll \
	Process.cpp
	
INCLUDES= \
	-I../../include \
	-I@ELM_PATH@/include \
	-I../gliss \
	@GLISSPPC_CFLAGS@
CXXFLAGS=-g3
YFLAG=-d
