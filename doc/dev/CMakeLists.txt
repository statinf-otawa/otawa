add_custom_command(
	OUTPUT dev.html
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS dev.thot dev-loader.thot dev-script.thot dev-ana.thot
	COMMAND python 
	"${THOT_PATH}" -t html dev.thot -o dev.html -DHTML_ONE_FILE_PER=chapter
	COMMENT "Making developper guide.")
add_custom_target(all-dev ALL DEPENDS dev.html)	
install(DIRECTORY "." DESTINATION share/Otawa/doc/dev
	PATTERN "*.thot" EXCLUDE
	PATTERN "*Make*" EXCLUDE
	PATTERN "*cmake*" EXCLUDE)
