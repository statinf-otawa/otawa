loaderdir=$(libdir)/otawa/loader
loader_LTLIBRARIES=s12x.la

AM_CXXFLAGS = @GLISS_S12X_CXXFLAGS@ @OTAWA_CXXFLAGS@ @GEL_CXXFLAGS@

s12x_la_LDFLAGS = -module

s12x_la_LIBADD = @GLISS_S12X_LIBS@ @GEL_PPC_LIBS@ @OTAWA_LIBS@ ../oloader/liboloader.la

s12x_la_SOURCES = s12x.cpp


install-data-hook:
	rm -f $(libdir)/otawa/loader/elf_53.la
	$(LN_S) s12x.la $(libdir)/otawa/loader/elf_53.la

