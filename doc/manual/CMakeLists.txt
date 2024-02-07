set(MANUALS
	manual-ai.thot
	manual-appli.thot
	manual-cfg.thot
	manual-dfa.thot
	manual-elm.thot
	manual-formats.thot
	manual-gloss.thot
	manual-graph.thot
	manual-install.thot
	manual-prog.thot
	manual-props.thot
	manual-refs.thot
	manual.thot
	manual-wcet.thot)

add_custom_command(
	OUTPUT manual.html
	DEPENDS ${MANUALS}
	COMMAND python 
	"${THOT_PATH}" -t html manual.thot -o manual.html -DHTML_ONE_FILE_PER=chapter
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
	COMMENT "Making manual.")
add_custom_target(all-manual ALL DEPENDS manual.html)

install(DIRECTORY "." DESTINATION share/Otawa/doc/manual
	PATTERN "*.thot" EXCLUDE
	PATTERN "*Make*" EXCLUDE
	PATTERN "*cmake*" EXCLUDE)
