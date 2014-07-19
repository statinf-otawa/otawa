# ppc2 plugin build
set(PROC 			"ppc")
set(ELF_NUM			"20")
set(PROC_PREFIX 	"${CMAKE_SOURCE_DIR}/../ppc2/")
set(GLISS_LIB 		"${PROC_PREFIX}/src/lib${PROC}.a")
set(PPC_IRG		    "${CMAKE_SOURCE_DIR}/../ppc2/ppc.irg")

if(MINGW OR UNIX)
	include_directories(${CMAKE_SOURCE_DIR}/../ppc2/include)
	set(LIBPPC2 "${PROJECT_SOURCE_DIR}/../ppc2/src/libppc.a")
else()
	message(FATAL_ERROR "unsupported OS")
endif()


# sources
set(GLISS_ATTR	"${CMAKE_SOURCE_DIR}/../gliss2/gep/gliss-attr")
include("../../data/cmake/Otawa.cmake")
MAKE_GLISS_FUNCTION(OTAWA_KIND       "otawa_kind"      "otawa_kind.nmp"      "${PPC_IRG}"    "return 0;")
MAKE_GLISS_FUNCTION(OTAWA_TARGET     "otawa_target"    "otawa_target.nmp"    "${PPC_IRG}"    "return 0;")
MAKE_GLISS_PROCEDURE(OTAWA_SEM       "otawa_sem"       "otawa_sem.nmp"       "${PPC_IRG}"    ";")
MAKE_GLISS_FUNCTION(OTAWA_PRED       "otawa_pred"      "otawa_pred.nmp"      "${PPC_IRG}"    "return 0;")
MAKE_GLISS_FUNCTION(OTAWA_USED_REGS  "otawa_used_regs" "otawa_used_regs.nmp" "${PPC_IRG}"    "return 0;")

set(ppc2_SOURCES
	"ppc.cpp"
	${OTAWA_KIND}
	${OTAWA_PRED}
	${OTAWA_TARGET}
	${OTAWA_USED_REGS}
	${OTAWA_SEM}
)

set(CMAKE_INSTALL_RPATH "\\$ORIGIN/../../")
set(CMAKE_MACOSX_RPATH true)


# build the library
add_library(ppc2 SHARED ${ppc2_SOURCES})
set_property(TARGET ppc2 PROPERTY PREFIX "")
set(SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}")
target_link_libraries(ppc2 ${LIBELM})
target_link_libraries(ppc2 otawa)
target_link_libraries(ppc2 ${LIBPPC2})

# installation
install(TARGETS ppc2 DESTINATION "${LOADERDIR}")
install(FILES elf_20.eld ppc-config.xml DESTINATION "${LOADERDIR}")

