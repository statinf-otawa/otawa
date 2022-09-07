#
#	CMake module for OTAWA
#
#	This file is part of OTAWA
#	Copyright (c) 2022, IRIT UPS.
#
#	OTAWA is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 2 of the License, or
#	(at your option) any later version.
#
#	OTAWA is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with OTAWA; if not, write to the Free Software
#	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
#	02110-1301  USA
#

function(OTAWA_FIND)
	if(NOT OTAWA_CONFIG)
		find_program(OTAWA_CONFIG "otawa-config" DOC "path to otawa-config")
		if(NOT OTAWA_CONFIG)
			message(FATAL_ERROR "ERROR: otawa-config is required !")
		endif()
		set(OTAWA_CONFIG "${OTAWA_CONFIG}" PARENT_SCOPE)
	endif()
	message(STATUS "otawa-config at ${OTAWA_CONFIG}")
endfunction()

function(OTAWA_PLUGIN _PLUGIN _NAMESPACE _SOURCES)

	# C++ management
	if(CMAKE_VERSION LESS "3.1")
		add_compile_options(--std=c++11)
		message(STATUS "C++11 set using cflags")
	else()
		set(CMAKE_CXX_STANDARD 11)
		message(STATUS "C++ set using CMAKE_CXX_STANDARD")
	endif()

	# get configuration
	OTAWA_FIND()
	execute_process(COMMAND "${OTAWA_CONFIG}" --prefix
		OUTPUT_VARIABLE OTAWA_PREFIX 	OUTPUT_STRIP_TRAILING_WHITESPACE)
	execute_process(COMMAND "${OTAWA_CONFIG}" --plugdir
		OUTPUT_VARIABLE OTAWA_PLUGDIR	OUTPUT_STRIP_TRAILING_WHITESPACE)
	execute_process(COMMAND "${OTAWA_CONFIG}" --make-plug "${_PLUGIN}" --cflags
		OUTPUT_VARIABLE OTAWA_CFLAGS 	OUTPUT_STRIP_TRAILING_WHITESPACE)
	execute_process(COMMAND "${OTAWA_CONFIG}" --make-plug "${_PLUGIN}" --libs -r
		OUTPUT_VARIABLE OTAWA_LDFLAGS 	OUTPUT_STRIP_TRAILING_WHITESPACE)
	message(STATUS "OTAWA_LDFLAGS=${OTAWA_LDFLAGS}")

	
	# export the configuration
	set(OTAWA_PREFIX "${OTAWA_PREFIX}" PARENT_SCOPE)
	set(OTAWA_PLUGDIR "${OTAWA_PLUGDIR}" PARENT_SCOPE)
	set(OTAWA_CFLAGS "${OTAWA_CFLAGS}" PARENT_SCOPE)
	set(OTAWA_LDFLAGS "${OTAWA_LDFLAGS}" PARENT_SCOPE)
	set(OTAWA_INC_DIR "${OTAWA_PREFIX}/include" PARENT_SCOPE)
	set(OTAWA_BIN_DIR "${OTAWA_PREFIX}/bin" PARENT_SCOPE)
	set(OTAWA_LIB_DIR "${OTAWA_PREFIX}/lib" PARENT_SCOPE)

	# plugin definition
	add_library				("${_PLUGIN}" MODULE ${_SOURCES})
	set_property			(TARGET "${_PLUGIN}" PROPERTY PREFIX "")
	set_property			(TARGET "${_PLUGIN}" PROPERTY COMPILE_FLAGS "${OTAWA_CFLAGS}")
	target_link_libraries	("${_PLUGIN}" "${OTAWA_LDFLAGS}")

	# installation
	install(TARGETS		"${_PLUGIN}"		LIBRARY DESTINATION "${OTAWA_PLUGDIR}/${_NAMESPACE}")
	install(FILES		"${_PLUGIN}.eld"	DESTINATION 		"${OTAWA_PLUGDIR}/${_NAMESPACE}")

	# autodoc support
	if(WITH_DOC)

		# look for Doxygen command
		find_program(DOXYGEN "doxygen")
		if(NOT DOXYGEN)
			message(FATAL_ERROR "cannot find doxygen")
		endif()
		
		if(NOT AUTODOC_INSTALL_DIR)
			set(AUTODOC_INSTALL_DIR "${OTAWA_PREFIX}/share/Otawa/autodoc-${PLUGIN}")
		endif()

		# any Doxyfile.in?
		if(EXISTS "Doxyfile.in")
			if(ELM_HOME)
				set(ELM_AUTODOC_PATH "${ELM_HOME}/share/Elm/autodoc")
			else()
				set(ELM_AUTODOC_PATH "${OTAWA_PREFIX}/share/Elm/autodoc")
			endif()
			set(OTAWA_AUTODOC_PATH "${OTAWA_PREFIX}/share/Otawa/autodoc")

			# prepare the Doxyfile
			set(TAGFILES "")
			if(EXISTS "${ELM_AUTODOC_PATH}/elm.tag")
				file(RELATIVE_PATH RELPATH "${AUTODOC_INSTALL_DIR}" "${ELM_AUTODOC_PATH}")
				set(TAGFILES "${TAGFILES} ${ELM_AUTODOC_PATH}/elm.tag=${RELPATH}")
			endif()
			if(EXISTS "${OTAWA_AUTODOC_PATH}/otawa.tag")
				file(RELATIVE_PATH RELPATH "${AUTODOC_INSTALL_DIR}" "${OTAWA_AUTODOC_PATH}")
				set(TAGFILES "${TAGFILES} ${OTAWA_AUTODOC_PATH}/otawa.tag=${RELPATH}")
			endif()
			configure_file(Doxyfile.in Doxyfile)
		endif()

		# set the targets
		add_custom_target(autodoc ALL COMMAND "${DOXYGEN}")
		install(DIRECTORY "autodoc" DESTINATION "${AUTODOC_INSTALL_DIR}/${_PLUGIN}-autodoc")
	endif()
