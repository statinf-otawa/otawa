bin_PROGRAMS=mkff
mkff_SOURCES= \
	mkff.cpp

AM_CXXFLAGS=-g3

INCLUDES= \
	-I../../include \
	@ELM_CFLAGS@

mkff_LDADD= \
	@GLISSPPC_LDADD@ \
	-L../prog -lprog \
	@ELM_LDADD@

mkff_DEPENDENCIES= \
	../gliss/libgliss.a \
	../prog/libprog.a
