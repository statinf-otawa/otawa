
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

add_library(bpred MODULE ${bpred_SOURCES})
install(TARGETS bpred DESTINATION lib)
