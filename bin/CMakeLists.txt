 find_program(BASH_PATH bash)
 set(prefix ${CMAKE_INSTALL_PREFIX})
 
 if(HAS_LP_SOLVE)
	set(HAS_LP_SOLVE_TRUE "")
else()
	set(HAS_LP_SOLVE_TRUE "#")
endif()

if(LP_SOLVE5)
	set(HAS_LP_SOLVE5_TRUE "")
else()
	set(HAS_LP_SOLVE5_TRUE "#")
endif()
 
if(HAS_GLISS_PPC)
	set(HAS_GLISS_PPC_TRUE "")
else()
	set(HAS_GLISS_PPC_TRUE "#")
endif()

if(HAS_GLISS_ARM)
	set(HAS_GLISS_ARM_TRUE "")
else()
	set(HAS_GLISS_ARM_TRUE "#")
endif()

if(HAS_LP_SOLVE OR HAS_LP_SOLVE5)
	set(HAS_ILP_TRUE "")
else()
	set(HAS_ILP_TRUE "#")
endif()

set(HAS_SO "yes")


configure_file(
	otawa-config.in
	otawa-config
	@ONLY
)

install(FILES otawa-config DESTINATION bin PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE)

