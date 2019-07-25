CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Just configure following variables
set(EXEC "")               # executable name
set(MODULES "")            # special plugins of OTAWA
set(SOURCES )              # sources

# script
project(${EXEC})

# look for OTAWA
if(NOT OTAWA_CONFIG)
	find_program(OTAWA_CONFIG otawa-config DOC "path to otawa-config")
	if(NOT OTAWA_CONFIG)
		message(FATAL_ERROR "ERROR: otawa-config is required !")
	endif()
endif()
message(STATUS "otawa-config at ${OTAWA_CONFIG}")
execute_process(COMMAND "${OTAWA_CONFIG}" --cflags ${MODULES} OUTPUT_VARIABLE OTAWA_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND "${OTAWA_CONFIG}" --libs ${MODULES} --rpath OUTPUT_VARIABLE OTAWA_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND "${OTAWA_CONFIG}" --prefix OUTPUT_VARIABLE OTAWA_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)

# plugin definition
include_directories("${CMAKE_SOURCE_DIR}" "." )
add_executable(${EXEC} ${SOURCES})
set_property(TARGET ${EXEC} PROPERTY COMPILE_FLAGS "${OTAWA_CFLAGS}")
target_link_libraries(${EXEC} "${OTAWA_LDFLAGS}")
