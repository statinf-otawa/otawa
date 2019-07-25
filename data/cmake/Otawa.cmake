

# look for gliss-attr command
function(ENSURE_GLISS_ATTR)
    if(NOT GLISS_ATTR)
        find_program(GLISS_ATTR "gliss-attr")
        if(GLISS_ATTR)
            message(STATUS "found gliss-attr at ${GLISS_ATTR}")
           else()
               message(FATAL_ERROR "command gliss-attr not found")
        endif()
    endif()
endfunction()


# Build a ${KEY}.h file using gliss-attr from a template ${KEY}.tpl using ${NMP} file.
# Default value is ${DEF}.
function(MAKE_GLISS_FUNCTION _VAR _KEY _NMP _IRG _DEF) 
    ENSURE_GLISS_ATTR()

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


# Build a ${KEY}.h file using gliss-attr from a template ${KEY}.tpl using ${NMP} file.
# Default value is ${DEF}.
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
