
#include header files
include_directories("../include")
include_directories(".")
include_directories("..")
include_directories("../../elm/include")
include_directories("../../gel/include")

add_executable(test_MemorySet "test_MemorySet.cpp")
target_link_libraries(test_MemorySet otawa ${LIBELM})

add_subdirectory(ai)
#add_subdirectory(clp)
add_subdirectory(props)
add_subdirectory(reg)
add_subdirectory(cfg)
add_subdirectory(dom)
add_subdirectory(lexicon)
#add_subdirectory(steps)
add_subdirectory(sem)
