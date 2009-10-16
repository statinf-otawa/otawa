SUBDIRS=src include bin test data doc

EXTRA_DIST=Doxyfile

install-data-local: autodoc ilp_default
	install -d $(pkgdatadir)
	cp -R autodoc $(pkgdatadir)

uninstall-local:
	rm -rf $(pkgdatadir)

autodoc:
	doxygen

# Default ILP plugin
ILP_PATH=$(libdir)/otawa/ilp
ilp_default:
if HAS_PLUGIN
if HAS_ILP
	test -e $(ILP_PATH)/default.la && rm -f $(ILP_PATH)/default.la; \
	ln -s $(ILP_PATH)/@ILP_DEFAULT@.la $(ILP_PATH)/default.la
endif
endif
