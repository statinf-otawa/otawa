# arm2 plugin build process

set(PROC 			"arm")
set(ELF_NUM			"40")
set(PROC_PREFIX     "${ARM_PATH}")
set(GLISS_LIB 		"${PROC_PREFIX}/src/lib${PROC}.a")
set(IRG			    "${PROC_PREFIX}/${PROC}.irg")

if(ARM7)
    set(ARM_AUX "armv7t.nmp")
else()
    set(ARM_AUX "armv5t.nmp")
endif()


# source generation
set(CMAKE_CXX_FLAGS "-Wall" )
set(GLISS_ATTR	"${CMAKE_SOURCE_DIR}/../gliss2/gep/gliss-attr")
include("../../data/cmake/Otawa.cmake")
MAKE_GLISS_FUNCTION(OTAWA_KIND    "otawa_kind"      "otawa_kind.nmp" "${IRG}"    "return 0;"    "${ARM_AUX}")
MAKE_GLISS_FUNCTION(OTAWA_TARGET  "otawa_target"    "otawa_target.nmp" "${IRG}"  "return 0;"    "${ARM_AUX}")
MAKE_GLISS_PROCEDURE(OTAWA_SEM    "otawa_sem"       "otawa_sem.nmp" "${IRG}"      ";"           "${ARM_AUX}")
MAKE_GLISS_FUNCTION(OTAWA_MULTI   "otawa_multi"     "otawa_multi.nmp" "${IRG}"   "return 0;"    "${ARM_AUX}")

set(SOURCES
	"${PROC}.cpp"
	${OTAWA_KIND}
	${OTAWA_TARGET}
	${OTAWA_SEM}
	${OTAWA_MULTI}
)

include_directories(${PROC_PREFIX}/include)

if(ARM_SIM)
	add_definitions(-DARM_SIM)
endif()

# module definition
set(CMAKE_INSTALL_RPATH "${ORIGIN}/../../")
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
install(FILES arm.eld DESTINATION "${PROCDIR}/otawa")
