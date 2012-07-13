add_custom_command(
	OUTPUT dev.html
	DEPENDS dev.thot
	COMMAND "${THOT_PATH}"
	ARGS -t html dev.thot -o dev.html -DHTML_ONE_FILE_PER=chapter)
add_custom_target(all-dev ALL DEPENDS dev.html)	
install(DIRECTORY "." DESTINATION share/Otawa/doc
	PATTERN "*.thot" EXCLUDE
	PATTERN "*Make*" EXCLUDE
	PATTERN "*cmake*" EXCLUDE)
