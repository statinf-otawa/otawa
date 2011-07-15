

set (lp_solve_SOURCES
	"lp_solve_System.cpp"
	"lp_solve_Constraint.cpp"
	"lp_solve_Plugin.cpp"
	)
	
include_directories(".")

	
add_library(lp_solve MODULE ${lp_solve_SOURCES})
install(TARGETS lp_solve DESTINATION lib)
