set(CMAKE_INSTALL_RPATH "../../\\$ORIGIN")

if(UNIX OR APPLE)
	include_directories(${CMAKE_SOURCE_DIR}/../lp_solve5)
	set(LPSOLVE "${CMAKE_SOURCE_DIR}/../lp_solve5/lpsolve55/liblpsolve55.a")	
elseif(MINGW_LINUX)
	include_directories(${CMAKE_SOURCE_DIR}/../lp_solve5)
	set(LPSOLVE "${CMAKE_SOURCE_DIR}/../lp_solve5/lpsolve55.lib")
elseif(WIN32)
	include_directories("${CMAKE_SOURCE_DIR}/../lp_solve_5.5.2.0_dev_win32")
	find_library(LPSOLVE lpsolve55 PATHS "${CMAKE_SOURCE_DIR}/../lp_solve_5.5.2.0_dev_win32")
endif()

set(lp_solve5_SOURCES
	"lp_solve5.cpp"
)
	
add_library(lp_solve5 MODULE ${lp_solve5_SOURCES})
target_link_libraries(lp_solve5 ${LIBELM})
target_link_libraries(lp_solve5 otawa)
target_link_libraries(lp_solve5 ${LPSOLVE})
install(TARGETS lp_solve5 DESTINATION lib/otawa/ilp)
