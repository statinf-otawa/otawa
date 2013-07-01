add_custom_command(
	OUTPUT manual.html
	DEPENDS manual.thot
	COMMAND python
	ARGS  ${THOT_PATH} -t html manual.thot -o manual.html -DHTML_ONE_FILE_PER=chapter)
add_custom_target(all ALL DEPENDS manual.html)	
install(DIRECTORY "." DESTINATION share/Otawa/doc/manual
	PATTERN "*.thot" EXCLUDE
	PATTERN "*Make*" EXCLUDE
	PATTERN "*cmake*" EXCLUDE)
