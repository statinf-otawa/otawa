/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	src/oipet/oipet.cpp -- oipet command.
 */

#include <stdlib.h>
#include <elm/io.h>
#include <elm/io/OutFileStream.h>
#include <elm/options.h>
#include <otawa/otawa.h>
#include <otawa/ipet.h>
#include <otawa/ilp.h>
#include <elm/system/StopWatch.h>
#include <otawa/proc/ProcessorException.h>
#include <otawa/hard/CacheConfiguration.h>

using namespace elm;
using namespace elm::option;
using namespace otawa;
using namespace otawa::ipet;
using namespace otawa::hard;
using namespace elm::option;


/*
 * @page oipet oipet Command
 * 
 * oipet allows to use WCET IPET computation facilities of OTAWA. Currently,
 * you may only choose the algorithm for instruction cache support:
 * 
 * @li ccg for Cache Conflict Graph from  Li, Malik, Wolfe, Efficient
 * microarchitecture modelling and path analysis for real-time software,
 * Proceedings of the 16th IEEE Real-Time Systems Symposium, 1995.
 * 
 * @li cat for categorization approach (an adaptation to IPET of Healy, Arnold, 
 * Mueller, Whalley, Harmon, Bounding pipeline and instruction cache performance,
 * IEEE Trans. Computers, 1999.
 * 
 * @par Syntax
 * 
 * @code
 * $ oipet [options] binary_file [function1 function2 ...]
 * @endcode
 * This command compute the WCET of the given function using OTAWA IPET facilities.
 * If no function is given, the main() function is used.
 * 
 * @par options
 * 
 * -i method, --icache=method: selects how the instruction must be managed.
 * The methods includes:
 * @li none: ignore instruction cache or the hardware does not have an
 * instruction cache.
 * @li ccg: use CCG algorithm (default),
 * @li cat: use CAT algorithm.
 * 
 * -, --dump-constraints: for each function, generate a file named
 * function_name.lp containing the generated ILP system. The generated file
 * use the lp_solve syntax and may feed the lp_solve command.
 */

// Options
class Command: public elm::option::Manager {
	String file;
	genstruct::Vector<String> funs;
	otawa::Manager manager;
	FrameWork *fw;
public:
	Command(void);
	void compute(String fun);
	void run(void);
	
	// Manager overload
	virtual void process (String arg);
};
Command command;


// Instruction cache management
typedef enum icache_t {
	icache_def = 0,
	icache_none,
	icache_ccg,
	icache_cat
} icache_t;
EnumOption<int>::value_t icache_values[] = {
	{ "method", icache_def },
	{ "none", icache_none },
	{ "ccg", icache_ccg },
	{ "cat", icache_cat }
};
EnumOption<int> icache_option(command, 'i', "icache",
	"instruction management method", icache_values);


// Basic-block Timing options
typedef enum bbtime_t {
	bbtime_sim,
	bbtime_delta,
	bbtime_exegraph
} bbtime_t;
EnumOption<int>::value_t bbtime_values[] = {
	{ "method", bbtime_sim },
	{ "sim", bbtime_sim },
	{ "delta", bbtime_delta },
	{ "exegraph", bbtime_exegraph }
};
EnumOption<int> bbtime_option(command, 't', "bbtiming",
	"basic block timing method", bbtime_values);


// Dump options
BoolOption dump_constraints(command, 'c', "dump-constraints",
	"dump lp_solve constraints", false);
BoolOption dump_graph(command, 'g', "dump-graph",
	"dump DOT graph of the processed function", false);


// Other options
BoolOption verbose(command, 'v', "verbose", "verbose mode", false);


/**
 * Build the command manager.
 */
Command::Command(void) {
	program = "oipet";
	version = "1.0.0";
	author = "H. Cassé";
	copyright = "Copyright (c) 2006, IRIT-UPS";
	description = "Compute the WCET of some tasks of a binary using IPET techniques.";
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
		
	// Prepare processor configuration
	PropList props;
	if(dump_constraints || dump_graph)
		props.set(EXPLICIT, true);
	if(verbose) {
		PROC_VERBOSE(props) = true;
		cerr << "verbose !\n";
	}
	
	// Compute BB times
	TrivialBBTime tbt(5, props);
	tbt.processCFG(fw, &vcfg);
		
	// Trivial data cache
	TrivialDataCacheManager dcache(props);
	dcache.processCFG(fw, &vcfg);
		
	// Assign variables
	VarAssignment assign(props);
	assign.processCFG(fw, &vcfg);
		
	// Build the system
	BasicConstraintsBuilder builder(props);
	builder.processCFG(fw, &vcfg);
		
	// Process the instruction cache
	switch(icache_option) {
	case icache_ccg:
		{	
			// build ccg graph
			CCGBuilder ccgbuilder;
			ccgbuilder.processCFG(fw, &vcfg );
			
			// Build ccg contraint
			CCGConstraintBuilder decomp(fw);
			decomp.processCFG(fw, &vcfg );
			
			//Build the objectfunction
			CCGObjectFunction ofunction(fw);
			ofunction.processCFG(fw, &vcfg );
		}
		break;
		
	case icache_cat:
		{
			// build Cat lblocks
			CATBuilder catbuilder;
			catbuilder.processCFG(fw, &vcfg);
			
			// Build CAT contraint
			CATConstraintBuilder decomp;
			decomp.processCFG(fw, &vcfg);
		}
	
	case icache_none:
		{
			// Build the object function to maximize
			BasicObjectFunctionBuilder fun_builder;
			fun_builder.processCFG(fw, &vcfg);	
		}
		break;
	}

	// Load flow facts
	ipet::FlowFactLoader loader(props);
	loader.processCFG(fw, &vcfg);
		
	// Resolve the system
	WCETComputation wcomp(props);
	wcomp.processCFG(fw, &vcfg);

	// Get the result
	ilp::System *sys = vcfg.use<ilp::System *>(SYSTEM);
	cout << "WCET [" << file << ":" << fun << "] = "
		 << vcfg.use<int>(WCET) << io::endl;
	
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
	PropList props;
	props.set<Loader *>(Loader::ID_Loader, &Loader::LOADER_Gliss_PowerPC);
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
	
	// Instruction cache preparation
	if(icache_option == icache_def) {
		if(fw->platform()->cache().instCache())
			icache_option = icache_ccg;
		else
			icache_option = icache_none;
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
		return 2;
	}
}

