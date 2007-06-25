BUILD NOTES
-----------

  This file describes how to build Otawa from a developer point of view.
The unique mandatory dependency concerns Elm library that may be check-out from
the OTAWA CVS repository (look at section 2 for compiling elm):
	$ cvs co elm

  Optional dependencies includes third-party programs that may be downloaded
from developers websites (sections 3 and 4). For making compilation easier,
you may create their directories and unarchive them at the same level than
OTAWA. In this way, you do not need to pass special arguments to configure
for finding dependencies. Usually, the easier way to proceed is to create
a top directory and to unarchive or check-out all modules in this directory.

  The first section, that follows, shows how to compile Otawa for developing
purpose. Please, note also that OTAWA may be build using a script disctributed
on the official site: http://www.irit.fr/recherches/ARCHI/MARCH/OTAWA.

1. Compiling Otawa

  Go in otawa and prepare the configuration:
    $ cvs -d :pserver:anonymous@cvs.irit.fr:/usr/local/CVS_IRIT/CVS_OTAWA co otawa
  	$ cd otawa
  	$ ./bootstrap
	$ ./configure <options>
	$ make install

  Type "configure --help" for the list of configuration option. OTAWA specific
configuration options are:

  --with-plugin: enable the plug-in of OTAWA (instead, the plugin linkage is
  	static),
  --with-loader={ppc,s12x}: for a static linkage of plugin, select the 
    supported ISA (default to ppc),
  --with-mode={dev,debug,normal,final}: select the compilation mode
	dev: for OTAWA developper only,
	debug: compile with debug options,
	normal: normal (assertion stays activated),
	final: for production mode only (inactive assertions cause a little
	  performance improvement),
  --prefix=PATH: select the installation directory (default to /usr/local).

2. Compiling Elm Library

  Elm may be got from OTAWA CVS repository:
    $ cvs -d :pserver:anonymous@cvs.irit.fr:/usr/local/CVS_IRIT/CVS_OTAWA co elm
  Then you may compile with the following commands:
  	$ cd elm
  	$ ./bootstrap
	$ ./configure <options>
	$ make install

Options includes:
  --with-mode={dev,normal,final}: select the compilation mode,
    dev: for ELM developpers,
    normal: normal use of ELM (assertion activated),
    final: for production work (assertions unactivated).
  --prefix=PATH: select the installation directory (default to /usr/local).

  	
3. Compiling Gliss or S12X PPC Package

  You can get the Gliss archive and either the PowerPC Gliss implementation, or
the S12X implementation from CVS. In the following, the commands dedicared to
an ISA are prefixed with [ppc] or [s12x].

  To check-out the archive type:
       $ cvs -d :pserver:anonymous@cvs.irit.fr:/usr/local/CVS_IRIT/CVS_OTAWA co glis
[ppc]  $ cvs -d :pserver:anonymous@cvs.irit.fr:/usr/local/CVS_IRIT/CVS_OTAWA co ppc
[s12x] $ cvs -d :pserver:anonymous@cvs.irit.fr:/usr/local/CVS_IRIT/CVS_OTAWA co hcs12

  Compile them:
       $ cd gliss
       $ make all
[ppc]  $ cd ../ppc
       $ make make all OPT=-DISS_DISASM GEP_OPTS="-a user0 int8 -a category int8"
[s12x] $ cd ../hcs12
       $ make all OPT=-DISS_DISASM GEP_OPTS="-a otawa_kind uint32 -a time string -a time_select uint8 -a time_select2 uint8"


4. lp_solve Package

  The lp_solve package may be used for providing ILP support. OTAWA works
with version 4.0 of lp_solve but the compilation does not work with versions
less than 2.0.

  The lp_solve package may be retrieved from address
ftp://ftp.es.ele.tue.nl/pub/lp_solve/old_versions_which_you_probably_dont_want/.

	$ wget ftp://ftp.es.ele.tue.nl/pub/lp_solve/old_versions_which_you_probably_dont_want/ftp://ftp.es.ele.tue.nl/pub/lp_solve/old_versions_which_you_probably_dont_want/lp_solve_4.0.tar.gz

  Uncompress it and compile it:
  	$ tar xvfz lp_solve_4.0.tar.gz
  	$ cd lp_solve_4.0
  	$ make -f Makefile.linux
  	$ cd ..
  	$ ln -s lp_solve-2.0 lp_solve
