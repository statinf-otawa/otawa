cmake_minimum_required(VERSION 2.6)

project("otawa")
set(PACKAGE "otawa")

# debugging activated
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -g3")

set(VERSION "0.9.1")

# gcc based compiler lack of support by cmake (opposed to VS)
# so need to make some manual adjustment in this case
if (CMAKE_COMPILER_IS_GNUCXX AND (CMAKE_BUILD_TYPE MATCHES Release))
  set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG -Wl,--kill-at -Wl,--add-stdcall-alias")
  set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wl,--kill-at -Wl,--add-stdcall-alias")
endif (CMAKE_COMPILER_IS_GNUCXX AND (CMAKE_BUILD_TYPE MATCHES Release))

#set(HAS_RELOCATION on)

# build configuration file
configure_file (
  "${PROJECT_SOURCE_DIR}/config.in"
  "${PROJECT_SOURCE_DIR}/config.h"
)

if(MINGW_LINUX OR WIN32)
	set(LIBDIR "bin")
else()
	set(LIBDIR "lib")
endif()

#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_DIR}/lib)

# adding Flex and Bison support
# the version of flex included with msys is bugged, 
# so those scripts had to be heavily modded
if(WIN32)
include(CMakeUserUseBison.cmake RESULT_VARIABLE BISON)
include(CMakeUserUseFlex.cmake RESULT_VARIABLE FLEX)
endif(WIN32)

