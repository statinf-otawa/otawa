bin_PROGRAMS=oshell
oshell_SOURCES= \
	parser.yy \
	lexer.ll \
	cursor.cpp

YFLAGS=-d
CXXFLAGS=-g3
INCLUDES=-I../../include -I../../elm/include
oshell_LDADD=-L../../elm/src -L../prog -L../gliss -L../../gliss-ppc/lib -lgliss -lprog -lelm -lemul

