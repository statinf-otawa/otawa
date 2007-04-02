AM_CXXFLAGS = @OTAWA_CXXFLAGS@

AM_YFLAGS=-d

bin_PROGRAMS=oshell

oshell_LDADD = @LOADER_LIBS@ @OTAWA_LIBS@

oshell_DEPENDENCIES = @LOADER_DEPS@ @OTAWA_DEPS@

oshell_SOURCES= \
	parser.yy \
	lexer.ll \
	cursor.cpp \
	FunCursor.cpp \
	FrameWorkCursor.cpp \
	CFGCursor.cpp \
	FileCursor.cpp \
	SegmentCursor.cpp \
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
