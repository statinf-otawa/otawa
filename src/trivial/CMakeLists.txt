set(CMAKE_INSTALL_RPATH "${ORIGIN}/../../")
set(CMAKE_MACOSX_RPATH true)
set(CMAKE_CXX_FLAGS "-Wall")

add_library(trivial SHARED
	trivial.cpp)
set_property(TARGET trivial PROPERTY PREFIX "")
target_link_libraries(trivial ${LIBELM})
target_link_libraries(trivial otawa)

install(TARGETS trivial DESTINATION "${PLUGDIR}/otawa")
install(FILES trivial.eld DESTINATION "${PLUGDIR}/otawa")
