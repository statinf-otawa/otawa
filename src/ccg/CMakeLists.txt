set(CMAKE_INSTALL_RPATH "\\$ORIGIN/../../..")
set(CMAKE_MACOSX_RPATH true)

set(ccg_SOURCES
    ccg_Builder.cpp
    ccg_ConstraintBuilder.cpp
    ccg_DFA.cpp
    ccg_Edge.cpp
    ccg_hook.cpp
    ccg_Node.cpp
)

include_directories(".")
include_directories("../../include")

add_library(ccg SHARED ${ccg_SOURCES})
set_property(TARGET ccg PROPERTY PREFIX "")
set(SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}")
target_link_libraries(ccg otawa)
install(TARGETS ccg DESTINATION "${PROCDIR}/otawa")
