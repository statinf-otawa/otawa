# arm1 plugin

# arm plugin build
set(CMAKE_INSTALL_RPATH "\\$ORIGIN/../..")

set(arm1_SOURCES "arm.cpp")

# plugin build
include_directories("${ARM_PATH}/include")
add_library(arm1 SHARED ${arm1_SOURCES})
set_property(TARGET arm1 PROPERTY PREFIX "")
set(SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}")
target_link_libraries(arm1 ${LIBELM})
target_link_libraries(arm1 otawa)
target_link_libraries(arm1 ${ARM_LIB})
target_link_libraries(arm1 oloader)

# plugin installation
set(INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/lib/otawa/loader)
install(TARGETS arm1 DESTINATION ${INSTALL_PATH})
if(WIN32)
	file(WRITE elf_40.link "arm1.dll")
	install(FILES elf_40.link DESTINATION ${INSTALL_PATH})
	file(WRITE arm.link "arm1.dll")
	install(FILES arm.link DESTINATION ${INSTALL_PATH})
elseif(UNIX OR APPLE)
	add_custom_command(OUTPUT elf_40${SUFFIX} COMMAND ln -s arm1${SUFFIX} elf_40${SUFFIX})
	add_custom_command(OUTPUT arm${SUFFIX} COMMAND ln -s arm1${SUFFIX} arm${SUFFIX})
	add_custom_target(ELF_40 ALL DEPENDS elf_40${SUFFIX} arm${SUFFIX})
	install(FILES elf_40${SUFFIX} arm${SUFFIX} DESTINATION ${INSTALL_PATH})
endif()


