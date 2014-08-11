set(CMAKE_INSTALL_RPATH "\\$ORIGIN/../../..")
set(CMAKE_MACOSX_RPATH true)

set(cat_SOURCES
    cat_CATBuilder.cpp
    cat_CATConstraintBuilder.cpp
    cat_CATDFA.cpp
    cat_CATNode.cpp
    cat_hook.cpp
)

include_directories(".")
include_directories("../../include")

add_library(cat SHARED ${cat_SOURCES})
set_property(TARGET cat PROPERTY PREFIX "")
set(SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}")
target_link_libraries(cat otawa)
install(TARGETS cat DESTINATION "${PROCDIR}/otawa")
