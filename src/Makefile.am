SUBDIRS= \
	prog \
	ast \
	ets \
	odisplay \
	oloader 

# ILP Plugins
if HAS_LP_SOLVE
SUBDIRS += lp_solve
endif
if HAS_LP_SOLVE5
SUBDIRS += lp_solve5
endif

# Loader selection
if HAS_GLISS2_PPC
SUBDIRS += ppc2
else
if HAS_GLISS_PPC
SUBDIRS += ppc
endif
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
	ostat \
	opcg \
	odisasm \
	owcet

# Dependent libraries
if HAS_SYSTEMC
SUBDIRS += ogensim
endif


# Dependent tools
if HAS_ILP
SUBDIRS += bpred
endif
SUBDIRS += oipet



#all-local:
#	(cd mkast; make)

#clean-local:
#	(cd mkast; make clean)

#install-exec-hook:
#	(cd mkast; make install)
