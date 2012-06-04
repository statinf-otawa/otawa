set(CMAKE_INSTALL_RPATH "\\$ORIGIN/../lib")

add_executable(odec "odec.cpp")
target_link_libraries(odec otawa)
install(TARGETS odec DESTINATION bin)