add_definitions(-DILP_PATHS=\"${PROJECT_BINARY_DIR}/lib/otawa/ilp\")
add_definitions(-DLOADER_PATHS=\"${PROJECT_BINARY_DIR}/lib/otawa/loader\")
add_definitions(-DSIMULATOR_PATHS=\"${PROJECT_BINARY_DIR}/lib/otawa/sim\")
add_definitions(-DPROC_PATHS=\"${PROJECT_BINARY_DIR}/lib/otawa/proc\")
add_definitions(-DHAS_RELOCATION)
add_definitions(-DOTAWA_CMAKE)

# looking for ARM loader
set(ARM_PATH "${CMAKE_SOURCE_DIR}/../arm" CACHE PATH "path to GLISS ARM path")
if(EXISTS "${ARM_PATH}")
	find_library(ARM_LIB emul PATHS "${ARM_PATH}/lib" NO_DEFAULT_PATH)
endif()
if(ARM_LIB)
	message(STATUS "ARM library found at ${ARM_LIB}")
else()
	message(STATUS "ARM library not found")
endif()

# ==== for Date ====
if(WIN32)
	execute_process(COMMAND "cmd" "/C date /T" OUTPUT_VARIABLE RESULT)
#	message(STATUS ${RESULT})
elseif(UNIX OR MINGW_LINUX)
	execute_process(COMMAND "date" "+%d/%m/%Y" OUTPUT_VARIABLE RESULT OUTPUT_STRIP_TRAILING_WHITESPACE)
endif(WIN32)
if(WIN32 AND NOT MINGW_LINUX)
string(REGEX REPLACE "([0-9][0-9])/([0-9][0-9])/([0-9][0-9][0-9][0-9]).*" "\\1\\2\\3" RESULT ${RESULT})
endif()
add_definitions(-DDAYDATE=\"${RESULT}\")
#	message(STATUS ${RESULT})


###########################
#adding Doxygen support
if(UNIX OR APPLE)
	#if using a unix system
	message(STATUS "Using Unix system, not MacOS")
	find_program(DOXYGEN doxygen
		/usr/local/bin
		/usr/bin
		)
elseif(WIN32 OR MINGW)
	#not tested
	message(STATUS "Using Win32 system")
	find_program(DOXYGEN doxygen
		PATH
		)
elseif(APPLE)
	#not tested either, and shall not test
	message(WARNING "MacOS not supported")
else(UNIX AND NOT APPLE)
	#in case of some more exotic OS
	message(WARNING "OS not supported")
endif()
	
#once Doxygen path has been searched for
#generation of documentation IF Doxygen has been found
#tried testing for DEFINED DOXYGEN_DIR, doesn't work
if(DOXYGEN AND WIN32)
	message(STATUS "Doxygen found, generation of documentation")
	execute_process(COMMAND "cmd" "/C doxygen Doxyfile")
elseif(DOXYGEN AND UNIX) #not tested
	message(STATUS "Doxygen found, generation of documentation")
	execute_process(COMMAND "bash" "doxygen Doxyfile")
else(DOXYGEN AND WIN32)
	message(WARNING "Doxygen not found, cannot generate documentation")
endif(DOXYGEN AND WIN32)

###########################


# linking to elm and gel libraries
# linking is hardcoded, but libraries will be searched for
# in all directories in PATH
if(MINGW_LINUX)
	set(LIBELM ${otawa_SOURCE_DIR}/../site/bin/libelm.dll)
	set(LIBGEL ${otawa_SOURCE_DIR}/../site/bin/libgel.dll)
	set(LIBGEL_DWARF ${otawa_SOURCE_DIR}/../site/bin/libgel_dwarf.dll)
	message(STATUS "DEBUG: MINGW_LINUX: ${LIBELM}")
elseif(WIN32)
	find_library(LIBELM elm PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../elm/src)
	find_library(LIBGEL gel PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../gel/src)
	find_library(LIBGEL_DWARF gel_dwarf PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../gel/src)
else()
	find_library(LIBELM elm PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../elm/src)
	find_library(LIBGEL gel PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../gel/src)
	find_library(LIBGEL_DWARF gel_dwarf PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../gel/src)
endif()


# look for lp_solve5
if(MINGW_LINUX OR MINGW_WIN)
	set(LP_SOLVE5_INCLUDE "${CMAKE_SOURCE_DIR}/../libotawa/include" CACHE PATH "path to lp_solve5 include")
	set(LP_SOLVE5 "${CMAKE_SOURCE_DIR}/../libotawa/bin/lpsolve55.dll" CACHE FILEPATH "path to lp_solve5 library")
elseif(UNIX OR APPLE)
	set(LP_SOLVE5_PATH ${CMAKE_SOURCE_DIR}/../lp_solve5)
	find_library(LP_SOLVE5 lpsolve55 PATHS ${LP_SOLVE5_PATH}/lpsolve55 NO_DEFAULT_PATH)
elseif(WIN32)
	set(LP_SOLVE5_PATH "${CMAKE_SOURCE_DIR}/../lp_solve5")
	find_library(LP_SOLVE5 lpsolve55 PATHS ${LP_SOLVE5_PATH})
endif()
if(LP_SOLVE5)
	set(LP_SOLVE5_INCLUDE ${LP_SOLVE5_PATH})
endif()


# add subdirectories
add_subdirectory(src)
add_subdirectory(doc)
add_subdirectory(include)


# share installation
set(EXCLUDED_FILES PATTERN "CVS" EXCLUDE PATTERN "Makefile*" EXCLUDE PATTERN ".cvsignore" EXCLUDE)
install(DIRECTORY data/caches DESTINATION share/Otawa/ ${EXCLUDED_FILES})
install(DIRECTORY data/makefiles DESTINATION share/Otawa/ ${EXCLUDED_FILES})
install(DIRECTORY data/procs DESTINATION share/Otawa/ ${EXCLUDED_FILES})
install(DIRECTORY data/scripts DESTINATION share/Otawa/ ${EXCLUDED_FILES})


# testing
if(OTAWA_TEST)
	add_subdirectory(test)
endif()



# handle autodoc
if(DOXYGEN)
add_custom_target("make-autodoc" ALL DEPENDS "autodoc")
add_custom_command(
	OUTPUT "autodoc"
	DEPENDS "Doxyfile"
	COMMAND "${DOXYGEN}"	
)
install(DIRECTORY autodoc DESTINATION "${CMAKE_INSTALL_PREFIX}/share/Otawa/")
endif()
