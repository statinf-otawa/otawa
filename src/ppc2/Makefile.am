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
	rm -f $(libdir)/otawa/proc/otawa/ppc.la
	$(LN_S) $(libdir)/otawa/loader/ppc.la $(libdir)/otawa/proc/otawa/ppc.la
	cp ppc-config.xml $(libdir)/otawa/loader/


# generate files
PPC2_IRG=../../../ppc2/ppc.irg
otawa_kind.h: kind.tpl $(PPC2_IRG) otawa_kind.nmp
	@GLISS2_ATTR_PATH@ $(PPC2_IRG) -o $@ -a otawa_kind -f -d "return 0;" -t $< -e otawa_kind.nmp

otawa_target.h: target.tpl $(PPC2_IRG) otawa_target.nmp
	@GLISS2_ATTR_PATH@ $(PPC2_IRG) -o $@ -a otawa_target -d "return 0;" -f -t $< -e otawa_target.nmp

otawa_used_regs.h: used_regs.tpl $(PPC2_IRG) otawa_used_regs.nmp
	@GLISS2_ATTR_PATH@ $(PPC2_IRG) -o $@ -a otawa_used_regs -d "tmp_var[0] = END_REG; return 0;" -f -t $< -e otawa_used_regs.nmp

otawa_pred.h: pred.tpl $(PPC2_IRG) otawa_pred.nmp
	@GLISS2_ATTR_PATH@ $(PPC2_IRG) -o $@ -a otawa_pred -d "return -1;" -f -t $< -e otawa_pred.nmp

otawa_sem.h: otawa_sem.tpl $(PPC2_IRG) otawa_sem.nmp
	@GLISS2_ATTR_PATH@ $(PPC2_IRG) -o $@ -p -a otawa_sem -d "" -f -t $< -e otawa_sem.nmp

ppc.lo: otawa_kind.h otawa_target.h otawa_used_regs.h otawa_pred.h otawa_sem.h

