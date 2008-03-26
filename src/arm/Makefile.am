loaderdir=$(libdir)/otawa/loader
loader_LTLIBRARIES=arm.la

AM_CXXFLAGS = @GLISS_ARM_CXXFLAGS@ @OTAWA_CXXFLAGS@ -Wall

arm_la_LDFLAGS = -module

arm_la_LIBADD = @GLISS_ARM_LIBS@ @OTAWA_LIBS@ ../oloader/liboloader.la

arm_la_SOURCES = arm.cpp


install-data-hook:
	rm -f $(libdir)/otawa/loader/elf_40.la
	$(LN_S) arm.la $(libdir)/otawa/loader/elf_40.la

