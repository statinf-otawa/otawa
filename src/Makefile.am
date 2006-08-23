SUBDIRS= \
	prog \
	@PLUGINS@ \
	oshell \
	dumpcfg \
	mkff \
	ostat \
	odisplay

if WITH_ILP
SUBDIRS += oipet
endif

if WITH_SYSTEMC
SUBDIRS += ogensim
endif

#all-local:
#	(cd mkast; make)

#clean-local:
#	(cd mkast; make clean)

#install-exec-hook:
#	(cd mkast; make install)
