BUILD NOTES
-----------

  This file describes how to build Otawa from a developer point of view.
The unique mandatory dependency concerns Elm library that may be check-out from
the Otawa CVS repository (look at section 2 for compiling elm):
	$ cvs co elm

  Optional dependencies includes third-party programs that may be downloaded
from developers websites (sections 3 and 4). For making compilation easier,
you may create in Otawa directory a directory called "extern" and unarchive and
compile third-party dependencies in it. In this way, you do not need to pass
special arguments to configure for finding dependencies:
	$ cd otawa
	$ mkdir extern

  The first section, that follows, shows how to compile Otawa for developing
purpose.

1. Compiling Otawa

  Go in otawa and prepare the configuration:
  	$ cd otawa
	$ autoheader
	$ aclocal
	$ autoconf
	$ automake --add-missing
	$ ./configure <options>
	$ make

  Type "configure --help" for the list of configuration option. Otawa specific
configuration options are:

	--with-elm: assume the Elm directory is at "otawa/elm".
	--with-elm=PATH: use the Elm library in the given PATH.

	--with-glissppc: use the Gliss PPC module (default) and look for the
		GlissPPC package in "otawa/gliss-ppc".
	--with-glissppc=PATH: allow specifiying the path to the Gliss PPC package.
	--without-glissppc: do not use the Gliss PPC module.
	
	--with-lp_solve=yes: use lp_solve assumed to be in extern sub-directory.
	--with-lp_solve=PATH: use lp_solve at the given path.
	
	--enable-heptane: use the Heptane AST loader (default, require glissppc).
	--disable-heptane: do not use the Heptane AST loader.
	
	--enable-doc: enable the automatic documentation generation
		(require doxygen).
	--disable-doc: disable the automatic documentation generation.

2. Compiling Elm Library

  Elm may be got from OTAWA CVS repository:
  	$ cvs co elm
  Then you may compile with the following commands:
  	$ cd elm
	$ autoheader
	$ aclocal
	$ autoconf
	$ automake --add-missing
	$ ./configure <options>
	$ make

  	
3. Compiling Gliss PPC Package

  You can get the Gliss archive and the PowerPC Gliss implementation at the
address http://www.irit.fr/recherches/ARCHI/MARCH/.
	$ wget http://www.irit.fr/recherches/ARCHI/MARCH/GEP/gliss.tgz
	$ wget http://www.irit.fr/recherches/ARCHI/MARCH/GEP/ppc.tgz

  Uncompress these archives:
  	$ tar xvfz gliss.tgz
  	$ tar xvfz ppc.tgz

  Add the option line to the Makefile of ppc directory:
  		OPT=-DEMUL_DISASM
Or if you use the version 1.6:
		OPT=-DISS_DISASM
  
  And compile them:
  	$ cd gliss
  	$ make all
  	$ cd ..
  	$ cd ppc
  	$ make all
 
 And rename the "ppc" directory to "gliss-ppc":
 	$ mv ppc gliss-ppc


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
