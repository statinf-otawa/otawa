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

noinst_HEADERS = \
	CFGCursor.h \
	CodeCursor.h \
	FileCursor.h \
	FrameWorkCursor.h \
	FunCursor.h \
	InstCursor.h \
	oshell.h \
	parser.h \
	SegmentCursor.h

AM_YFLAGS=-d
oshell_LDADD= \
	-L../heptane -lothept \
	-L../gliss -lgliss \
	-L../prog -lprog

oshell_DEPENDENCIES=../gliss/libgliss.a ../prog/libprog.a ../heptane/libothept.a
