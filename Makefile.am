SUBDIRS=src examples include bin test

EXTRA_DIST=Doxyfile

pkgdata_DATA=$(shell if test $(USE_DOX) = yes; then echo autodoc; fi)

install-data-local:
	cp -R autodoc $(pkgdatadir)

uninstall-local:
	rm -rf $(pkgdatadir)

autodoc:
	doxygen
