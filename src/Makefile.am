SUBDIRS=prog @GLISSPPC_LIB@ @HEPTANE_LIB@ oshell
noinst_PROGRAMS=test
test_SOURCES= \
	test.cpp

CXXFLAGS=-g3
INCLUDES=-I../include @ELM_CFLAGS@
test_LDADD=@ELM_LDADD@ -Lprog -lprog -lelm @GLISSPPC_LDADD@ 
