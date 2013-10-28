
#include header files	
include_directories("../include")
include_directories(".")
include_directories("..")
#some .h files need elm files 
include_directories("../../elm/include")
include_directories("../../gel/include")



add_subdirectory(checksum)
add_subdirectory(clp)
add_subdirectory(dom)
add_subdirectory(hard)
add_subdirectory(slook)


# to be checked
#add_subdirectory(ct)
#add_subdirectory(dynproc)
#add_subdirectory(ets)
#add_subdirectory(graph)
#add_subdirectory(newexegraphs)
#add_subdirectory(pcg)
#add_subdirectory(pfg)
#add_subdirectory(props)
#add_subdirectory(reg)
#Add_subdirectory(script)
#add_subdirectory(source)
#add_subdirectory(subcfg)


if(HAS_ILP)
add_subdirectory(ccg)
add_subdirectory(categorisation)
add_subdirectory(ilp)
endif(HAS_ILP)
