# Configuration
MODULES 			= @MODULES@
PROGRAM 			= @PROJECT@
SOURCES 			=
CUSTOM_CXXFLAGS		=
CUSTOM_LDFLAGS		=
CUSTOM_LDLIBS		=
CUSTOM_CLEAN		=

# Tools
CXX		= g++
LD		= libtool --mode=link --tag=CXX g++
CONFIG	= otawa-config

# Internal
FLAGS		= $(shell $(CONFIG) --cflags $(MODULES))
DATADIR 	= $(shell $(CONFIG) --data $(MODULES))
CXXFLAGS	= $(CUSTOM_CXXFLAGS) $(FLAGS) -DDATA_DIR="\"$(DATADIR)\""
LDFLAGS 	= $(CUSTOM_LDFLAGS)
LDLIBS		= $(CUSTOM_LDLIBS) $(shell $(CONFIG) --libs $(MODULES))
OBJECTS 	= $(SOURCES:.cpp=.o)
DEPS		= $(addprefix .deps/,$(SOURCES:.cpp=.d))
CLEAN		= $(CUSTOM_CLEAN) $(PROGRAM) $(OBJECTS) .deps

# Rules
all: .deps $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(LD) -o $@ $^ $(LDFLAGS) $(LDLIBS)
	
clean:
	rm -rf $(CLEAN)

.deps:
	mkdir .deps

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ 
	@$(CXX) $(CXXFLAGS) -MM -MF .deps/$*.d -c $<

-include $(DEPS)
