set(CMAKE_INSTALL_RPATH "${ORIGIN}/../../")
set(CMAKE_MACOSX_RPATH true)
set(CMAKE_CXX_FLAGS "-Wall")

add_library(icat3 SHARED
	icat3_LBlockBuilder.cpp
	icat3_hook.cpp)
set_property(TARGET icat3 PROPERTY PREFIX "")
target_link_libraries(icat3 ${LIBELM})
target_link_libraries(icat3 otawa)

install(TARGETS icat3 DESTINATION "${PLUGDIR}/otawa")
install(FILES icat3.eld DESTINATION "${PLUGDIR}/otawa")