endfunction()

function(OTAWA_INSTALL_INCLUDE _DIR)
	OTAWA_FIND()
	execute_process(COMMAND "${OTAWA_CONFIG}" --prefix
		OUTPUT_VARIABLE OTAWA_PREFIX 	OUTPUT_STRIP_TRAILING_WHITESPACE)
	install(DIRECTORY "${_DIR}" DESTINATION "${OTAWA_PREFIX}/include")
endfunction()


function(MAKE_GLISS_FUNCTION _VAR _KEY _NMP _IRG _DEF)
	set(_H "${CMAKE_CURRENT_BINARY_DIR}/${_KEY}.h")
	set(${_VAR} "${_H}" PARENT_SCOPE)
	set(_ARGN_NMP )
	foreach(_NMP ${ARGN})
		list(APPEND _ARGN_NMP -e ${_NMP})
	endforeach()
	add_custom_command(
		OUTPUT "${_H}"
		DEPENDS "${_KEY}.tpl" ${_NMP} "${_IRG}" ${ARGN}
		COMMAND "${GLISS_ATTR}"
		ARGS "${_IRG}" -o "${_H}" -a "${_KEY}" -f -t "${_KEY}.tpl" -d "${_DEF}" -e "${_NMP}" ${_ARGN_NMP}
     	VERBATIM
     )
 endfunction()

 
function(MAKE_GLISS_PROCEDURE _VAR _KEY _NMP _IRG _DEF) 
	set(_H "${CMAKE_CURRENT_BINARY_DIR}/${_KEY}.h")
	set(${_VAR} "${_H}" PARENT_SCOPE)
	set(_ARGN_NMP )
	foreach(_NMP ${ARGN})
		list(APPEND _ARGN_NMP -e ${_NMP})
	endforeach()
	add_custom_command(
		OUTPUT "${_H}"
		DEPENDS "${_KEY}.tpl" ${_NMP} "${_IRG}" ${ARGN}
		COMMAND "${GLISS_ATTR}"
		ARGS "${_IRG}" -o "${_H}" -a "${_KEY}" -p -t "${_KEY}.tpl" -d "${_DEF}" -e "${_NMP}" ${_ARGN_NMP}
     	VERBATIM
	)
endfunction()


function(OTAWA_ILP _PLUGIN _NAMESPACE _SOURCES)
        OTAWA_PLUGIN("${_PLUGIN}" "${_NAMESPACE}" "${_SOURCES}")
        file(WRITE ilp.eld
                "[elm-plugin]\n"
                "path=$ORIGIN/../${_NAMESPACE}/${_PLUGIN}\n"
        )
        install(FILES   ilp.eld
                DESTINATION "${OTAWA_PLUGDIR}/ilp"
                RENAME "default.eld")
        install(FILES   ilp.eld
                DESTINATION "${OTAWA_PLUGDIR}/ilp"
                RENAME "${PLUGIN}.eld")
endfunction()
