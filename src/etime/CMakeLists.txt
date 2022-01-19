set(CMAKE_INSTALL_RPATH "${ORIGIN}/../..:${ORIGIN}")
set(CMAKE_MACOSX_RPATH true)
set(CMAKE_CXX_FLAGS "-Wall" )

set(PLUGIN etime)
set(SOURCES
	"AbstractTimeBuilder.cpp"
    "StandardXGraphBuilder.cpp"
    "StandardXGraphSolver.cpp"
    "StandardILPGenerator.cpp"
    "EdgeTimeBuilder.cpp"
	"hook.cpp"
    "StandardEventBuilder.cpp"
    "TimeUnitTimer.cpp"
    "StepGraphBuilder.cpp"
)

include_directories(".")
include_directories("../../include")

add_library(${PLUGIN} SHARED ${SOURCES})
set_property(TARGET ${PLUGIN} PROPERTY PREFIX "")
set(SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}")
target_link_libraries(${PLUGIN} otawa branch)

install(TARGETS ${PLUGIN} DESTINATION "${PROCDIR}/otawa")
install(FILES etime.eld DESTINATION "${PROCDIR}/otawa")
