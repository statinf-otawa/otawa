BUILD NOTES
-----------

  This file describes resources coming from thrid-party development
that are not stored under CVS and that needs to be available for building
otawa.

(1) GLISS DEPENDENCY
  As Otawa depends upon gliss-ppc, you must uncompress the GLISS PPC archive
'ppc.tgz' in the root directory of otawa and rename it 'gliss-ppc'.
As the GLISS PPC requires gliss on the upper level, you must also uncompress GLISS 1.1.6
'gliss.tgz' on the upper directory.
	> cd otawa
	> tar xvfz PATH_TO_ARCHIVE/gliss.tgz
	> tar xvfz PATH_TO_ARCHIVE/ppc.tgz
	> mv ppc gliss-ppc
  Then, you have to compile both packages
  	> cd gliss
	> make
	> cd ../gliss-ppc
	> make all

NOTE: a bug in GLISS prevents actually from linking with Otawa.
Fixed versions of GLISS and GLISS-PPC are available from "~casse".
	> cd otawa
	> tar xvfz ~casse/gliss.tgz
	> tar xvfz ~casse/gliss-ppc.tgz
  	> cd gliss
	> make
	> cd ../gliss-ppc
	> make all

(2) ELM DEPENDENCY
  I have not yet archived because I do not know if it must be inserted in the
otawa CVS archive. For using, you may get it from "~casse/elm.tgz" and
then uncompress it in otawa root directory. Then, you may compile it.
	> tar xvfz ~casse/elm.tgz
	> cd elm
	> ./configure
	> make

(3) WORKING IN OTAWA
  Go in otawa and prepare the configuration:
  	> cd otawa
	> autoheader
	> aclocal
	> autoconf
	> automake --add-missing
	> ./configure
Then you may compile it:
	> make

(4) BUILDING THE DOCUMENTATION
  Go in otawa and call Doxygen.
  	> cd otawa
	> doxygen
