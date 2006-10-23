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
#include <otawa/exegraph/ExeGraphBBTime.h>
#include <otawa/exegraph/Microprocessor.h>

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
	FrameWork *fw;
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
BoolOption dump_constraints(command, 'c', "dump-cons",
	"dump lp_solve constraints", false);
BoolOption verbose(command, 'v', "verbose", "verbose mode", false);
BoolOption exegraph(command, 'E', "exegraph", "use exegraph method", false);
IntOption delta(command, 'D', "delta", "use delta method with given sequence length", "length", 0);
IntOption degree(command, 'd', "degree", "superscalar degree power (real degree = 2^power)", "degree", 1);
StringOption proc(command, 'p', "processor", "used processor", "processor", "deg1.xml");
BoolOption do_stats(command, 's', "stats", "display statistics", false);
 

/**
 * Build the command manager.
 */
Command::Command(void) {
	program = "piconsens";
	version = "1.0.0";
	author = "H. Cassé";
	copyright = "Copyright (c) 2006, IRIT-UPS";
	description = "pipeline context-sensitivity experimentation.";
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
	ENTRY_CFG(fw) = &vcfg;
		
	// Prepare processor configuration
	PropList props;
	if(verbose) {
		PROC_VERBOSE(props) = true;
		cerr << "verbose !\n";
	}
	if(dump_constraints)
		props.set(EXPLICIT, true);
	if(do_stats)
		PROC_STATS(props) = &stats;
	
	// Compute BB time
	if(exegraph && !delta) {
		ExeGraphBBTime tbt(props);
		tbt.process(fw);
	}
	else {
		BBTimeSimulator bbts(props);
		bbts.process(fw);
	}
	
	// Assign variables
	VarAssignment assign(props);
	assign.process(fw);
		
	// Build the system
	BasicConstraintsBuilder builder(props);
	builder.process(fw);
		
	// Load flow facts
	ipet::FlowFactLoader loader(props);
	loader.process(fw);

	// Build the object function to maximize
	BasicObjectFunctionBuilder fun_builder;
	fun_builder.process(fw);	

	// Use delta approach
	if(delta) {
		Delta::LEVELS(props) = delta;
		Delta delta(props);
		delta.process(fw);
	}
	
	// Resolve the system
	WCETComputation wcomp(props);
	wcomp.process(fw);

	// Get the result
	ilp::System *sys = vcfg.use<ilp::System *>(SYSTEM);
	if(!do_stats) 
		cout << WCET(vcfg);
	else {
		const Vector<ExeGraphBBTime::stat_t>& prefs = *EXEGRAPH_PREFIX_STATS(stats);
		for(int i = 0; i < prefs.length(); i++)
			cout << i << '\t'
				/*<< prefs[i].total_span_sum << '\t'
				 << prefs[i].total_vals_sum << '\t'
				 << prefs[i].bb_cnt << '\t'*/
				 << ((double)prefs[i].total_span_sum / prefs[i].bb_cnt) << '\t'
				 << ((double)prefs[i].total_vals_sum / prefs[i].bb_cnt) << io::endl;
	}	

	// Dump the ILP system
	if(dump_constraints) {
		String out_file = fun + ".lp";
		io::OutFileStream stream(&out_file);
		if(!stream.isReady())
			throw MessageException("cannot create file \"%s\".", &out_file);
		sys->dump(stream);
	}
}


/**
 * Launch the work.
 */
void Command::run(void) {
	
	// Any file
	if(!file)
		throw OptionException("binary file path required !");
	
	// Load the file
	GenericSimulator sim;
	PropList props;
	SIMULATOR(props) = &sim;
	DEGREE(props) = degree;
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
