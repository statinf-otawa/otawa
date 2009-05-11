SUBDIRS = \
	dom \
	checksum \
	ct \
	dynproc \
	ets \
	graph \
	hard \
	pcg \
	pfg \
	props \
	reg \
	source

if HAS_ILP
SUBDIRS += \
	ccg \
	categorisation \
	ilp 
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
