SUBDIRS = \
	dom \
	ct \
	ets \
	graph \
	hard \
	pcg \
	props \
	reg \
	source

if HAS_ILP
SUBDIRS += \
	ccg \
	categorisation
endif

if HAS_SYSTEMC
SUBDIRS += \
	delta \
	generic_simulator \
	display
endif

if HAS_ILP
if HAS_SYSTEMC
SUBDIRS += ipet
endif
endif
