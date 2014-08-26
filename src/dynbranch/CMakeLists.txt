set(CMAKE_INSTALL_RPATH "\\$ORIGIN/../../..")
set(CMAKE_MACOSX_RPATH true)

set(branch_SOURCES
#    dynbranch_DynamicBranching.cpp
#    dynbranch_GlobalAnalysisFeature.cpp
#    dynbranch_GlobalAnalysisProblem.cpp
    dynbranch_hook.cpp
    dynbranch_MemType.cpp
    dynbranch_PotentialValue.cpp
    dynbranch_State.cpp
)

include_directories(".")
include_directories("../../include")

add_library(dynbranch SHARED ${branch_SOURCES})
set_property(TARGET dynbranch PROPERTY PREFIX "")
set(SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}")
target_link_libraries(dynbranch otawa)
install(TARGETS dynbranch DESTINATION "${PROCDIR}/otawa")
