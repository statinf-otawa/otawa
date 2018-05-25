
add_executable(test_props "test_props.cpp")
target_link_libraries(test_props otawa ${LIBELM})

add_test(test_props test_props)
