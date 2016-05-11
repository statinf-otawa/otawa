set(CMAKE_INSTALL_RPATH "${ORIGIN}")
set(CMAKE_MACOSX_RPATH true)
set(CMAKE_CXX_FLAGS "-Wall" )

add_library(opy SHARED "opy.cpp")
install(TARGETS opy DESTINATION ${LIBDIR})
target_link_libraries(opy otawa ${LIBELM})
