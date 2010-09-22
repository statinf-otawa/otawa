# Configuration
MODULES 			= 	@MODULES@
PROGRAM 			=	@PROJECT@
SOURCES 			=
ELF					=
CUSTOM_CXXFLAGS		=
CUSTOM_LDFLAGS		=
CUSTOM_LDLIBS		=
CUSTOM_CLEAN		=

# Tools
CXX			= libtool --tag=CXX --mode=compile g++
LD			= libtool --tag=CXX --mode=link g++  -module
INSTALL		= libtool --tag=CXX --mode=install install
CONFIG		= otawa-config

# Internals
PREFIX		= $(shell $(CONFIG) --prefix)
INSTALL_DIR = $(PREFIX)/lib/otawa/loader
INSTALL_LIB = $(INSTALL_DIR)/$(PROGRAM)
INSTALL_LINK= $(INSTALL_DIR)/elf_$(ELF).la
FLAGS		= $(shell $(CONFIG) --cflags $(MODULES))
DATADIR 	= $(shell $(CONFIG) --data $(MODULES))
CXXFLAGS	= $(CUSTOM_CXXFLAGS) $(FLAGS) -DDATA_DIR="\"$(DATADIR)\""
LDLIBS		= $(CUSTOM_LDLIBS) $(shell $(CONFIG) --libs $(MODULES))
LDFLAGS 	= $(CUSTOM_LDFLAGS)  -rpath $(INSTALL_DIR) -Wl,-rpath=\$$ORIGIN/../..
OBJECTS 	= $(SOURCES:.cpp=.lo)
DEPS		= $(addprefix .deps/,$(SOURCES:.cpp=.d))
CLEAN		= $(CUSTOM_CLEAN) $(PROGRAM) $(OBJECTS) .deps

# Rules
all: .deps $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(LD) -o $@ $^ $(LDFLAGS) $(LDLIBS)

install: all
	$(INSTALL) -D $(PROGRAM) $(INSTALL_LIB)
	test -e $(INSTALL_LINK) || ln -s $(PROGRAM) $(INSTALL_LINK)

test:
	$(LD) -rpath $(PWD)/.otawa/loader -o $@ $^ $(LDFLAGS) $(LDLIBS)

clean:
	rm -rf $(CLEAN)

.deps:
	mkdir .deps

%.lo: %.cpp
	@$(CXX) $(CXXFLAGS) -MM -MF .deps/$*.d -c $<
	$(CXX) $(CXXFLAGS) -c $< -o $@ 

-include $(DEPS)
