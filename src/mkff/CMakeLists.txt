set(CMAKE_INSTALL_RPATH "\\$ORIGIN/../lib")


add_executable(mkff mkff.cpp)
target_link_libraries(mkff ${LIBELM})
target_link_libraries(mkff otawa)
install(TARGETS mkff DESTINATION bin)
