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

message(STATUS "MANUALS=${MANUALS}")

add_custom_target(manual.html
	ALL
	SOURCES ${MANUALS}
	COMMAND python ${THOT_PATH} -t html manual.thot -o manual.html -DHTML_ONE_FILE_PER=chapter
	COMMENT "Making manual."
)

install(DIRECTORY "." DESTINATION share/Otawa/doc/manual
	PATTERN "*.thot" EXCLUDE
	PATTERN "*Make*" EXCLUDE
	PATTERN "*cmake*" EXCLUDE)
