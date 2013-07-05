set(CMAKE_INSTALL_RPATH "\\$ORIGIN/../../..")

set(cfgio_la_SOURCES
	"cfgio_Output.cpp")

include_directories("../../include")

add_library(cfgio SHARED ${cfgio_la_SOURCES})
set_property(TARGET cfgio PROPERTY PREFIX "")
set(SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}")
install(TARGETS cfgio DESTINATION ${LIBDIR}/otawa/proc/otawa)
target_link_libraries(cfgio otawa)
