/*
 *	$Id$
 *	Copyright (c) 2006-08, IRIT UPS.
 *
 *	test/ipet/exegraph.cpp -- test for Execution Graph modeling feature.
 */

#include <elm/io.h>
#include <otawa/otawa.h>
#include <otawa/ipet.h>
#include <otawa/proc/ProcessorException.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/ilp.h>
#include <otawa/sim/BasicBlockDriver.h>
#include <otawa/gensim/GenericSimulator.h>
#include <otawa/sim/Driver.h>
#include <otawa/gensim/GenericState.h>

using namespace elm;
using namespace elm::io;
using namespace otawa;
using namespace otawa::ipet;
using namespace std;

// Verbose flag
bool my_verbose = false;
bool my_trace = false;

// MyDriver class
class MyDriver: public sim::Driver {
public:
	
  MyDriver(SimState& _state, Inst *start, Inst *_exit)
    : state(_state), next(start), exit(_exit) {	 }
	
  virtual Inst *nextInstruction(sim::State &_state, Inst *inst) {
    Inst *res = next;
    if(next) {
      if(my_verbose)
	elm::cerr << "executing " << next->address() << ": " << next << io::endl;
      next = state.execute(next);
      if(next == exit)
	next = 0;
    }
    return res;
  }
	
  virtual void terminateInstruction (sim::State &_state, Inst *inst) { }
  virtual void redirect (sim::State &_state, Inst *branch, bool direction) { }
  virtual bool PredictBranch (sim::State &_state, Inst *branch, bool pred) { }

	// memory accesses
	virtual Address lowerRead(void) { return state.lowerRead(); }
	virtual Address upperRead(void) { return state.upperRead(); }
	virtual Address lowerWrite(void) { return state.lowerWrite(); }
	virtual Address upperWrite(void) { return state.upperWrite(); }

private:
  SimState& state;
  Inst *next, *exit;
};

	
int main(int argc, char **argv) {

  PropList props;
  String processor;
  String cache;
  String tracefile;
	
  // Scan the arguments
  int i = 1;
  while(i < argc) {
    if(CString(argv[i]) == "-p") {
      i++;
      if(i >= argc)
	throw otawa::Exception("no argument after -p");
      processor = argv[i++]; 
    }
    if(CString(argv[i]) == "-c") {
      i++;
      if(i >= argc)
	throw otawa::Exception("no argument after -c");
      cache = argv[i++]; 
    }
    else if(CString(argv[i]) == "-v") {
      i++;
      my_verbose = true;
    }
    else if(CString(argv[i]) == "-t") {
      i++;
      my_trace = true;
      if(i >= argc)
	throw otawa::Exception("no argument after -t");
      tracefile = argv[i++]; 
    }
    else if(CString(argv[i]) == "-h") {
      elm::cout << "USAGE: ./simulator [-vh] [-p PROCESSOR.xml]"
 	" [-c CACHE.xml] BINARY ARGUMENTS...\n";
      elm::cout << "\t-c CACHE.xml -- use the given cache description with the structural simulator\n";
      elm::cout << "\t-h -- display this help\n";
      elm::cout << "\t-p PROCESSOR.xml -- perform structural simulation with the given processor\n";
      elm::cout << "\t-v -- display executed instructions\n";
      elm::cout << "\t-t <filename> -- output simulation trace to <filename>\n";
      return 0;
    }
    else
      break;
  }
  if(i >= argc)
    throw otawa::Exception("no executable given !");
	
  ARGC(props) = argc - i;
  ARGV(props) = argv + i;
  try {
		
    // Configure the process if any
    if(processor) {
      PROCESSOR_PATH(props) = processor;
      if(cache)
	CACHE_CONFIG_PATH(props) = cache;
    }
    if (tracefile) {
      gensim::TRACE_FILE_PATH(props) = tracefile;
      gensim::TRACE_LEVEL(props) = 9;
    }
	
    otawa::gensim::ICACHE(props) = &(sim::CacheDriver::ALWAYS_HIT);
    otawa::gensim::DCACHE(props) = &(sim::CacheDriver::ALWAYS_HIT);
    // Load program
    Processor::VERBOSE(props) = my_verbose;
    WorkSpace *ws = MANAGER.load(argv[i], props);
		
    // Find interesting instruction
    Inst *start_inst = ws->process()->findInstAt("_start");
    if(!start_inst)
      throw otawa::Exception("no _start label !");
    Inst *exit_inst = ws->process()->findInstAt("_exit");
    if(!exit_inst)
      throw otawa::Exception("no _exit label !");
    if(my_verbose) {
      elm::cerr << "_start = " << start_inst->address() << io::endl;
      elm::cerr << "_exit = " << exit_inst->address() << io::endl;
    }
    Inst *main_inst = ws->process()->findInstAt(Address(0x100004c8));
		
    // Build the functional state
    SimState *state = ws->process()->newState();
		
    // Full simulation
    if(processor) {
      MyDriver driver(*state, start_inst, exit_inst);
      gensim::GenericSimulator simulator;
      gensim::GenericState *simulator_state = (gensim::GenericState *) simulator.instantiate(ws, props);
      //sim::State *simulator_state = simulator.instantiate(ws, props);
      simulator_state->run(driver);
      simulator_state->dumpStats(elm::cout);
    }
		
    // Functional simulation only
    else {
      Inst *inst = start_inst;
      while(inst != exit_inst) {
	if(inst == main_inst) {
	  elm::cerr << "found !\n";
	}
	if(my_verbose)
	  elm::cerr << "executing " << inst->address() << ": " << inst << io::endl;
	inst = state->execute(inst);
      }
    }
  }

  catch(elm::Exception& e) {
    elm::cerr << "ERROR: " << e.message() << '\n';
    return 1;
  }
  return 0;

}

