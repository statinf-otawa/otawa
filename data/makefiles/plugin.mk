# Configuration
MODULES 			= 	@MODULES@
PROGRAM 			=	@PROJECT@
SOURCES 			=
CUSTOM_CXXFLAGS		=
CUSTOM_LDFLAGS		=
CUSTOM_LDLIBS		=
CUSTOM_CLEAN		=

# Tools
KIND		= 			# one of loader, ilp, proc/path
CXX			= libtool --tag=CXX --mode=compile g++
LD			= libtool --tag=CXX --mode=link g++  -module
CONFIG		= otawa-config
INSTALL_DIR =	$(otawa-config --prefix)/lib/otawa/$(KIND)

# Internals
PREFIX		= $(shell $(CONFIG) --prefix)
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
	$(LD) -rpath $(PWD)/.otawa/loader -o $@ $^ $(LDFLAGS) $(LDLIBS)

install: $(OBJECTS)
	$(LD) -rpath $(PREFIX)/lib/otawa/loader -o $@ $^ $(LDFLAGS) $(LDLIBS)

clean:
	rm -rf $(CLEAN)

.deps:
	mkdir .deps

%.lo: %.cpp
	@$(CXX) $(CXXFLAGS) -MM -MF .deps/$*.d -c $<
	$(CXX) $(CXXFLAGS) -c $< -o $@ 

-include $(DEPS)
