SUBDIRS=src include

EXTRA_DIST=Doxyfile

pkgdata_DATA=html

install-data-local:
	cp -R html $(pkgdatadir)

uninstall-local:
	rm -rf $(pkgdatadir)

html: Doxyfile
	doxygen

