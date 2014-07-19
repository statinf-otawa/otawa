# arm2 plugin build process

set(CMAKE_INSTALL_RPATH "\\$ORIGIN/../../..")
set(CMAKE_MACOSX_RPATH true)

set(SOURCES
	"dcache.cpp"
	"dcache_ACSBuilder.cpp"
	"dcache_ACSMayBuilder.cpp"
	"dcache_BlockBuilder.cpp"
	"dcache_CatBuilder.cpp"
	"dcache_CatConstraintBuilder.cpp"
	"dcache_CLPBlockBuilder.cpp"
	"dcache_WCETFunctionBuilder.cpp"
	"Dirty.cpp"
	"Purge.cpp"
)

add_library(dcache SHARED ${SOURCES})
set_property(TARGET dcache PROPERTY PREFIX "")
set(SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}")
target_link_libraries(dcache ${LIBELM})
target_link_libraries(dcache otawa)

# installation
install(TARGETS dcache DESTINATION "${PROCDIR}/otawa")

