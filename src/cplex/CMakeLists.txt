set(CMAKE_INSTALL_RPATH "${ORIGIN}/../../")
set(CMAKE_MACOSX_RPATH true)
set(CMAKE_CXX_FLAGS "-Wall" )

add_library(cplex SHARED "cplex.cpp")
set_property(TARGET cplex PROPERTY PREFIX "")
include_directories(${CPLEX_INCLUDE} ".")
target_link_libraries(cplex ${LIBELM})
target_link_libraries(cplex otawa)
target_link_libraries(cplex ${CPLEX_LIB})

install(TARGETS cplex DESTINATION "${ILPDIR}")
install(FILES cplex.eld DESTINATION "${ILPDIR}")
