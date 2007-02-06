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
#include <otawa/ipet/BBTimeSimulator.h>
#include <otawa/exegraph/ExeGraphBBTime.h>
#include <otawa/gensim/GenericSimulator.h>
#include <otawa/util/LBlockBuilder.h>

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
 * And the algorithm to handle the pipeline:
 * 
 * @li trivial : consider a scalar processor without pipeline with 5 cycle
 * per instruction,
 * 
 * @li sim : use a simulator to time the program blocks,
 * 
 * @li delta : use the simulator and the delta approach to take in account
 * the inter-block effects as in J. Engblom, A. Ermedahl, M. Sjoedin, 
 * J. Gustafsson, H. Hansson, "Worst-case execution-time analysis for embedded
 * real-time systems", Journal of Software Tools for Technology Transfer, 2001,
 * 
 * @li exegraph : use the execution graph to time blocks as in X. Li,
 * A. Roychoudhury, T. Mitra, "Modeling Out-of-Order Processors for Software
 * Timing Analysis", RTSS'04.
 * 
 * @par Syntax
 * 
 * @code
 * $ oipet [options] binary_file [function1 function2 ...]
 * @endcode
 * This command compute the WCET of the given function using OTAWA IPET facilities.
 * If no function is given, the main() function is used.
 * 
 * @par Generic Options
 * 
 * -I, --inline: consider a program where each function call is inlined
 * (default to true). Remark that this option may improve the WCET accuracy
 * but, in turn, may result in an enlarged computation time.
 * 
 * @par Cache Management Options
 * 
 * -i method, --icache=method: selects how the instruction must be managed.
 * The method may be one of none, ccg or cat.
 * 
 * -c path, --cache=path : load the cache description from the file whose
 * path is given.
 * 
 * @par Pipeline Management Options
 * 
 * -t method, --bbtiming=method: selects the method to time the blocks. The
 * method may be one of trivial, sim, delta or exegraph.
 * 
 * -p path, --processor=path: load the processor description from the file
 * whose path is given.
 * 
 * -D depth, --delta=depth: select the depth of the delta algorithm, that is,
 * how many blocks are used to compute inter-blocks effects (default to 4).
 * Bigger is the depth, better is the accuracy but longer is the computing time.
 * 
 * @par Dump Options
 * 
 * -C, --dump-constraints: for each function, generate a file named
 * function_name.lp containing the generated ILP system. The generated file
 * use the lp_solve syntax and may feed the lp_solve command.
 *
 * -G, --dump-graph: for each function, generate a file named
 * function_name.dot containing the graph of the processed functions in DOT
 * file format.
 */

// Options
class Command: public elm::option::Manager {
	String file;
	genstruct::Vector<String> funs;
	otawa::Manager manager;
	FrameWork *fw;
	gensim::GenericSimulator sim;
	CacheConfiguration *caches;
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
	{ "cat", icache_cat },
	{ "" }
};
EnumOption<int> icache_option(command, 'i', "icache",
	"instruction management method", icache_values);
StringOption cache(command, 'c', "cache", "used cache", "path", "");


// Basic-block Timing options
typedef enum bbtime_t {
	bbtime_sim,
	bbtime_delta,
	bbtime_exegraph,
	bbtime_trivial
} bbtime_t;
EnumOption<int>::value_t bbtime_values[] = {
	{ "method", bbtime_trivial },
	{ "sim", bbtime_sim },
	{ "delta", bbtime_delta },
	{ "exegraph", bbtime_exegraph },
	{ "trivial", bbtime_trivial },
	{ "" }
};
EnumOption<int> bbtime_option(command, 't', "bbtiming",
	"basic block timing method", bbtime_values);
StringOption proc(command, 'p', "processor", "used processor", "path", "");
IntOption delta(command, 'D', "delta", "use delta method with given sequence length", "length", 4);


// Dump options
BoolOption dump_constraints(command, 'C', "dump-constraints",
	"dump lp_solve constraints", false);
BoolOption dump_graph(command, 'G', "dump-graph",
	"dump DOT graph of the processed function", false);


