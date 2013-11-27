set(CMAKE_INSTALL_RPATH "../lib/\\$ORIGIN")

# compilation configuration
find_path(CONFIG otawa-config DOC "path to otawa-config")
if(NOT CONFIG)
        message(FATAL_ERROR "ERROR: otawa-config is required !")
endif()

execute_process(COMMAND otawa-config --prefix OUTPUT_VARIABLE CMAKE_INSTALL_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND otawa-config --cflags OUTPUT_VARIABLE OTAWA_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND otawa-config --libs OUTPUT_VARIABLE OTAWA_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}")

#include header files   
include_directories("../../include")
include_directories(".")
#include_directories("..")
#some .h files need elm files 
include_directories("../../../elm/include")
include_directories("../../../gel/include")


set(newexegraphs_SOURCES
	exegraph2_ExecutionGraph.cpp
	exegraph2_EGBBTime.cpp
	exegraph2_EGProc.cpp
	exegraph2_EGBlockSeq.cpp
	exegraph2_EGBuilder.cpp
	exegraph2_EGSolver.cpp
	exegraph2_EGDisplay.cpp
	exegraph2_EGScenario.cpp
	test_exegraph2.cpp
	)
	
add_executable(newexegraphs ${newexegraphs_SOURCES})

target_link_libraries(newexegraphs ${LIBELM})
target_link_libraries(newexegraphs ${LIBOTAWA})

#install(TARGETS owcet DESTINATION bin)

if(APPLE)
	set_target_properties(newexegraphs PROPERTIES INSTALL_RPATH "../lib/@executable_path")
endif()
