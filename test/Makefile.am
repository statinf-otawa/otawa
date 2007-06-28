SUBDIRS = \
	dom \
	ct \
	ets \
	graph \
	hard \
	pcg \
	props

if HAS_ILP
SUBDIRS += \
	ccg \
	categorisation
endif

if HAS_SYSTEMC
SUBDIRS += \
	delta \
	generic_simulator
endif

if HAS_ILP
if HAS_SYSTEMC
SUBDIRS += ipet \
	display
endif
endif
