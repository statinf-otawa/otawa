SUBDIRS=prog @GLISSPPC_LIB@ @HEPTANE_LIB@ oshell
bin_PROGRAMS=test
test_SOURCES= \
	test.cpp

CXXFLAGS=-g3
INCLUDES=-I../include -I@ELM_PATH@/include
test_LDADD=-L@ELM_PATH@/src -Lprog -lprog -lelm @GLISSPPC_LDADD@ 
