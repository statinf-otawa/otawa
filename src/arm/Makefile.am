loaderdir=$(libdir)/otawa/loader
loader_LTLIBRARIES=arm.la

AM_CXXFLAGS = @ARM_PPC_CXXFLAGS@ @ARM_CXXFLAGS@

arm_la_LDFLAGS = -module

arm_la_LIBADD = @ARM_PPC_LIBS@ @OTAWA_LIBS@

arm_la_SOURCES =


install-data-hook:
	rm -f $(libdir)/otawa/loader/elf_40.la
	$(LN_S) arm.la $(libdir)/otawa/loader/elf_40.la

