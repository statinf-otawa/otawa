SUBDIRS=src examples include

EXTRA_DIST=Doxyfile

pkgdata_DATA=$(shell if test $(USE_DOX) = no; then echo html; fi)

install-data-local:
	cp -R html $(pkgdatadir)

uninstall-local:
	rm -rf $(pkgdatadir)

html: Doxyfile
	doxygen
