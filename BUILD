BUILD NOTES
-----------

  This file describes not stored under CVS coming from thrid-party development
that needs to be available in the otawa source tree.

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

(2) ELM
  I have not yet archived because I do not know if it must be inserted in the
otawa CVS archive. For using, you may get it from "~casse/elm.tgz" and
then uncompress it in otawa root directory. Then, you may compile it.
	> tar xvfz ~casse/Elm-0.1.tar.gz
	> mv Elm-0.1 elm
	> cd elm
	> ./configure
	> make
