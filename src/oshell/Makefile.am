bin_PROGRAMS=oshell
oshell_SOURCES= \
	parser.yy \
	lexer.ll \
	cursor.cpp \
	FunCursor.cpp \
	FrameWorkCursor.cpp \
	CFGCursor.cpp \
	FileCursor.cpp \
	SegmentCursor.cpp \
	CodeCursor.cpp \
	InstCursor.cpp

YFLAGS=-d
CXXFLAGS=-g3
INCLUDES=-I../../include -I@ELM_PATH@/include
oshell_LDADD= \
	-L../heptane -lothept \
	@GLISSPPC_LDADD@ \
	-L../prog -lprog \
	-L@ELM_PATH@/src -lelm

oshell_DEPENDENCIES=../gliss/libgliss.a ../prog/libprog.a ../heptane/libothept.a