// Other options
BoolOption verbose(command, 'v', "verbose", "verbose mode", false);
BoolOption inlining(command, 'I', "inline", "inline function calls", true);


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
	//VirtualCFG vcfg(cfg);
	
	if(!inlining && icache_option != icache_ccg) {
		inlining.set(true);
		cerr << "WARNING: using CCG without inlining may induce, in some cases, "
			    "an invalid WCET: inlining is activated.\n";
	}
	
	// Prepare processor configuration
	PropList props;
	if(!inlining)
		ENTRY_CFG(props) = cfg;
	else
		ENTRY_CFG(props) = new VirtualCFG(cfg);
	if(dump_constraints || dump_graph)
		props.set(EXPLICIT, true);
	if(verbose) {
		otawa::Processor::VERBOSE(props) = true;
		//cerr << "verbose !\n";
	}
	
	// Compute BB times
	switch(bbtime_option) {

	case bbtime_sim:
	case bbtime_delta: {
			BBTimeSimulator bbts;
			bbts.process(fw, props);
		}
		break;
		
	case bbtime_exegraph: {
			ExeGraphBBTime tbt(props);
			tbt.process(fw);
		}
		break;
		
	case bbtime_trivial: {
			TrivialBBTime tbt;
			ipet::PIPELINE_DEPTH(props) = 5;
			tbt.process(fw, props);
		}
		break;

	default:
		assert(0);
	};
		
	// Trivial data cache
	TrivialDataCacheManager dcache;
	dcache.process(fw, props);
		
	// Assign variables
	VarAssignment assign;
	assign.process(fw, props);
		
	// Build the system
	BasicConstraintsBuilder builder;
	builder.process(fw, props);
		
	// Process the instruction cache
	switch(icache_option) {
	case icache_ccg:
		{
			// build LBlock
			LBlockBuilder lbb;
			lbb.process(fw, props);
			
			// build ccg graph
			CCGBuilder ccgbuilder;
			ccgbuilder.process(fw, props);
			
			// Build ccg contraint
			CCGConstraintBuilder decomp;
			decomp.process(fw, props);
		}
		break;
		
	case icache_cat:
		{
			// build Cat lblocks
			CATBuilder catbuilder;
			catbuilder.process(fw, props);
			
			// Build CAT contraint
			CATConstraintBuilder decomp;
			decomp.process(fw, props);
		}
	
	case icache_none:
		{
			// Build the object function to maximize
			BasicObjectFunctionBuilder fun_builder;
			fun_builder.process(fw, props);	
		}
		break;
	}

	// Delta processing
	if(bbtime_option == bbtime_delta) {
		Delta::LEVELS(props) = delta;
		Delta delta;
		delta.process(fw, props);
	}

	// Load flow facts
	ipet::FlowFactLoader loader;
	loader.process(fw, props);
		
	// Resolve the system
	WCETComputation wcomp;
	wcomp.process(fw, props);

	// Get the result
	ilp::System *sys = SYSTEM(fw); 
	//vcfg.use<ilp::System *>(SYSTEM);
	cout << "WCET [" << file << ":" << fun << "] = "
		 << WCET(fw) << io::endl;
	
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

	// Build the cache
 	Cache::info_t info;
 	info.block_bits = 3;  // 2^3 octets par bloc
 	info.line_bits = 3;   // 2^3 lignes
 	info.set_bits = 0;    // 2^0 élément par ensemble (cache direct)
 	info.replace = Cache::NONE;
 	info.write = Cache::WRITE_THROUGH;
 	info.access_time = 0;
 	info.miss_penalty = 10;
 	info.allocate = false;
 	Cache *level1 = new Cache(info);
 	caches = new CacheConfiguration(level1);
	
	// Load the file
	PropList props;
	NO_SYSTEM(props) = true;
	if(proc) {
		PROCESSOR_PATH(props) = proc.value();
		SIMULATOR(props) = &sim;
	}
	if(cache)
		CACHE_CONFIG_PATH(props) = elm::system::Path(cache);
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

