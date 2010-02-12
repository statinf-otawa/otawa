loaderdir=$(libdir)/otawa/loader
loader_LTLIBRARIES=ppc.la

AM_CXXFLAGS = \
	@GLISS2_PPC_CXXFLAGS@ \
	@OTAWA_CXXFLAGS@ \
	@GEL_CXXFLAGS@ \
	-I../../../ppc2/src

ppc_la_LDFLAGS = -module

ppc_la_LIBADD = \
	@GLISS2_PPC_LIBS@ \
	@OTAWA_LIBS@ \
	@GEL_PPC_LIBS@ \
	@GEL_DWARF_LIBS@ \
	@PLUGIN_LIBS@

ppc_la_SOURCES = \
	ppc.cpp


# install alias
install-data-hook:
	rm -f $(libdir)/otawa/loader/elf_20.la
	$(LN_S) ppc.la $(libdir)/otawa/loader/elf_20.la


# generate files
PPC2_IRG=../../../ppc2/ppc.irg
otawa_kind.h: kind.tpl $(PPC2_IRG)
	@GLISS2_ATTR_PATH@ $(PPC2_IRG) -o $@ -a otawa_kind -f -t $<

otawa_target.h: target.tpl $(PPC2_IRG)
	@GLISS2_ATTR_PATH@ $(PPC2_IRG) -o $@ -a otawa_target -d "return 0;" -f -t $<

otawa_used_regs.h: used_regs.tpl $(PPC2_IRG)
	@GLISS2_ATTR_PATH@ $(PPC2_IRG) -o $@ -a otawa_used_regs -d "tmp_var[0] = END_REG; return 0;" -f -t $<

ppc.lo: otawa_kind.h otawa_target.h otawa_used_regs.h
