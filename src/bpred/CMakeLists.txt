set(CMAKE_INSTALL_RPATH "\\$ORIGIN/../../..")
set(CMAKE_MACOSX_RPATH true)

set(bpred_SOURCES
	  "BBHG.cpp"
	  "BBHGDrawer.h"
	  "BBHGDrawer.cpp"
	  "BHG.h"
	  "BHG.cpp"
	  "BHGDrawer.h"
	  "BHGDrawer.cpp"
	  "BCG.h"
	  "BCG.cpp"
	  "BCGDrawer.cpp"
	  "BCGDrawer.h"
	  "BSets.cpp"
	  "BSets.h"
	  "BPredProcessor_NoConflict.cpp"
	  "BPredProcessor_Bimodal.cpp"
	  "BPredProcessor_Global1B.cpp"
	  "BPredProcessor_Global2B.cpp"
	  "BPredProcessor_stats.cpp"
	  "BPredProcessor.cpp"
	  )

# BBHG.h

add_library(bpred SHARED ${bpred_SOURCES})
set_property(TARGET bpred PROPERTY PREFIX "")
set(SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}")
target_link_libraries(bpred ${LIBELM})
target_link_libraries(bpred otawa)
target_link_libraries(bpred odisplay)
install(TARGETS bpred DESTINATION "${PROCDIR}/otawa")
