set(CMAKE_INSTALL_RPATH "${ORIGIN}")
set(CMAKE_MACOSX_RPATH true)
set(CMAKE_CXX_FLAGS "-Wall" )

set(liboslice_reg_la_SOURCES
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

add_library(oslice_reg SHARED ${liboslice_reg_la_SOURCES})
install(TARGETS oslice_reg DESTINATION ${LIBDIR})

target_link_libraries(oslice_reg otawa)
target_link_libraries(oslice_reg ${LIBELM})

install(FILES oslice_reg.eld DESTINATION "${LIBDIR}/otawa/proc/otawa")
