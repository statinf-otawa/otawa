# ppc2 plugin build

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

set(ppc2_SOURCES
	"ppc.cpp"
	${OTAWA_KIND}
	${OTAWA_PRED}
	${OTAWA_TARGET}
	${OTAWA_USED_REGS}
)

# GLISS derived sources
set(PPC_IRG		"${CMAKE_SOURCE_DIR}/../ppc2/ppc.irg")
set(GLISS_ATTR	"${CMAKE_SOURCE_DIR}/../gliss2/gep/gliss-attr")
add_custom_command(
	OUTPUT ${OTAWA_KIND} DEPENDS "kind.tpl" COMMAND ${GLISS_ATTR}
	ARGS ${PPC_IRG} -o ${OTAWA_KIND} -a otawa_kind -f -t "kind.tpl"
)
add_custom_command(
	OUTPUT ${OTAWA_PRED} DEPENDS "pred.tpl" COMMAND ${GLISS_ATTR}
	ARGS ${PPC_IRG} -o ${OTAWA_PRED} -a otawa_pred -f -t "pred.tpl" -e otawa_pred.nmp -d "return -1\\;"
)
add_custom_command(
	OUTPUT ${OTAWA_TARGET} DEPENDS "target.tpl" COMMAND ${GLISS_ATTR}
	ARGS ${PPC_IRG} -o ${OTAWA_TARGET} -a otawa_target -f -t "target.tpl" -d "return 0\\;"
)
add_custom_command(
	OUTPUT ${OTAWA_USED_REGS} DEPENDS "used_regs.tpl" COMMAND ${GLISS_ATTR}
	ARGS ${PPC_IRG} -o ${OTAWA_USED_REGS} -a otawa_used_regs -f -t "used_regs.tpl" -d "tmp_var[0] = END_REG\\; return 0\\;"
)


# ppc2.link on Windows
#add_custom_command(
#	OUTPUT ppc2.link COMMAND 
#)
if(WIN32)
file(WRITE elf_20.link "libppc2.dll")
endif()


add_library(ppc2 MODULE ${ppc2_SOURCES})
target_link_libraries(ppc2 ${LIBELM})
target_link_libraries(ppc2 libotawa)
target_link_libraries(ppc2 ${LIBPPC2})
install(TARGETS ppc2 DESTINATION lib/otawa/loader)
if(WIN32)
install(FILES elf_20.link DESTINATION lib/otawa/loader)
endif()
