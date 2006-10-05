SUBDIRS=src examples include bin test data

EXTRA_DIST=Doxyfile

install-data-local: autodoc
	install -d $(pkgdatadir)
	cp -R autodoc $(pkgdatadir)

uninstall-local:
	rm -rf $(pkgdatadir)

autodoc:
	doxygen
