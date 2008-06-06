# Sub-directories
SUBDIRS= \
	ast \
	cache \
	cfg \
	dfa \
	display \
	ets \
	exegraph \
	hard \
	hardware \
	ilp \
	ipet \
	loader \
	pcg \
	proc \
	prog \
	prop \
	sim \
	util


if HAS_SYSTEMC
SUBDIRS += gensim
endif

if HAS_LP_SOLVE
SUBDIRS += lp_solve
endif


# Headers
includeotawadir=$(includedir)/otawa
includeotawa_HEADERS = \
	sim.h \
	ast.h \
	base.h  \
	cfg.h \
	ets.h\
	flowfact \
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

