set(CMAKE_INSTALL_RPATH "\\$ORIGIN/../../..")
set(CMAKE_MACOSX_RPATH true)

set(PLUGIN etime)
set(SOURCES
    "StandardEventBuilder.cpp"
    "EdgeTimeBuilder.cpp"
	"hook.cpp"
)

include_directories(".")
include_directories("../../include")

add_library(${PLUGIN} SHARED ${SOURCES})
set_property(TARGET ${PLUGIN} PROPERTY PREFIX "")
set(SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}")
target_link_libraries(${PLUGIN} otawa branch dcache)

install(TARGETS ${PLUGIN} DESTINATION "${PROCDIR}/otawa")
install(FILES etime.eld DESTINATION "${PROCDIR}/otawa")
