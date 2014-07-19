# arm2 plugin build process

set(PROC 			"arm")
set(ELF_NUM			"40")
set(PROC_PREFIX 	"${CMAKE_SOURCE_DIR}/../armv5t/")
set(GLISS_LIB 		"${PROC_PREFIX}/src/lib${PROC}.a")
set(IRG			    "${PROC_PREFIX}/${PROC}.irg")

# source generation
set(GLISS_ATTR	"${CMAKE_SOURCE_DIR}/../gliss2/gep/gliss-attr")
include("../../data/cmake/Otawa.cmake")
MAKE_GLISS_FUNCTION(OTAWA_KIND    "otawa_kind"      "otawa_kind.nmp" "${IRG}"    "return 0;")
MAKE_GLISS_FUNCTION(OTAWA_TARGET  "otawa_target"    "otawa_target.nmp" "${IRG}"  "return 0;")
MAKE_GLISS_PROCEDURE(OTAWA_SEM     "otawa_sem"       "otawa_sem.nmp" "${IRG}"     ";")
MAKE_GLISS_FUNCTION(OTAWA_MULTI   "otawa_multi"     "otawa_multi.nmp" "${IRG}"   "return 0;")

set(SOURCES
	"${PROC}.cpp"
	${OTAWA_KIND}
	${OTAWA_TARGET}
	${OTAWA_SEM}
	${OTAWA_MULTI}
)

include_directories(${PROC_PREFIX}/include)


# module definition
set(CMAKE_INSTALL_RPATH "\\$ORIGIN/../../")
set(CMAKE_MACOSX_RPATH true)
add_library(${PROC} SHARED ${SOURCES})
set_property(TARGET ${PROC} PROPERTY PREFIX "")
set(SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}")
target_link_libraries(${PROC} ${LIBELM})
target_link_libraries(${PROC} otawa)
target_link_libraries(${PROC} ${GLISS_LIB})

# installation
install(TARGETS ${PROC} DESTINATION "${LOADERDIR}")
install(FILES elf_${ELF_NUM}.eld DESTINATION "${LOADERDIR}")
