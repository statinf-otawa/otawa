


# generating c files from Bison and Flex files
if(WIN32)
	# Compiling Flex and Bison files to C files, and adding them to source files.
	set(BISON_PREFIX_OUTPUTS TRUE CACHE BOOL "activating Bison")
	set(FLEX_PREFIX_OUTPUTS TRUE CACHE BOOL "activating Flex")
elseif(UNIX)
	find_package(BISON 0)
	#find_package(FLEX 0)
	include("${CMAKE_SOURCE_DIR}/CMakeUserUseFlex.cmake")
	
endif(WIN32)


#include header files	
include_directories("../include")
include_directories(".")
include_directories("..")
include_directories("../../elm/include")
include_directories("../../gel/include")


# subdirectories
add_subdirectory(prog)
add_subdirectory(ast)
add_subdirectory(oloader)
#add_subdirectory(ets)
add_subdirectory(ocpp)
add_subdirectory(odisplay)


# arm check
if(EXISTS "${CMAKE_SOURCE_DIR}/../armv5t/arm.irg")
	add_subdirectory(arm2)
elseif(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/../../arm/lib/libemul.a")
	add_subdirectory(arm)
endif()


# ILP Plugins
if(HAS_LP_SOLVE)
add_subdirectory(lp_solve)
endif(HAS_LP_SOLVE)

if(LP_SOLVE5)
add_subdirectory(lp_solve5)
endif(LP_SOLVE5)


# Loader selection
if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/../../ppc2")
add_subdirectory(ppc2)
#message(STATUS "${CMAKE_CURRENT_BINARY_DIR}/../../ppc2")
elseif()
add_subdirectory(ppc)
endif()


# Tools
add_subdirectory(dumpcfg)
add_subdirectory(mkff)
add_subdirectory(odfa)
add_subdirectory(ostat)
add_subdirectory(opcg)
add_subdirectory(odisasm)
add_subdirectory(owcet)


# Dependent tools
add_subdirectory(bpred)
if(LP_SOLVE5)
add_subdirectory(oipet)
endif()
