set(CMAKE_INSTALL_RPATH "\\$ORIGIN/../../..")
set(CMAKE_MACOSX_RPATH true)

set(cfgio_la_SOURCES
    "cfgio_Input.cpp"
	"cfgio_Output.cpp")

include_directories("../../include")

add_library(cfgio SHARED ${cfgio_la_SOURCES})
set_property(TARGET cfgio PROPERTY PREFIX "")
set(SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}")
target_link_libraries(cfgio otawa)

install(TARGETS cfgio DESTINATION "${PROCDIR}/otawa")
