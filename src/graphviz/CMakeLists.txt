set(CMAKE_INSTALL_RPATH "${ORIGIN};${ORIGIN}/../..")
set(CMAKE_MACOSX_RPATH true)
set(CMAKE_CXX_FLAGS "-Wall" )

set(SOURCES "graphviz.cpp")
add_library(graphviz SHARED ${SOURCES})
set_property(TARGET graphviz PROPERTY PREFIX "")
set(SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}")
target_link_libraries(graphviz ${LIBELM} otawa display)

install(TARGETS graphviz DESTINATION "${PLUGDIR}/otawa")
install(FILES graphviz.eld DESTINATION "${PLUGDIR}/otawa")
