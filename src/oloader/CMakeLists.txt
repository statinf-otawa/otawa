

set(SOURCES
	gliss.cpp
	loader_new_gliss_BranchInst.cpp
	loader_new_gliss_Inst.cpp
	loader_new_gliss_Process.cpp
	loader_old_gliss_BranchInst.cpp
	loader_old_gliss_Inst.cpp
	loader_old_gliss_Process.cpp
)
add_library(oloader STATIC ${SOURCES})
