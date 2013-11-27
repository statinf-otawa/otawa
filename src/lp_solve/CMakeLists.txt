set(CMAKE_INSTALL_RPATH "\\$ORIGIN/../../")
set(CMAKE_MACOSX_RPATH true)

set (lp_solve_SOURCES
	"lp_solve_System.cpp"
	"lp_solve_Constraint.cpp"
	"lp_solve_Plugin.cpp"
	)
	
include_directories(".")

	
add_library(lp_solve SHARED ${lp_solve_SOURCES})
set_property(TARGET lp_solve PROPERTY PREFIX "")
target_link_libraries(lp_solve ${LIBELM})
target_link_libraries(lp_solve otawa)


install(TARGETS lp_solve DESTINATION lib)
