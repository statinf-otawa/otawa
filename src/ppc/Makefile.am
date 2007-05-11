loaderdir=$(libdir)/otawa/loader
loader_LTLIBRARIES=ppc.la

AM_CXXFLAGS = @GLISS_PPC_CXXFLAGS@ @OTAWA_CXXFLAGS@ @GEL_CXXFLAGS@

ppc_la_LDFLAGS = -module

ppc_la_LIBADD = @GLISS_PPC_LIBS@ @GEL_PPC_LIBS@ @OTAWA_LIBS@ ../oloader/liboloader.la

ppc_la_SOURCES = ppc.cpp


install-data-hook:
	rm -f $(libdir)/otawa/loader/elf_20.la
	$(LN_S) arm.la $(libdir)/otawa/loader/elf_20.la

