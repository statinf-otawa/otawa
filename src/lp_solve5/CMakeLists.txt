set(CMAKE_INSTALL_RPATH "\\$ORIGIN/../../")

set(lp_solve5_SOURCES
	"lp_solve5.cpp"
)
	
add_library(lp_solve5 SHARED ${lp_solve5_SOURCES})
set_property(TARGET lp_solve5 PROPERTY PREFIX "")
include_directories(${LP_SOLVE5_INCLUDE})
target_link_libraries(lp_solve5 ${LIBELM})
target_link_libraries(lp_solve5 otawa)
target_link_libraries(lp_solve5 STATIC ${LP_SOLVE5})
install(TARGETS lp_solve5 DESTINATION lib/otawa/ilp)
