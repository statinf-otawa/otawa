cmake_minimum_required(VERSION 2.6)
project("otawa")
set(PACKAGE "otawa")

#add_compile_options(-Werror -Wall)

# C++ flags
if(CMAKE_BUILD_TYPE MATCHES Release)
	add_definitions(-DNDEBUG)
endif()
add_compile_options(-Wall)
if(CMAKE_VERSION LESS "3.1")
	add_compile_options(--std=c++11)
else()
	set(CMAKE_CXX_STANDARD 11)
endif()

# get version
file(READ "VERSION" VERSION)
if(NOT APPLE)
	set(ORIGIN $ORIGIN)
else()
	set(ORIGIN @loader_path)
	set(CMAKE_MACOSX_RPATH true)
endif()

# installation level
set(INSTALL_TYPE "bin,lib,inc" CACHE STRING "Type of installation (a ',' separated combination of bin, lib, inc, doc).")
set(INSTALL_BIN ON)
set(INSTALL_LIB ON)
set(INSTALL_INC ON)
set(INSTALL_DOC OFF)
if(INSTALL_TYPE)
	if(INSTALL_TYPE MATCHES "all")
		set(INSTALL_DOC ON)	
	else()
		if(NOT INSTALL_TYPE MATCHES ".*bin.*")
			set(INSTALL_BIN OFF)
		endif()
		if(NOT INSTALL_TYPE MATCHES ".*lib.*")
			set(INSTALL_LIB OFF)
		endif()
		if(NOT INSTALL_TYPE MATCHES ".*inc.*")
			set(INSTALL_INC OFF)
		endif()
		if(INSTALL_TYPE MATCHES ".*doc.*")
			set(INSTALL_DOC ON)
		endif()
	endif()
endif()

# gcc based compiler lack of support by cmake (opposed to VS)
# so need to make some manual adjustment in this case
if (CMAKE_COMPILER_IS_GNUCXX AND (CMAKE_BUILD_TYPE MATCHES Release))
  set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG -Wl,--kill-at -Wl,--add-stdcall-alias")
  set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wl,--kill-at -Wl,--add-stdcall-alias")
endif (CMAKE_COMPILER_IS_GNUCXX AND (CMAKE_BUILD_TYPE MATCHES Release))

include_directories(include)


# install directories
if(MINGW_LINUX OR WIN32)
	set(LIBDIR "bin")
else()
	set(LIBDIR "lib")
endif()
set(PLUGDIR      "${CMAKE_INSTALL_PREFIX}/lib/otawa")
set(PROCDIR      "${PLUGDIR}")
set(LOADERDIR    "${PLUGDIR}/loader")
set(ILPDIR       "${PLUGDIR}/ilp")


# adding Flex and Bison support
# the version of flex included with msys is bugged,
# so those scripts had to be heavily modded
if(WIN32)
include(CMakeUserUseBison.cmake RESULT_VARIABLE BISON)
include(CMakeUserUseFlex.cmake RESULT_VARIABLE FLEX)
endif(WIN32)

