otawaprocdir=$(libdir)/otawa/proc/otawa

AM_CXXFLAGS = @OTAWA_CXXFLAGS@ @GEL_CXXFLAGS@

otawaproc_LTLIBRARIES = ets.la

ets_la_LDFLAGS = -module
ets_la_LIBADD = @OTAWA_LIBS@  @PLUGIN_LIBS@
ets_la_DEPENDENCIES = @OTAWA_DEPS@

ets_la_SOURCES = \
	ets_AbstractCacheState.cpp \
	ets_ACSComputation.cpp \
	ets_CacheFirstMissComputation.cpp \
	ets_CacheHitComputation.cpp \
	ets_CacheMissComputation.cpp \
	ets_ETS.cpp \
	ets_FlowFactLoader.cpp \
	ets_TrivialAstBlockTime.cpp \
	ets_WCETComputation.cpp
