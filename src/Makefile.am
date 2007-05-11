SUBDIRS= \
	prog \
	odisplay \
	oloader

if HAS_GLISS_PPC
SUBDIRS += ppc
endif

if HAS_GLISS_ARM
SUBDIRS += arm
endif

SUBDIRS += \
	dumpcfg \
	mkff \
	ostat

if HAS_SYSTEMC
SUBDIRS += ogensim
endif

if HAS_LP_SOLVE
SUBDIRS += lp_solve
endif

if HAS_ILP
SUBDIRS += oipet
endif



#all-local:
#	(cd mkast; make)

#clean-local:
#	(cd mkast; make clean)

#install-exec-hook:
#	(cd mkast; make install)
