# arm2 plugin build process

set(PROC 			"arm")
set(PROC_PREFIX 	"${CMAKE_SOURCE_DIR}/../armv5t/")
set(GLISS_LIB 		"${PROC_PREFIX}/src/lib${PROC}.a")
set(ELF_NUM			"40")

set(CMAKE_INSTALL_RPATH "\\$ORIGIN/../../")
include_directories(${PROC_PREFIX}/include)

# file definitions
set(OTAWA_KIND 			"${CMAKE_CURRENT_BINARY_DIR}/otawa_kind.h")
set(OTAWA_PRED 			"${CMAKE_CURRENT_BINARY_DIR}/otawa_pred.h")
set(OTAWA_TARGET 		"${CMAKE_CURRENT_BINARY_DIR}/otawa_target.h")
#set(OTAWA_USED_REGS	"${CMAKE_CURRENT_BINARY_DIR}/otawa_used_regs.h")

set(SOURCES
	"${PROC}.cpp"
	${OTAWA_KIND}
	${OTAWA_TARGET}
#	${OTAWA_USED_REGS}
)

set(IRG			"${PROC_PREFIX}/${PROC}.irg")
set(GLISS_ATTR	"${CMAKE_SOURCE_DIR}/../gliss2/gep/gliss-attr")


# otawa_XXX build
add_custom_command(
	OUTPUT ${OTAWA_KIND} DEPENDS "otawa_kind.tpl" "otawa_kind.nmp"
	COMMAND ${GLISS_ATTR}
	ARGS ${IRG} -o ${OTAWA_KIND} -a otawa_kind -f -t "otawa_kind.tpl" -d "return 0\\;" -e otawa_kind.nmp
)
add_custom_command(
	OUTPUT ${OTAWA_TARGET} DEPENDS "otawa_target.tpl" "otawa_target.nmp"
	COMMAND ${GLISS_ATTR}
	ARGS ${IRG} -o ${OTAWA_TARGET} -a otawa_target -f -t "otawa_target.tpl" -d "return 0\\;" -e otawa_target.nmp
)
#add_custom_command(
#	OUTPUT ${OTAWA_USED_REGS} DEPENDS "used_regs.tpl" COMMAND ${GLISS_ATTR}
#	ARGS ${IRG} -o ${OTAWA_USED_REGS} -a otawa_used_regs -f -t "used_regs.tpl" -d "tmp_var[0] = END_REG\\; return 0\\;" -e otawa_used_regs.nmp
#)


# module definition
add_library(${PROC} SHARED ${SOURCES})
set_property(TARGET ${PROC} PROPERTY PREFIX "")
set(SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}")
target_link_libraries(${PROC} ${LIBELM})
target_link_libraries(${PROC} otawa)
target_link_libraries(${PROC} ${GLISS_LIB})

# installation
set(INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/lib/otawa/loader)
install(TARGETS ${PROC} DESTINATION ${INSTALL_PATH})
if(WIN32)
	file(WRITE elf_${ELF_NUM}.link "${PROC}.dll")
	install(FILES elf_${ELF_NUM}.link DESTINATION ${INSTALL_PATH})
elseif(UNIX OR APPLE)
	add_custom_command(OUTPUT elf_${ELF_NUM}${SUFFIX} COMMAND ln -s ${PROC}${SUFFIX} elf_${ELF_NUM}${SUFFIX})
	add_custom_target(ELF ALL DEPENDS elf_${ELF_NUM}${SUFFIX})
	install(FILES elf_${ELF_NUM}${SUFFIX} DESTINATION ${INSTALL_PATH})
endif()

