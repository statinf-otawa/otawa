set(CMAKE_INSTALL_RPATH "${ORIGIN};${ORIGIN}/../..")
set(CMAKE_MACOSX_RPATH true)
set(CMAKE_CXX_FLAGS "-Wall" )



execute_process(COMMAND "${OTAWA_CONFIG}" --cflags ${MODULES} OUTPUT_VARIABLE OTAWA_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
set(CMAKE_CXX_FLAGS "${OTAWA_CFLAGS}")
set(SOURCES "clp_analysis.cpp" "clp_symbolic_expr.cpp" "clp_export.cpp" "clp_import.cpp")

add_library(clp SHARED ${SOURCES})
set_property(TARGET clp PROPERTY PREFIX "")
set(SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}")
target_link_libraries(clp ${LIBELM} otawa)

install(TARGETS clp DESTINATION "${PLUGDIR}/otawa")
install(FILES clp.eld DESTINATION "${PLUGDIR}/otawa")
