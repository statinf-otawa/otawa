set(CMAKE_INSTALL_RPATH "${ORIGIN}")
set(CMAKE_MACOSX_RPATH true)
set(CMAKE_CXX_FLAGS "-Wall" )

set(liboslice_la_SOURCES
	"oslice_DotDisplayer.cpp"
	"oslice_hook.cpp"
	"oslice_identifiers.cpp"
	"proc_CondBranchCollector.cpp"
	"proc_DummySlicer.cpp"
	"proc_InstCollector.cpp"
	"proc_LivenessChecker.cpp"
	"proc_Slicer.cpp"
)

include_directories(".")

add_library(oslice SHARED ${liboslice_la_SOURCES})
install(TARGETS oslice DESTINATION ${LIBDIR})

target_link_libraries(oslice otawa)
target_link_libraries(oslice ${LIBELM})

install(FILES oslice.eld DESTINATION "${LIBDIR}/otawa/proc/otawa")
