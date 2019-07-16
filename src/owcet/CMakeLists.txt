set(CMAKE_INSTALL_RPATH "${ORIGIN}/../lib;${ORIGIN}/../lib/otawa/proc/otawa;${ORIGIN}/../lib/otawa/otawa")
set(CMAKE_CXX_FLAGS "-Wall -g" )

set(owcet_SOURCES
	"owcet.cpp"
	)

add_executable(owcet ${owcet_SOURCES})

target_link_libraries(owcet otawa ${LIBELM})

install(TARGETS owcet DESTINATION bin)
