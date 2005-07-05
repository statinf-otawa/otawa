SUBDIRS=src examples include bin test

EXTRA_DIST=Doxyfile

pkgdata_DATA=@AUTODOC@

install-data-local:
	cp -R autodoc $(pkgdatadir)

uninstall-local:
	rm -rf $(pkgdatadir)

autodoc:
	doxygen
