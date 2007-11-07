SUBDIRS=src examples include bin test data

EXTRA_DIST=Doxyfile

install-data-local: autodoc ilp_default
	install -d $(pkgdatadir)
	cp -R autodoc $(pkgdatadir)

uninstall-local:
	rm -rf $(pkgdatadir)

autodoc:
	doxygen
 	
ilp_default:
if HAS_PLUGIN
if HAS_ILP
	ln -s $(libdir)/otawa/ilp/@ILP_DEFAULT@.la $(libdir)/otawa/ilp/default.la
endif
endif
