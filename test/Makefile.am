SUBDIRS = \
	dom \
	display \
	ct \
	ets \
	graph \
	hard \
	pcg \
	props

if HAS_ILP
SUBDIRS += \
	ccg \
	categorisation \
	exegraph
endif

if HAS_SYSTEMC
SUBDIRS += \
	delta \
	systemc \
	generic_simulator
endif

if HAS_ILP
if HAS_SYSTEMC
SUBDIRS += ipet  
endif
endif
