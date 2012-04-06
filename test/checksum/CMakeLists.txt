
add_executable(test_checksum "test_checksum.cpp")

target_link_libraries(test_checksum otawa)
target_link_libraries(test_checksum ${LIBELM})
