

add_executable(test_ct test_ct.cpp)

target_link_libraries(test_ct libotawa)
target_link_libraries(test_ct ${LIBELM})
