/*
 *	$Id$
 *	oipet command
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2006-07, IRIT UPS.
 * 
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software 
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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
#include <otawa/exegraph/LiExeGraphBBTime.h>
#include <otawa/gensim/GenericSimulator.h>
#include <otawa/util/LBlockBuilder.h>
#include <otawa/display/CFGDrawer.h>
#include <otawa/cfg/CFGCollector.h>
#include <otawa/cfg/Virtualizer.h>
#include <otawa/cfg/CFGOutput.h>

using namespace elm;
using namespace elm::option;
using namespace otawa;
using namespace otawa::ipet;
using namespace otawa::hard;
using namespace elm::option;
using namespace elm::system;


/**
 * @page oipet oipet Command
 * 
 * oipet allows to use WCET IPET computation facilities of OTAWA. Currently,
 * you may only choose the algorithm for instruction cache support:
 * 
 * @li ccg for Cache Conflict Graph from  Li, Malik, Wolfe, "Efficient
 * microarchitecture modelling and path analysis for real-time software",
 * Proceedings of the 16th IEEE Real-Time Systems Symposium, 1995.
 * 
 * @li cat for categorization approach (an adaptation to IPET of Healy, Arnold, 
 * Mueller, Whalley, Harmon, "Bounding pipeline and instruction cache performance,"
 * IEEE Trans. Computers, 1999).
 * 
 * @li cat2 for categorization by abstraction interpretation (Ferdinand,
 * Martin, Wilhelm, "Applying Compiler Techniques to Cache Behavior Prediction.",
 * ACM SIGPLAN Workshop on Language, Compiler and Tool Support for Real-Time
 * Systems, 1997) improved in OTAWA.
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
 * @li -I, --do-not-inline -- cause to not inline functions for the WCET computation.
 * Consider that this option may save computation time but, conversely,
 * may reduce the WCET accuracy.
 * @li --ilp @i solver -- select the ILP solver to use (currently lp_solver (V4) or
 * lp_solve5.
 * 
 * @par Cache Management Options
 * 
 * @li -i method, --icache=method -- selects how the instruction must be managed.
 * The method may be one of none, ccg, cat, or cat2.
 * 
 * @li -c path, --cache=path -- load the cache description from the file whose
 * path is given.
 *
 * @li -p, --pseudounrolling -- enable Pseudo-Unrolling (cat2 only)
 *
 * @li -l, --linkedblocks -- enable LinkedBlocksDetector (cat2 only)
 *
 * @li -P type, --pers=type -- select persistence type for cat2: multi, outer,
 * or inner (default to multilevel).
 * 
 * @par Pipeline Management Options
 * 
 * @li -t method, --bbtiming=method -- selects the method to time the blocks. The
 * method may be one of trivial, sim, delta or exegraph.
 * 
 * @li -p path, --processor=path -- load the processor description from the file
 * whose path is given.
 * 
 * @li -D depth, --delta=depth -- select the depth of the delta algorithm, that is,
 * how many blocks are used to compute inter-blocks effects (default to 4).
 * Bigger is the depth, better is the accuracy but longer is the computing time.
 * 
 * @par Dump Options
 * 
 * @li -C, --dump-constraints -- for each function, generate a file named
 * function_name.lp containing the generated ILP system. The generated file
 * use the lp_solve syntax and may feed the lp_solve command.
 *
 * @li -G, --dump-graph -- for each function involved in the task, generate a file named
 * function_name.ps containing the graph of the processed functions in DOT
 * file format.
 * 
 * @li -o, --output prefix -- prepend the given prefix to the dump files names.
 */

// BBRatioDisplayer class
class BBRatioDisplayer: public BBProcessor {
public:
	BBRatioDisplayer(void): BBProcessor("BBTimeDisplayer", Version(1, 0, 0)) {
		require(ipet::WCET_FEATURE);
		require(ipet::ASSIGNED_VARS_FEATURE);
	}
protected:
	virtual void setup(WorkSpace *ws) {
		wcet = ipet::WCET(ws);
		system = ipet::SYSTEM(ws);
		out << "ADDRESS\tSIZE\tTIME\tCOUNT\tRATIO\tFUNCTION\n";
	}
	
	virtual void processCFG(WorkSpace *fw, CFG *cfg) {
		BBProcessor::processCFG(fw, cfg);
		out << "TOTAL FUNCTION\t"
			<< SUM(cfg) << '\t'
			<< (int)system->valueOf(ipet::VAR(cfg->entry())) << '\t'
			<< (float)SUM(cfg) * 100 / wcet << "%\t"
			<< cfg->label() << io::endl; 		
	}
	
