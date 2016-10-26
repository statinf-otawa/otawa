set(CMAKE_INSTALL_RPATH "${ORIGIN}/../..;${ORIGIN}/../../otawa/proc/otawa;${ORIGIN}/../../otawa/otawa")
set(CMAKE_MACOSX_RPATH true)
set(CMAKE_CXX_FLAGS "-Wall" )

set(SOURCES
	"DotDisplayer.cpp"
	"oslice_hook.cpp"
	"oslice_identifiers.cpp"
	"CondBranchCollector.cpp"
	"DummySlicer.cpp"
	"InstructionCollector.cpp"
	"LivenessChecker.cpp"
	"Slicer.cpp"
)

include_directories(".")

add_library(oslice SHARED ${SOURCES})
set_property(TARGET oslice PROPERTY PREFIX "")
set(SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}")
target_link_libraries(oslice otawa ${LIBELM} display clp)

install(TARGETS oslice DESTINATION "${PLUGDIR}/otawa")
install(FILES oslice.eld DESTINATION "${PLUGDIR}/otawa")
