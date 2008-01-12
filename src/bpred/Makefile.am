procdir=$(libdir)/otawa/proc

AM_CXXFLAGS = @OTAWA_CXXFLAGS@ @GEL_CXXFLAGS@

proc_LTLIBRARIES = bpred.la

bpred_la_LDFLAGS = -module
bpred_la_LIBADD = @OTAWA_LIBS@
bpred_la_DEPENDENCIES = @OTAWA_DEPS@

bpred_la_SOURCES = \
					  BBHG.cpp \
					  BBHGDrawer.h \
					  BBHGDrawer.cpp \
					  BHG.h \
					  BHG.cpp \
					  BHGDrawer.h \
					  BHGDrawer.cpp \
					  BCG.h \
					  BCG.cpp \
					  BCGDrawer.cpp \
					  BCGDrawer.h \
					  BSets.cpp \
					  BSets.h \
					  BPredProcessor_NoConflict.cpp \
					  BPredProcessor_Bimodal.cpp \
					  BPredProcessor_Global1B.cpp \
					  BPredProcessor_Global2B.cpp \
					  BPredProcessor_stats.cpp \
					  BPredProcessor.cpp

# BBHG.h 