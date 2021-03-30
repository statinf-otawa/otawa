set(CMAKE_INSTALL_RPATH "${ORIGIN}/../../:${ORIGIN}")
set(CMAKE_MACOSX_RPATH true)
set(CMAKE_CXX_FLAGS "-Wall")

add_library(icat3 SHARED
	icat3_CatBuilder.cpp
	icat3_EdgeEventBuilder.cpp
	icat3_EventBuilder.cpp
	icat3_hook.cpp
	icat3_LBlockBuilder.cpp
	icat3_MustDomain.cpp
	icat3_MustPersDomain.cpp
	icat3_MustPersAnalysis.cpp
	icat3_PersDomain.cpp
	icat3_MayAnalysis.cpp)
set_property(TARGET icat3 PROPERTY PREFIX "")
target_link_libraries(icat3 ${LIBELM})
target_link_libraries(icat3 otawa)
target_link_libraries(icat3 etime)

install(TARGETS icat3 DESTINATION "${PLUGDIR}/otawa")
install(FILES icat3.eld DESTINATION "${PLUGDIR}/otawa")
