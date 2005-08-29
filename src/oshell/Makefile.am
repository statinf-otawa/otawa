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

oshell_LDADD = @LOADER_PLUGIN@ @PROG_LIBS@

oshell_DEPENDENCIES = @LOADER_DEP@ @PROG_DEP@

AM_CXXFLAGS = @PROG_CXXFLAGS@

AM_YFLAGS=-d

