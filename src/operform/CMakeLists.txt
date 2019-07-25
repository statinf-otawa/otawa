set(CMAKE_INSTALL_RPATH "${ORIGIN}/../lib;${ORIGIN}/../lib/otawa/proc/otawa;${ORIGIN}/../lib/otawa/otawa")
set(CMAKE_CXX_FLAGS "-Wall" )

set(operform_SOURCES "operform.cpp")
add_executable(operform ${operform_SOURCES})

target_link_libraries(operform ${LIBELM})
target_link_libraries(operform otawa)
target_link_libraries(operform display)

install(TARGETS operform DESTINATION bin)