add_definitions(-DILP_PATHS=\"${PROJECT_BINARY_DIR}/lib/otawa/ilp\")
add_definitions(-DLOADER_PATHS=\"${PROJECT_BINARY_DIR}/lib/otawa/loader:${CMAKE_INSTALL_PREFIX}/lib/otawa/loader\")
add_definitions(-DSIMULATOR_PATHS=\"${PROJECT_BINARY_DIR}/lib/otawa/sim\")
add_definitions(-DPROC_PATHS=\"${PROJECT_BINARY_DIR}/lib/otawa/proc\")
add_definitions(-DHAS_RELOCATION)
add_definitions(-DOTAWA_CMAKE)
add_definitions(-g3)

# concurrency support
if(OTAWA_CONC)
	message(STATUS "concurrency support enabled!")
else()
	message(STATUS "concurrency support disabled!")
endif()


# looking for version
find_program(HG_PATH hg)
if(HG_PATH)
    execute_process(
        COMMAND "${HG_PATH}" identify --num -r .
        OUTPUT_VARIABLE OTAWA_VERSION
        ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()
if(OTAWA_TAG)
    set(OTAWA_VERSION "${OTAWA_VERSION}-${OTAWA_TAG}")
endif()
message(STATUS "OTAWA version: ${OTAWA_VERSION}")


# compute the date
if(WIN32)
	execute_process(COMMAND "cmd" "/C date /T" OUTPUT_VARIABLE RESULT)
elseif(UNIX OR MINGW_LINUX)
	execute_process(COMMAND "date" "+%d/%m/%Y" OUTPUT_VARIABLE RESULT OUTPUT_STRIP_TRAILING_WHITESPACE)
endif(WIN32)
if(WIN32 AND NOT MINGW_LINUX)
    string(REGEX REPLACE "([0-9][0-9])/([0-9][0-9])/([0-9][0-9][0-9][0-9]).*" "\\1\\2\\3" RESULT ${RESULT})
endif()
set(OTAWA_DATE "${RESULT}")
message(STATUS "OTAWA date: ${OTAWA_DATE}")



# looking for elm
set(ELM_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../elm")
if(MINGW_LINUX)
	set(LIBELM "${ELM_PATH}/src/libelm.dll")
elseif(WIN32)
	find_library(LIBELM elm PATHS "${ELM_PATH}/src")
else()
	find_library(LIBELM elm NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH HINTS "${CMAKE_CURRENT_SOURCE_DIR}/../elm/src" "${CMAKE_INSTALL_PREFIX}/bin")
endif()
if(NOT LIBELM)
	message(FATAL_ERROR "libelm is mandatory!")
else()
	message(STATUS "libelm found at ${LIBELM}")
	get_filename_component(dir "${LIBELM}" DIRECTORY)
	include_directories("${dir}/../include")
endif()


# looking for gel++
if(MINGW_LINUX)
	set(LIBGEL "${otawa_SOURCE_DIR}/../gel/src/libgel++.dll")
elseif(WIN32)
	find_library(LIBGEL gel++ PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../gel++/src)
else()
	find_library(LIBGEL gel++ NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH HINTS "${CMAKE_CURRENT_SOURCE_DIR}/../gel++/src"  "${CMAKE_INSTALL_PREFIX}/bin")
endif()
if(NOT LIBGEL)
	message(FATAL_ERROR "libgel++ is mandatory!")
else()
	message(STATUS "libgel++ found at ${LIBGEL}")
	get_filename_component(dir "${LIBGEL}" DIRECTORY)
	include_directories("${dir}/../include")
endif()


# look for graphviz
find_program(DOT "dot" HINTS ${GRAPHVIZ_PATH}/bin)
if(DOT)
	message(STATUS "GraphViz dot found at ${DOT}")
else()
	message(STATUS "GraphViz dot not found")
endif()

# look for python
find_package(PythonInterp)
find_package(PythonLibs)

# look for pygtk
if(PYTHONINTERP_FOUND)
	execute_process(COMMAND ${PYTHON_EXECUTABLE} -c "import pygtk" RESULT_VARIABLE res)
	if("${res}" EQUAL "0")
		set(PYGTK ON)
		message(STATUS "pygtk found")
	else()
		set(PYGTK OFF)
		message(STATUS "pygtk not found")
	endif()
endif()


# enabled otawa-xdot?
if(DOT AND PYGTK)
	message(STATUS "otawa-xdot.py can be installed.")
	set(XDOT_ENABLED ON)
else()
	message(STATUS "otawa-xdot.py cannot be installed.")
	set(XDOT_ENABLED OFF)
endif()


# look for system view
if(UNIX)
	find_program(XDG_OPEN "xdg-open")
	if(XDG_OPEN)
		set(SYSTEM_VIEW_ENABLED ON)
		set(SYSTEM_VIEW ${XDG_OPEN})
	else()
		set(SYSTEM_VIEW_ENABLED OFF)
	endif()
else()
	set(SYSTEM_VIEW_ENABLED OFF)
endif()
if(SYSTEM_VIEW_ENABLED)
	message(STATUS "system view tool found (${SYSTEM_VIEW})")
else()
	message(STATUS "no system view tool found")
endif()

# add subdirectories
add_subdirectory(src)
add_subdirectory(doc)
add_subdirectory(include)
add_subdirectory(bin)


# share installation
add_subdirectory(data)

# testing
if(OTAWA_TEST)
	enable_testing()
	add_subdirectory(test)
endif()


# build configuration file
configure_file (
  "${PROJECT_SOURCE_DIR}/config.in"
  "${PROJECT_SOURCE_DIR}/config.h"
)


# handle autodoc
find_program(DOXYGEN doxygen)
if(DOXYGEN)
	add_custom_target("autodoc"
		COMMAND "${DOXYGEN}"
		COMMENT "Building automatic documentation"
		SOURCES "Doxyfile" "${SOURCES}"
	)
	if(INSTALL_DOC)
		message(STATUS "install doc: ${INSTALL_DOC}")
		add_custom_target("install-autodoc" ALL DEPENDS "autodoc")
		install(DIRECTORY autodoc DESTINATION "${CMAKE_INSTALL_PREFIX}/share/Otawa/")
	endif()
endif()

