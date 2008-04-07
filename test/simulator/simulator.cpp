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

using namespace elm;
using namespace elm::io;
using namespace otawa;
using namespace otawa::ipet;
using namespace std;

// Verbose flag
bool my_verbose = false;

// MyDriver class
class MyDriver: public sim::Driver {
public:
	
	MyDriver(SimState& _state, Inst *start, Inst *_exit)
	: state(_state), next(start), exit(_exit) {	 }
	
	virtual Inst *nextInstruction(sim::State &_state, Inst *inst) {
		Inst *res = next;
		if(next) {
			if(my_verbose)
				cerr << "executing " << next->address() << ": " << next << io::endl;
			next = state.execute(next);
			if(next == exit)
				next = 0;
		}
		return res;
	}
	
	virtual void terminateInstruction (sim::State &_state, Inst *inst) { }
	virtual void redirect (sim::State &_state, Inst *branch, bool direction) { }
	virtual bool PredictBranch (sim::State &_state, Inst *branch, bool pred) { }

private:
	SimState& state;
	Inst *next, *exit;
};

	
int main(int argc, char **argv) {

	PropList props;
	String processor;
	String cache;
	
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
			cerr << "_start = " << start_inst->address() << io::endl;
			cerr << "_exit = " << exit_inst->address() << io::endl;
		}
		Inst *main_inst = ws->process()->findInstAt(Address(0x100004c8));
		
		// Build the functional state
		SimState *state = ws->process()->newState();
		
		// Full simulation
		if(processor) {
			MyDriver driver(*state, start_inst, exit_inst);
			gensim::GenericSimulator simulator;
			sim::State *simulator_state = simulator.instantiate(ws, props);
			simulator_state->run(driver);
		}
		
		// Functional simulation only
		else {
			Inst *inst = start_inst;
			while(inst != exit_inst) {
				if(inst == main_inst) {
					cerr << "found !\n";
				}
				if(my_verbose)
					cerr << "executing " << inst->address() << ": " << inst << io::endl;
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

