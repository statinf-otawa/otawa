

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

	
add_library(ets MODULE ${ets_la_SOURCES})
install(TARGETS ets DESTINATION lib)

target_link_libraries(ets ${LIBELM})
target_link_libraries(ets libotawa)
target_link_libraries(ets ast)