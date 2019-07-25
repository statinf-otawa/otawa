set(CMAKE_INSTALL_RPATH "${ORIGIN}/../..;${ORIGIN}")
set(CMAKE_MACOSX_RPATH true)

set(SOURCES
    dynbranch_DynamicBranching.cpp
    dynbranch_GlobalAnalysis.cpp
    dynbranch_GlobalAnalysisProblem.cpp
    dynbranch_hook.cpp
    dynbranch_MemType.cpp
    dynbranch_PotentialValue.cpp
    dynbranch_State.cpp
    FastStateWrapper.cpp
)

include_directories(".")

add_library(dynbranch SHARED ${SOURCES})
set_property(TARGET dynbranch PROPERTY PREFIX "")
set(SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}")
target_link_libraries(dynbranch clp)

install(TARGETS dynbranch DESTINATION "${PLUGDIR}/otawa")
install(FILES dynbranch.eld DESTINATION "${PLUGDIR}/otawa")
