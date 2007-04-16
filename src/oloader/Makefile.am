AM_CXXFLAGS = @OTAWA_CXXFLAGS@

lib_LTLIBRARIES = liboloader.la

liboloader_la_LDFLAGS = -all-static
liboloader_la_LIBADD = @OTAWA_LIBS@
liboloader_la_DEPENDENCIES = @OTAWA_DEPS@

liboloader_la_SOURCES = \
	loader_old_gliss_BranchInst.cpp \
	loader_old_gliss_Inst.cpp \
	loader_old_gliss_Process.cpp
