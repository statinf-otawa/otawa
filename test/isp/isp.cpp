/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	src/oipet/piconsens.cpp -- pipeline context-sensitivity experimentation.
 */

#include <errno.h>
#include <elm/io.h>
#include <elm/io/OutFileStream.h>
#include <elm/options.h>
#include <otawa/otawa.h>
#include <otawa/ipet.h>
#include <otawa/ilp.h>
#include <otawa/prog/WorkSpace.h>
#include <elm/system/StopWatch.h>
#include <otawa/proc/ProcessorException.h>
#include <otawa/cfg.h>
#include <otawa/cache/cat2/CAT2Builder.h>
#include <otawa/cfg/CFGCollector.h>
#include <otawa/ipet/BBTimeSimulator.h>
#include <otawa/exegraph/ParamExeGraphBBTime.h>
#include <otawa/exegraph/Microprocessor.h>
#include <otawa/ipet/TimeDeltaObjectFunctionModifier.h>
#include <otawa/ipet/WCETCountRecorder.h>
#include <otawa/cfg/Virtualizer.h>

#include "CFGSizeComputer.h"
#include "FunctionBlockBuilder.h"
#include "ISPCATBuilder.h"
#include "ISPConstraintBuilder.h"

#include "ISPProblem.h" // pour le test

using namespace elm;
using namespace elm::option;
using namespace otawa;
using namespace otawa::ipet;
using namespace otawa::hard;
using namespace elm::option;


// Command
class Command: public elm::option::Manager {
  String file;
  genstruct::Vector<String> funs;
  otawa::Manager manager;
  WorkSpace *ws;
  PropList props;
  
  
public:
  Command(void);
  void compute(String fun);
  void run(void);
  
  // Manager overload
  virtual void process (String arg);
};
Command command;


// Options
StringOption proc(command, 'p', "processor", "used processor", "processor", "");
StringOption graphs(command, 'g', "graphs", "graphs directory", "graphs_dir", "");
IntOption isp_size(command,'s',"isp_size","size of the ISP", "ispsize", 2048);
StringOption icache(command, 'c', "icache", "used icache", "icache", "");
BoolOption dump(command, 'd', "dump ilp system", false);

/**
 * Build the command manager.
 */
Command::Command(void) {
  program = "isp";
  version = "1.0.0";
  author = "Christine Rochange";
  copyright = "Copyright (c) 2009, IRIT-Universite de Toulouse";
  description = "analysis of the instruction scratchpad for the MERASA project";
  free_argument_description = "binary_file function1 function2 ...";
}


/**
 * Process the free arguments.
 */
void Command::process (String arg) {
  if(!file)
    file = arg;
  else
    funs.add(arg);
}


/**
 * Compute the WCET for the given function.
 */
void Command::compute(String fun) {
  elm::cerr << "function: " << fun << io::endl;
  
  // Prepare processor configuration
  TASK_ENTRY(props) = fun.toCString();

  
  // Assign variables
  VarAssignment assign;
  assign.process(ws, props);
  
  LoopInfoBuilder lb;
  lb.process(ws, props);

  // virtual CFG
  Virtualizer virt;
  virt.process(ws, props);

  if (icache){
    CAT2Builder catbuilder;
    catbuilder.process(ws, props);
  }
  else { //scratchpad
    ISP_SIZE(props) = isp_size.value();
    FunctionBlockBuilder fbb;
    fbb.process(ws,props);
    ISPCATBuilder cat;
    cat.process(ws,props);

#ifdef TRACE_FOR_CHECKING
    for (CFGCollection::Iterator cfg(INVOLVED_CFGS(ws)); cfg; cfg++) {
      for (CFG::BBIterator bb(cfg); bb ; bb++) {
	if (!bb->isEnd() && bb->size()){
	  FunctionBlock *fb = FUNCTION_BLOCK(bb);
	  if (fb) {
	    elm::cout << "Function block found for bb" << bb->number() << " with cfg\"" << fb->cfg()->label() << "\", size=" << CFG_SIZE(fb->cfg()) << "\n";
	    elm::cout << "\t category = ";
	    switch(ISP_CATEGORY(bb)){
	    case ISP_ALWAYS_HIT:
	      elm::cout << "ALWAYS_HIT\n";
	      break;
	    case ISP_ALWAYS_MISS:
	      elm::cout << "ALWAYS_MISS\n";
	      break;
	    case ISP_NOT_CLASSIFIED:
	      elm::cout << "NOT_CLASSIFIED\n";
	      break;
	    case ISP_PERSISTENT:
	      elm::cout << "PERSISTENT (header=bb" << ISP_HEADER(bb)->number() << ")\n";
	      break;
	    default:
	      elm::cout << "unknown\n";
	    }
	  }
	}
      }
    }
#endif
  }
   
  ParamExeGraphBBTime tbt(props,0);
  tbt.process(ws);
		
  // Build the system
  EXPLICIT(props) = true;

  BasicConstraintsBuilder builder;
  builder.process(ws, props);
  
  // Load flow facts
  ipet::FlowFactLoader loader;
  loader.process(ws, props);
  
  // Build the object function to maximize
  BasicObjectFunctionBuilder fun_builder;
  fun_builder.process(ws, props);

  if (icache){
    CAT2ConstraintBuilder cache_constraint_builder;
    cache_constraint_builder.process(ws, props);
  }
  else { // scratchpad
    RAM_PENALTY(props) = 5;
    ISPConstraintBuilder isp_constraint_builder;
    isp_constraint_builder.process(ws, props);
  }
  
  // Resolve the system
  WCETComputation wcomp;
  wcomp.process(ws, props);
  
  // Get the results
  ilp::System *sys = SYSTEM(ws);
  elm::cout << "WCET = " << WCET(ws) << "\n";
  if (dump)
    sys->dump();
  
}


/**
 * Launch the work.
 */
void Command::run(void) {
	
  // Load the file
  PROCESSOR_PATH(props) = proc.value();

  if (icache){
    CACHE_CONFIG_PATH(props) = elm::system::Path(icache.value());
  }

  ws = manager.load(&file, props);
    
  // Now process the functions
  if(!funs)
    compute("main");
  else
    for(int i = 0; i < funs.length(); i++)
      compute(funs[i]);
}


/**
 * Program entry point.
 */
int main(int argc, char **argv) {
  try {
    command.parse(argc, argv);
    command.run();
    return 0;
  }
  catch(OptionException& e) {
    elm::cerr << "ERROR: " << e.message() << io::endl;
    command.displayHelp();
    return 1;
  }
  catch(elm::Exception& e) {
    elm::cerr << "ERROR: " << e.message() << io::endl;
    elm::cerr << strerror(errno) << io::endl;
    return 2;
  }
}
