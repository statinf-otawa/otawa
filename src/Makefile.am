SUBDIRS=prog gliss heptane oshell
bin_PROGRAMS=test
test_SOURCES= \
	test.cpp

CXXFLAGS=-g3
INCLUDES=-I../include -I../elm/include
test_LDADD=-L../elm/src -Lprog -Lgliss -L../gliss-ppc/lib -lgliss -lprog -lelm -lemul
