


# generating c files from Bison and Flex files
if(WIN32)
	# Compiling Flex and Bison files to C files, and adding them to source files.
	set(BISON_PREFIX_OUTPUTS TRUE CACHE BOOL "activating Bison")
	set(FLEX_PREFIX_OUTPUTS TRUE CACHE BOOL "activating Flex")
elseif(UNIX)
	find_package(Bison 0)
	find_package(Flex 0)
	
endif(WIN32)



#include header files	
include_directories("../include")
include_directories(".")
include_directories("..")
#some .h files need elm files 
include_directories("../../elm/include")
include_directories("../../gel/include")


add_subdirectory(prog)
add_subdirectory(ast)
#add_subdirectory(ets)
add_subdirectory(ocpp)
add_subdirectory(odisplay)


# ILP Plugins
if(HAS_LP_SOLVE)
add_subdirectory(lp_solve)
endif(HAS_LP_SOLVE)

#if(HAS_LP_SOLVE5)
add_subdirectory(lp_solve5)
#endif(HAS_LP_SOLVE5)


# Loader selection
if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/../../ppc2")
add_subdirectory(ppc2)
#message(STATUS "${CMAKE_CURRENT_BINARY_DIR}/../../ppc2")
elseif()
add_subdirectory(ppc)
endif()

if(HAS_GLISS_ARM)
add_subdirectory(arm)
endif(HAS_GLISS_ARM)


# Tools
add_subdirectory(dumpcfg)
add_subdirectory(mkff)
add_subdirectory(ostat)
add_subdirectory(opcg)
add_subdirectory(odisasm)
add_subdirectory(owcet)


# Dependent tools
if(HAS_ILP)
add_subdirectory(bpred)
endif(HAS_ILP)
add_subdirectory(oipet)
