find_program(THOT_PATH thot.py HINTS "${CMAKE_SOURCE_DIR}/../thot")

if(THOT_PATH)
	message(STATUS "thot.py found at ${THOT_PATH}")
	add_subdirectory("manual")
	add_subdirectory("dev")
else()
	message(STATUS "thot.py not found: cannot generate the OTAWA manual.")
endif()
