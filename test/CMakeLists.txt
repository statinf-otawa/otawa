
#include header files	
include_directories("../include")
include_directories(".")
include_directories("..")
#some .h files need elm files 
include_directories("../../elm/include")
include_directories("../../gel/include")



add_subdirectory(clp)
add_subdirectory(props)
add_subdirectory(reg)
