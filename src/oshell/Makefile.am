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
INCLUDES=-I../../include -I../../elm/include
oshell_LDADD=-L../../elm/src -L../prog -L../gliss -L../../gliss-ppc/lib -L../heptane -lothept -lgliss -lprog -lelm -lemul

oshell_DEPENDENCIES=../gliss/libgliss.a ../prog/libprog.a ../heptane/libothept.a
