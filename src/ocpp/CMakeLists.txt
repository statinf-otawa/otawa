set(CMAKE_INSTALL_RPATH "\\$ORIGIN/../lib")
set(CMAKE_MACOSX_RPATH true)

set(ocpp_SOURCES
	"Unmangler.cpp"
	)
	
add_library(ocpp SHARED ${ocpp_SOURCES})
install(TARGETS ocpp DESTINATION ${LIBDIR})

target_link_libraries(ocpp ${LIBELM})
