SUBDIRS= \
	prog \
	odisplay \
	oloader

# Loader selection
if HAS_GLISS_PPC
SUBDIRS += ppc
endif

if HAS_GLISS_ARM
SUBDIRS += arm
endif

if HAS_GLISS_S12X
SUBDIRS += s12x
endif


# Tools
SUBDIRS += \
	dumpcfg \
	mkff \
	ostat

# Dependent libraries
if HAS_SYSTEMC
SUBDIRS += ogensim
endif

# ILP Plugins
if HAS_LP_SOLVE
SUBDIRS += lp_solve
endif

# Dependent tools
if HAS_ILP
SUBDIRS += oipet
endif



#all-local:
#	(cd mkast; make)

#clean-local:
#	(cd mkast; make clean)

#install-exec-hook:
#	(cd mkast; make install)
