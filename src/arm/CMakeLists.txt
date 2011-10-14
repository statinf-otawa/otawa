# arm plugin build
set(CMAKE_INSTALL_RPATH "\\$ORIGIN/../..")

if(MINGW OR UNIX)
	include_directories(${CMAKE_SOURCE_DIR}/../arm/include)
	set(LIBARM "${PROJECT_SOURCE_DIR}/../arm/lib/libemul.a")
else()
	message(FATAL_ERROR "unsupported OS")
endif()


set(arm_SOURCES
	"arm.cpp"
)
# installation
add_library(arm SHARED ${arm_SOURCES})
set_property(TARGET arm PROPERTY PREFIX "")
set(SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}")
target_link_libraries(arm ${LIBELM})
target_link_libraries(arm otawa oloader)
target_link_libraries(arm ${LIBARM})
set(INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/lib/otawa/loader)
install(TARGETS arm DESTINATION ${INSTALL_PATH})
if (UNIX OR APPLE)
  add_custom_command(OUTPUT elf_40${SUFFIX} COMMAND ln -s arm${SUFFIX} elf_40${SUFFIX})
  add_custom_target(ELF_40 ALL DEPENDS elf_40${SUFFIX})
  install(FILES elf_40${SUFFIX} DESTINATION ${INSTALL_PATH})
endif()


