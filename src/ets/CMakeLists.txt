set(CMAKE_INSTALL_RPATH "\\$ORIGIN:\\$ORIGIN/../../..")
set(CMAKE_MACOSX_RPATH true)

set(ets_la_SOURCES
	"ets_AbstractCacheState.cpp"
	"ets_ACSComputation.cpp"
	"ets_CacheFirstMissComputation.cpp"
	"ets_CacheHitComputation.cpp"
	"ets_CacheMissComputation.cpp"
	"ets_ETS.cpp"
	"ets_FlowFactLoader.cpp"
	"ets_TrivialAstBlockTime.cpp"
	"ets_WCETComputation.cpp"
	)
	
include_directories(".")

	
add_library(ets SHARED ${ets_la_SOURCES})
set_property(TARGET ets PROPERTY PREFIX "")
set(SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}")

target_link_libraries(ets ${LIBELM})
target_link_libraries(ets otawa)
target_link_libraries(ets ast)

install(TARGETS ets DESTINATION "${PROCDIR}/otawa")
install(FILES ets.eld DESTINATION "${PROCDIR}/otawa")
