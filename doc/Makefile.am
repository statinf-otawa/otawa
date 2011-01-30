
EXTRA_DIST=\
	manual.thot \
	manual-appli.thot \
	manual-install.thot \
	manual-oipet.thot \
	manual-cfg.thot \
	manual-props.thot \
	manual-procs.thot \
	manual-elm.thot \
	manual-dfa.thot \
	manual-ai.thot \
	manual-refs.thot

if HAS_THOT

install-data-local: manual.html
	install -d $(pkgdatadir)/doc
	cp -R manual*.html manual-imports $(pkgdatadir)/doc

uninstall-local:
	rm -rf $(pkgdatadir)/doc

manual.html: manual.thot *.thot
	@THOT_PATH@ -t html $< -o $@ -DHTML_ONE_FILE_PER=chapter

clean-local:
	rm -rf manual*.html manual-imports

endif
