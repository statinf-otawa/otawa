/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	src/oipet/piconsens.cpp -- pipeline context-sensitivity experimentation.
 */

#include <errno.h>
#include <stdlib.h>
#include <elm/io.h>
#include <elm/io/OutFileStream.h>
#include <elm/options.h>
#include <otawa/otawa.h>
#include <otawa/ipet.h>
#include <otawa/ilp.h>
#include <elm/system/StopWatch.h>
#include <otawa/proc/ProcessorException.h>
#include <otawa/ipet/BBTimeSimulator.h>
#include <otawa/gensim/GenericSimulator.h>
#include <otawa/exegraph/LiExeGraphBBTime.h>
#include <otawa/exegraph/Microprocessor.h>
#include <otawa/ipet/TimeDeltaObjectFunctionModifier.h>

using namespace elm;
using namespace elm::option;
using namespace otawa;
using namespace otawa::ipet;
using namespace otawa::hard;
using namespace elm::option;
using namespace otawa::gensim;

// Command
class Command: public elm::option::Manager {
	String file;
	genstruct::Vector<String> funs;
	otawa::Manager manager;
	WorkSpace *fw;
	PropList stats;


public:
	Command(void);
	void compute(String fun);
	void run(void);
	
	// Manager overload
	virtual void process (String arg);
};
Command command;


// Options
BoolOption dump_constraints(command, 'u', "dump-cons", "dump lp_solve constraints", false);
BoolOption verbose(command, 'v', "verbose", "verbose mode", false);
StringOption proc(command, 'p', "processor", "used processor", "processor", "deg1.xml");
BoolOption do_time(command, 't', "time", "display basic block times", false);
IntOption depth(command, 'D', "depth", "prologue depth", "depth", 0);


/**
 * Build the command manager.
 */
Command::Command(void) {
	program = "genexegraph";
	version = "1.0.0";
	author = "H. Cassé";
	copyright = "Copyright (c) 2006, IRIT-UPS";
	description = "test of generic exegraph";
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
	
	// Get the VCFG
	CFG *cfg = fw->getCFGInfo()->findCFG(fun);
	if(!cfg) {
		cerr << "ERROR: binary file does not contain the function \""
			 << fun << "\".\n";
		return;
	}
	VirtualCFG vcfg(cfg);
	//ENTRY_CFG(fw) = &vcfg;
		
	// Prepare processor configuration
	PropList props;
	ENTRY_CFG(props) = &vcfg;
	if(verbose) {
	  //	PROC_VERBOSE(props) = true;
		cerr << "verbose !\n";
	}
	if(dump_constraints)
		props.set(EXPLICIT, true);
	
	// Assign variables
	VarAssignment assign;
	assign.process(fw, props);
		
	// Compute BB time
       	LiExeGraphBBTime tbt(props);
	tbt.process(fw);
		
	// Build the system
	BasicConstraintsBuilder builder;
	builder.process(fw, props);
		
	// Load flow facts
	ipet::FlowFactLoader loader;
	loader.process(fw, props);

	// Build the object function to maximize
	BasicObjectFunctionBuilder fun_builder;
	fun_builder.process(fw, props);	
  
	// Resolve the system
	WCETComputation wcomp;
	wcomp.process(fw, props);
  
	// Get the results
	ilp::System *sys = SYSTEM(fw);
	cout << "WCET = " << WCET(fw) << "\n";
	// Dump the ILP system
	if(dump_constraints) {
	  String out_file = fun + ".lp";
		io::OutFileStream stream(&out_file);
//		if(!stream.isReady())
//			throw MessageException("cannot create file \"%s\".", &out_file);
		sys->dump(stream);
	}
}


/**
 * Launch the work.
 */
void Command::run(void) {
	
	// Any file
//	if(!file)
//		throw OptionException("binary file path required !");
	
	// Load the file
	//GenericSimulator sim;
	PropList props;
	//SIMULATOR(props) = &sim;
	PROCESSOR_PATH(props) = proc.value();
	fw = manager.load(&file, props);
	
	// Removing __eabi call if available (should move in a file configuration)
	CFG *cfg = fw->getCFGInfo()->findCFG("main");
	if(cfg != 0)
		for(CFG::BBIterator bb(cfg); bb; bb++)
			for(BasicBlock::OutIterator edge(bb); edge; edge++)
				if(edge->kind() == Edge::CALL
				&& edge->target()
				&& edge->calledCFG()->label() == "__eabi") {
					delete(*edge);
					break;
				}
	
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
		cerr << "ERROR: " << e.message() << io::endl;
		command.displayHelp();
		return 1;
	}
	catch(elm::Exception& e) {
		cerr << "ERROR: " << e.message() << io::endl;
		cerr << strerror(errno) << io::endl;
		return 2;
	}
}
