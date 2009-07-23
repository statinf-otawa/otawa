loaderdir=$(libdir)/otawa/loader
loader_LTLIBRARIES=s12x.la
check_PROGRAMS=test_s12x

AM_CXXFLAGS = @GLISS_S12X_CXXFLAGS@ @OTAWA_CXXFLAGS@ @GEL_CXXFLAGS@


# Library configuration
s12x_la_LDFLAGS = -module

s12x_la_LIBADD = \
	@GLISS_S12X_LIBS@ \
	@GEL_PPC_LIBS@ \
	../oloader/liboloader.la \
	@GEL_DWARF_LIBS@ \
	@OTAWA_LIBS@ \
	@PLUGIN_LIBS@

s12x_la_SOURCES = \
	s12x.cpp \
	s12xsim.cpp


install-data-hook:
	rm -f $(libdir)/otawa/loader/elf_53.la
	$(LN_S) s12x.la $(libdir)/otawa/loader/elf_53.la


# Test program configuration
test_s12x_LDADD = @LOADER_LIBS@ @ILP_LIBS@ @ODISPLAY_LIBS@ @OTAWA_LIBS@

test_s12x_DEPENDENCIES = s12x.la @ILP_DEPS@ @ODISPLAY_DEPS@ @OTAWA_DEPS@ @ODISPLAY_DEPS@ 

test_s12x_SOURCES = test.cpp

