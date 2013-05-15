# arm2 plugin build process

set(CMAKE_INSTALL_RPATH "\\$ORIGIN/../../..")

set(SOURCES
	"dcache.cpp"
	"dcache_ACSBuilder.cpp"
	"dcache_ACSMayBuilder.cpp"
	"dcache_BlockBuilder.cpp"
	"dcache_CatBuilder.cpp"
	"dcache_CatConstraintBuilder.cpp"
	"Dirty.cpp"
)

add_library(dcache SHARED ${SOURCES})
set_property(TARGET dcache PROPERTY PREFIX "")
set(SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}")
target_link_libraries(dcache ${LIBELM})
target_link_libraries(dcache otawa)

# installation
set(INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/lib/otawa/proc/otawa)
install(TARGETS dcache DESTINATION ${INSTALL_PATH})

