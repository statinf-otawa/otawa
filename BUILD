BUILD NOTES
-----------

  This file describes resources coming from thrid-party development
that are not stored under CVS and that needs to be available for building
otawa.


1. Compiling Otawa

  Go in otawa and prepare the configuration:
  	> cd otawa
	> autoheader
	> aclocal
	> autoconf
	> automake --add-missing
	> ./configure <options>
	> make

  Type "configure --help" for the list of configuration option. Otawa specific
configuration options are:

	--with-elm: assume the Elm directory is at "otawa/elm".
	--with-elm=PATH: use the Elm library in the given PATH.

	--with-glissppc: use the Gliss PPC module (default) and look for the
		GlissPPC package in "otawa/gliss-ppc".
	--with-glissppc=PATH: allow specifiying the path to the Gliss PPC package.
	--without-glissppc: do not use the Gliss PPC module.
	
	--enable-heptane: use the Heptane AST loader (default, require glissppc).
	--disable-heptane: do not use the Heptane AST loader.
	
	--enable-doc: enable the automatic documentation generation
		(require doxygen).
	--disable-doc: disable the automatic documentation generation.


2. Compiling Elm Library

  I have not yet archived because I do not know if it must be inserted in the
otawa CVS archive. For using, you may get it from "~casse/elm.tgz" and
then uncompress it. Then, you may compile it.
	> tar xvfz ~casse/elm.tgz
	> cd elm
	> ./configure
	> make
	
  As a default behaviour, Otawa assumes elm directory is located in the Otawa top
level directory but with "--with-elm" configuration option, you may put it
anywhere.


3. Compiling Gliss PPC Package

You must uncompress the GLISS PPC archive 'ppc.tgz' the 'gliss.tgz' at the same
level. Then perform:
	> tar xvfz PATH_TO_ARCHIVE/gliss.tgz
	> tar xvfz PATH_TO_ARCHIVE/ppc.tgz
	
  Then, you have to compile both packages
  	> cd gliss
	> make
	> cd ../ppc
	
  Add the option line to the Makefile:
  		OPT=-DEMUL_DISASM
Or if you use the version 1.6:
		OPT=-DISS_DISASM
	
Then you can build the PCC simulator:	
	> make all
	
  As a default behaviour, Otawa assumes Gliss PPC directory is called "gliss-ppc"
and is located in the Otawa top level directory but with "--with-glissppc"
configuration option, you may put it anywhere.
