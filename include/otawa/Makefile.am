# Sub-directories
SUBDIRS= \
	ast \
	cfg \
	display \
	prog \
	prop \
	ilp \
	proc \
	ipet \
	util \
	ets \
	hard \
	hardware \
	sim \
	exegraph \
	pcg

if HAS_SYSTEMC
SUBDIRS += gensim
endif

if HAS_GLISS_PPC
SUBDIRS += gliss
endif

if HAS_LP_SOLVE
SUBDIRS += lp_solve
endif


# Headers
includeotawadir=$(includedir)/otawa
includeotawa_HEADERS = \
	ast.h \
	base.h  \
	cfg.h \
	ets.h\
	gliss.h \
	hard.h \
	ilp.h \
	instruction.h \
	ipet.h \
	manager.h \
	otawa.h \
	platform.h \
	program.h \
	properties.h \
	type.h \
	pcg.h

