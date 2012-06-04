# ppc2 plugin build
set(PROC 			"ppc")
set(PROC_PREFIX 	"${CMAKE_SOURCE_DIR}/../ppc2/")
set(GLISS_LIB 		"${PROC_PREFIX}/src/lib${PROC}.a")
set(ELF_NUM			"20")

set(CMAKE_INSTALL_RPATH "\\$ORIGIN/../../")

if(MINGW OR UNIX)
	include_directories(${CMAKE_SOURCE_DIR}/../ppc2/include)
	set(LIBPPC2 "${PROJECT_SOURCE_DIR}/../ppc2/src/libppc.a")
else()
	message(FATAL_ERROR "unsupported OS")
endif()

set(OTAWA_KIND 		"${PROJECT_BINARY_DIR}/src/ppc2/otawa_kind.h")
set(OTAWA_PRED 		"${PROJECT_BINARY_DIR}/src/ppc2/otawa_pred.h")
set(OTAWA_TARGET 	"${PROJECT_BINARY_DIR}/src/ppc2/otawa_target.h")
set(OTAWA_USED_REGS "${PROJECT_BINARY_DIR}/src/ppc2/otawa_used_regs.h")
set(OTAWA_SEM		"${PROJECT_BINARY_DIR}/src/ppc2/otawa_sem.h")

set(ppc2_SOURCES
	"ppc.cpp"
	${OTAWA_KIND}
	${OTAWA_PRED}
	${OTAWA_TARGET}
	${OTAWA_USED_REGS}
	${OTAWA_SEM}
)

# GLISS derived sources
if(NOT MINGW_WIN)
	set(PPC_IRG		"${CMAKE_SOURCE_DIR}/../ppc2/ppc.irg")
	set(GLISS_ATTR	"${CMAKE_SOURCE_DIR}/../gliss2/gep/gliss-attr")
	add_custom_command(
		OUTPUT ${OTAWA_KIND} DEPENDS "kind.tpl" COMMAND ${GLISS_ATTR}
		ARGS ${PPC_IRG} -o ${OTAWA_KIND} -a otawa_kind -f -t "kind.tpl" -d "return 0 \\;" -e otawa_kind.nmp
		DEPENDS ${PPC_IRG}
	)
	add_custom_command(
		OUTPUT ${OTAWA_PRED} DEPENDS "pred.tpl" COMMAND ${GLISS_ATTR}
		ARGS ${PPC_IRG} -o ${OTAWA_PRED} -a otawa_pred -f -t "pred.tpl" -e otawa_pred.nmp -d "return -1\\;"
		DEPENDS ${PPC_IRG}
	)
	add_custom_command(
		OUTPUT ${OTAWA_TARGET} DEPENDS "target.tpl" COMMAND ${GLISS_ATTR}
		ARGS ${PPC_IRG} -o ${OTAWA_TARGET} -a otawa_target -f -t "target.tpl" -d "return 0\\;" -e otawa_target.nmp
		DEPENDS ${PPC_IRG}
	)
	add_custom_command(
		OUTPUT ${OTAWA_USED_REGS} DEPENDS "used_regs.tpl" COMMAND ${GLISS_ATTR}
		ARGS ${PPC_IRG} -o ${OTAWA_USED_REGS} -a otawa_used_regs -f -t "used_regs.tpl" -d "tmp_var[0] = END_REG\\; return 0\\;" -e otawa_used_regs.nmp
		DEPENDS ${PPC_IRG}
	)
	add_custom_command(
		OUTPUT ${OTAWA_SEM} DEPENDS "otawa_sem.tpl" "otawa_sem.nmp" COMMAND ${GLISS_ATTR}
		ARGS ${PPC_IRG} -o ${OTAWA_SEM} -a otawa_sem -p -t "otawa_sem.tpl" -d "';'" -e otawa_sem.nmp
		DEPENDS ${PPC_IRG}
	)
endif()


# ppc2.link on Windows
#add_custom_command(
#	OUTPUT ppc2.link COMMAND 
#)


add_library(ppc2 SHARED ${ppc2_SOURCES})
set_property(TARGET ppc2 PROPERTY PREFIX "")
set(SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}")
target_link_libraries(ppc2 ${LIBELM})
target_link_libraries(ppc2 otawa)
target_link_libraries(ppc2 ${LIBPPC2})

# installation
set(INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/lib/otawa/loader)
install(TARGETS ppc2 DESTINATION ${INSTALL_PATH})
if(WIN32)
	file(WRITE elf_${ELF_NUM}.link "${PROC}.dll")
	install(FILES elf_${ELF_NUM}.link DESTINATION ${INSTALL_PATH})
elseif(UNIX OR APPLE)
	add_custom_command(OUTPUT elf_${ELF_NUM}${SUFFIX} COMMAND rm -rf elf_${ELF_NUM}${SUFFIX} COMMAND ln -s ${PROC}2${SUFFIX} elf_${ELF_NUM}${SUFFIX})
	add_custom_target(ELF${ELF_NUM} ALL DEPENDS elf_${ELF_NUM}${SUFFIX})
	install(FILES elf_${ELF_NUM}${SUFFIX} DESTINATION ${INSTALL_PATH})
endif()
install(FILES ppc-config.xml DESTINATION ${INSTALL_PATH})

