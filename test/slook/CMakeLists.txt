
add_executable(slook "slook.cpp")
target_link_libraries(slook otawa)
target_link_libraries(slook ${LIBELM})