	virtual void processBB(WorkSpace *fw, CFG *cfg, BasicBlock *bb) {
		if(bb->isEnd())
			return;
		int count = (int)system->valueOf(ipet::VAR(bb)),
			time = ipet::TIME(bb),
			total = time * count;
		SUM(cfg) = SUM(cfg) + total;
		out << bb->address() << '\t'
			<< bb->size() << '\t'
			<< time << '\t'
			<< count << '\t'
			<< (float)total * 100 / wcet << "%\t"
			<< cfg->label() << io::endl; 
	}
private:
	static otawa::Identifier<int> SUM;
	int wcet;
	ilp::System *system;
};
otawa::Identifier<int> BBRatioDisplayer::SUM("", 0);

// Options
class Command: public elm::option::Manager {
	String file;
	genstruct::Vector<String> funs;
	otawa::Manager manager;
	WorkSpace *fw;
	gensim::GenericSimulator sim;
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
	icache_cat,
	icache_cat2
} icache_t;
EnumOption<int>::value_t icache_values[] = {
	{ "method", icache_def },
	{ "none", icache_none },
	{ "ccg", icache_ccg },
	{ "cat", icache_cat },
	{ "cat2", icache_cat2 },
	{ "" }
};


EnumOption<int>::value_t pers_values[] = {
	{ "type",  FML_MULTI},
	{ "multi", FML_MULTI},
	{ "outer", FML_OUTER},
	{ "inner", FML_INNER},
	{ "" }
};


EnumOption<int> icache_option(command, 'i', "icache",
	"instruction management method", icache_values);
StringOption cache(command, 'c', "cache", "used cache", "path", "");
EnumOption<int> pers_option(command, 'P', "pers", "persistence type for cat2 (multi, inner, or outer)", pers_values);


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
BoolOption dump_ratio(command, 'R', "dump-ratios",
	"dump ratio of each BB for the WCET", false);


// Other options
BoolOption verbose(command, 'v', "verbose", "verbose mode", false);
BoolOption linkedblocks(command, 'l', "linkedblocks", "enable LinkedBlocksDetector (cat2 only)", false);
BoolOption pseudounrolling(command, 'u', "pseudounrolling", "enable Pseudo-Unrolling (cat2 only)", false);
BoolOption not_inlining(command, 'I', "do-not-inline", "do not inline function calls", false);
static StringOption ilp_plugin(command, "ilp", "select the ILP solver", "solver name");
static StringOption output_prefix(command, 'o', "output", "Prefix of output file names", "output prefix", "");


/**
 * Build the command manager.
 */
Command::Command(void) {
	program = "oipet";
	version = "1.1.0";
	author = "H. Cassé";
	copyright = "Copyright (c) 2006-07, IRIT-UPS";
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
	
	// Inlining required for CCG
	if(not_inlining && icache_option == icache_ccg) {
		not_inlining.set(false);
		cerr << "WARNING: using CCG without inlining may induce, in some cases, "
			    "an invalid WCET: inlining is activated.\n";
	}
	
	// Prepare processor configuration
	PropList props;
	TASK_ENTRY(props) = fun.toCString();
	if(dump_constraints || dump_graph)
		props.set(EXPLICIT, true);
	if(verbose) {
		otawa::Processor::VERBOSE(props) = true;
		//cerr << "verbose !\n";
	}
	if(ilp_plugin)
		ipet::ILP_PLUGIN_NAME(props) = ilp_plugin.value().toCString();
	
	// Virtualization
	if(!not_inlining) {
		Virtualizer virt;
		virt.process(fw, props);
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
			LiExeGraphBBTime tbt;
			tbt.process(fw, props);
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
		break;
	
	case icache_cat2:
		{
			// build Cat lblocks
			FIRSTMISS_LEVEL(props) = (fmlevel_t) (int) pers_option;
			PSEUDO_UNROLLING(props) = pseudounrolling;			
			CAT2NCBuilder cat2builder;
			cat2builder.process(fw, props);

			if (linkedblocks) {
				// Linked Blocks
	                        LinkedBlocksDetector lbd;
	                        lbd.process(fw, props);
			}
			
			// Build CAT contraint
			CAT2ConstraintBuilder decomp;
			decomp.process(fw, props);
		}
	}
	
	// Build the object function to maximize
	BasicObjectFunctionBuilder fun_builder;
	fun_builder.process(fw, props);	

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
		String out_file = _ << output_prefix.value() << fun << ".lp";
		io::OutFileStream stream(&out_file);
		if(!stream.isReady())
			throw MessageException(_ << "cannot create file \"" << out_file
				<< "\".");
		sys->dump(stream);
	}
	
	// Dump the CFG
	if(dump_graph) {
		
		// Record results
		WCETCountRecorder recorder;
		recorder.process(fw, props);
		
		// Generates output
		CFGOutput output;
		output.process(fw, props);
	}
	
	// Dump the ratio
	if(dump_ratio) {
		BBRatioDisplayer displayer;
		displayer.process(fw, props);
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

