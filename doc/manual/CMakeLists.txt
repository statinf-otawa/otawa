set(MANUALS
	manual-ai.thot
	manual-appli.thot
	manual-cfg.thot
	manual-dfa.thot
	manual-elm.thot
	manual-features.thot
	manual-formats.thot
	manual-gloss.thot
	manual-graph.thot
	manual-install.thot
	manual-procs.thot
	manual-props.thot
	manual-refs.thot
	manual.thot
	manual-wcet.thot)

message(STATUS "MANUALS=${MANUALS}")

add_custom_command(
	OUTPUT manual.html
	DEPENDS ${MANUALS}
	COMMAND python
	ARGS  ${THOT_PATH} -t html manual.thot -o manual.html -DHTML_ONE_FILE_PER=chapter)
add_custom_target(all ALL DEPENDS manual.html)	
install(DIRECTORY "." DESTINATION share/Otawa/doc/manual
	PATTERN "*.thot" EXCLUDE
	PATTERN "*Make*" EXCLUDE
	PATTERN "*cmake*" EXCLUDE)
