AM_CXXFLAGS = @OTAWA_CXXFLAGS@ -Wall

# processor plugin
dyndir=$(abs_builddir)/.otawa/proc
dyn_LTLIBRARIES=dyn.la

dyn_la_LDFLAGS = -module
dyn_la_LIBADD = @OTAWA_LIBS@
dyn_la_SOURCES = dyn.cpp


# test dynproc
check_PROGRAMS=test_dynproc

test_dynproc_LDADD = @LOADER_LIBS@ @OTAWA_LIBS@
test_dynproc_DEPENDENCIES = @LOADER_DEPS@ @OTAWA_DEPS@

test_dynproc_SOURCES=\
	test_dynproc.cpp
