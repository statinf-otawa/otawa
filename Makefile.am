SUBDIRS=src examples include bin test data

EXTRA_DIST=Doxyfile

install-data-local: @AUTODOC@
	if test "@AUTODOC@" != ""; then \
		test -z "$(pkgdatadir)" || mkdir -p "$(pkgdatadir)"; \
		cp -R autodoc $(pkgdatadir); \
	fi

uninstall-local:
	rm -rf $(pkgdatadir)

autodoc:
	doxygen
