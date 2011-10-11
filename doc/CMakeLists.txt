find_program(THOT_PATH thot.py HINTS "${CMAKE_SOURCE_DIR}/../thot")

if(THOT_PATH)
	message(STATUS "thot.py found at ${THOT_PATH}")
	add_custom_command(
		OUTPUT manual.html
		DEPENDS manual.thot
		COMMAND "${THOT_PATH}"
		ARGS -t html manual.thot -o manual.html -DHTML_ONE_FILE_PER=chapter)
	add_custom_target(all ALL DEPENDS manual.html)	
	install(DIRECTORY "." DESTINATION share/Otawa/doc
		PATTERN "CVS" EXCLUDE
		PATTERN "*cvs*" EXCLUDE
		PATTERN "*Make*" EXCLUDE
		PATTERN "*.thot" EXCLUDE
		PATTERN "*.sxd" EXCLUDE
		PATTERN "*cmake*" EXCLUDE)
else()
	message(STATUS "thot.py not found: cannot generate the OTAWA manual.")
endif()

