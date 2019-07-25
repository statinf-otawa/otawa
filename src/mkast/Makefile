OCAMLMAKEFILE = ../OCamlMakefile

SOURCES = mkast.ml
RESULT = mkast
INCDIRS = /home/casse/frontc/frontc
LIBDIRS = /home/casse/frontc/frontc
LIBS = unix frontc

include $(OCAMLMAKEFILE)

install:
	install -c $(RESULT)$(EXEEXT) $(DESTDIR)$(bindir)/$(RESULT)$(EXEEXT)
