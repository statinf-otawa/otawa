set(CMAKE_INSTALL_RPATH "\\$ORIGIN/../../..")
set(CMAKE_MACOSX_RPATH true)

set(branch_SOURCES
    branch_BranchBuilder.cpp
    branch_BranchProblem.cpp
    branch_CondNumber.cpp
    branch_ConsBuilder.cpp
    branch_hook.cpp
)

include_directories(".")
include_directories("../../include")

add_library(branch SHARED ${branch_SOURCES})
set_property(TARGET branch PROPERTY PREFIX "")
set(SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}")
target_link_libraries(branch otawa)
install(TARGETS branch DESTINATION "${PROCDIR}/otawa")
