
add_executable(test_checksum "test_checksum.cpp")

target_link_libraries(test_checksum libotawa)
target_link_libraries(test_checksum ${LIBELM})
