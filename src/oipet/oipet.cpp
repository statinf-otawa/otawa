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
#include <elm/io/OutFileStream.h>
#include <otawa/otawa.h>
#include <otawa/ipet.h>
#include <otawa/ilp.h>
#include <elm/system/StopWatch.h>
#include <otawa/proc/ProcessorException.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/tsim/BBTimeSimulator.h>
#include <otawa/exegraph/LiExeGraphBBTime.h>
#include <otawa/util/LBlockBuilder.h>
#include <otawa/display/CFGDrawer.h>
#include <otawa/cfg/CFGCollector.h>
#include <otawa/cfg/Virtualizer.h>
#include <otawa/display/CFGOutput.h>
#include <otawa/exegraph/ParamExeGraphBBTime.h>
#include <otawa/parexegraph/GraphBBTime.h>
#include <otawa/util/FlowFactLoader.h>
#include <otawa/util/BBRatioDisplayer.h>
#include <otawa/display/ILPSystemDisplayer.h>
#include <otawa/stats.h>
#include <otawa/ipet/VarAssignment.h>
#include <otawa/tsim/Delta.h>
#include <otawa/proc/DynProcessor.h>

using namespace elm;
using namespace elm::option;
using namespace otawa;
using namespace otawa::ipet;
using namespace otawa::hard;
using namespace elm::option;


/**
 * @addtogroup commands
 * @section oipet oipet Command
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
 *
 * @li --ilp @i solver -- select the ILP solver to use (currently lp_solver (V4) or
 * lp_solve5).
 *
 * @par Cache Management Options
 *
 * @li -c path, --cache=path -- load the cache description from the file whose
 * path is given.
 *
 * @li -f path, --flow-facts=path -- use the given flow fact file,
 *
 * @li -i method, --icache=method -- selects how the instruction must be managed.
 * The method may be one of none, ccg, cat, or cat2.
 *
 * @li -l, --linkedblocks -- enable LinkedBlocksDetector (cat2 only)
 *
 * @li -p, --pseudounrolling -- enable Pseudo-Unrolling (cat2 only)
 *
 * @li -P type, --pers=type -- select persistence type for cat2: multi, outer,
 * or inner (default to multilevel).
 *
 * @par Pipeline Management Options
 *
 * @li -D depth, --delta=depth -- select the depth of the delta algorithm, that is,
 * how many blocks are used to compute inter-blocks effects (default to 4).
 * Bigger is the depth, better is the accuracy but longer is the computing time.
 *
 * @li -p path, --processor=path -- load the processor description from the file
 * whose path is given.
 *
 * @li -t method, --bbtiming=method -- selects the method to time the blocks. The
 * method may be one of trivial, sim, delta, exegraph or paramexegraph.
 *
 * @par Dump Options
 *
 * @li -C, --dump-constraints=output_kind -- dump the constraints that has been used
 * to compute the WCET. The output_kind may be "lp" for lp_solve compatible output
 * (suffixed by ".lp") or "html" for HTML output.
 *
 * @li -G, --dump-graph -- for each function involved in the task, generate a file named
 * function_name.ps containing the graph of the processed functions in DOT
 * file format.
 *
 * @li -o, --output prefix -- prepend the given prefix to the dump files names.
 *
 * @li -T, --dump-time -- output statistics about time and execution count of BB.
 *
 *
 */


// Options
class Command: public elm::option::Manager {
	String file;
	genstruct::Vector<String> funs;
	otawa::Manager manager;
	WorkSpace *fw;
	#ifdef	HAS_SYSTEMC
		gensim::GenericSimulator sim;
	#endif
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
	bbtime_trivial,
	bbtime_paramexegraph,
	bbtime_parexegraph
} bbtime_t;
EnumOption<int>::value_t bbtime_values[] = {
	{ "method", bbtime_trivial },
	{ "sim", bbtime_sim },
	{ "delta", bbtime_delta },
	{ "exegraph", bbtime_exegraph },
	{ "trivial", bbtime_trivial },
	{ "paramexegraph", bbtime_paramexegraph },
	{ "parexegraph", bbtime_parexegraph },
	{ "" }
};
EnumOption<int> bbtime_option(command, 't', "bbtiming",
	"basic block timing method", bbtime_values);
StringOption proc(command, 'p', "processor", "used processor", "path", "");
IntOption delta(command, 'D', "delta", "use delta method with given sequence length", "length", 4);


// Dump options
typedef enum {
	NO_OUTPUT,
	LP,
	HTML
} const_output_t;
EnumOption<int>::value_t cons_output_kinds[] = {
		{ "output kind", NO_OUTPUT },
		{ "lp", LP },
		{ "html", HTML },
		{ "" }
};
EnumOption<int>::value_t graph_output_kinds[] = {
	{ "output kind", display::OUTPUT_ANY },
	{ "ps", display::OUTPUT_PS },
	{ "pdf", display::OUTPUT_PDF },
	{ "png", display::OUTPUT_PNG },
	{ "gif", display::OUTPUT_GIF },
	{ "jpg", display::OUTPUT_JPG },
	{ "svg", display::OUTPUT_SVG },
	{ "dot", display::OUTPUT_DOT }
};
EnumOption<int> dump_constraints(command, 'C', "dump-constraints",
	"dump lp_solve constraints (lp or html)", cons_output_kinds);
EnumOption<int> dump_graph(command, 'G', "dump-graph",
	"dump DOT graph of the processed function", graph_output_kinds);
BoolOption dump_time(command, 'T', "dump-time",
	"dump ratio of each BB for the WCET", false);


// Other options
BoolOption verbose(command, 'v', "verbose", "verbose mode", false);
BoolOption linkedblocks(command, 'l', "linkedblocks", "enable LinkedBlocksDetector (cat2 only)", false);
BoolOption pseudounrolling(command, 'u', "pseudounrolling", "enable Pseudo-Unrolling (cat2 only)", false);
BoolOption not_inlining(command, 'I', "do-not-inline", "do not inline function calls", false);
static StringOption ilp_plugin(command, "ilp", "select the ILP solver", "solver name");
static StringOption output_prefix(command, 'o', "output", "Prefix of output file names", "output prefix", "");
static StringOption flow_facts(command, 'f', "flowfacts", "Select the fow fact file", "flow facts file", "");
static SwitchOption stats(command, option::cmd, "-s", option::cmd, "--stat", option::description, "dumps statistics of the computation", option::end);



/**
 * Build the command manager.
 */
Command::Command(void):
	Manager(
		option::program, "oipet",
		option::version, new Version(1, 1, 0),
		option::author, "H. Cassé",
		option::copyright, "Copyright (c) 2006-07, IRIT-UPS",
		option::description, "Compute the WCET of some tasks of a binary using IPET techniques.",
		option::free_arg, "binary_file function1 function2 ...",
		option::end), fw(0)
{
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


/* class for collecting statistics */
class MyCollector: public StatCollector::Collector {
public:
	inline MyCollector(StatCollector *collector): coll(collector) { }
	virtual ~MyCollector(void) { }

	virtual void enter(const ContextualStep& step) {
		path.push(step);
	}

	virtual void collect(const Address& address, t::uint32 size, int value) {
		cout << value << ' ' << address << ':' << (address + size) << ' ' << path << io::endl;
	}

	virtual void leave(void) {
		path.pop();
	}

private:
	ContextualPath path;
	StatCollector *coll;
};

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
	if(stats)
		otawa::Processor::COLLECT_STATS(props) = true;
	TASK_ENTRY(props) = fun.toCString();
	if(dump_constraints || dump_graph !=  display::OUTPUT_ANY)
		EXPLICIT(props) = true;
	if(verbose) {
		otawa::Processor::VERBOSE(props) = true;
		//cerr << "verbose !\n";
	}
	if(ilp_plugin)
		ipet::ILP_PLUGIN_NAME(props) = ilp_plugin.value().toCString();
	if(flow_facts)
		FLOW_FACTS_PATH(props) = Path(flow_facts);
	if(::proc)
		PROCESSOR_PATH(props) = ::proc.value();

	// Virtualization
	if(!not_inlining) {
		Virtualizer virt;
		virt.process(fw, props);
	}

	// Compute BB times
	switch(bbtime_option) {

	case bbtime_sim:
	case bbtime_delta:
		{
			tsim::BBTimeSimulator bbts;
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

	case bbtime_paramexegraph: {
			//ParamExeGraphBBTime tbt;
			//tbt.process(fw, props);
			cerr << "ERROR: paramexegraph is deprecared\n";
			return;
		}
		break;

	case bbtime_parexegraph: {
			GraphBBTime<ParExeGraph> gbbt;
			gbbt.process(fw, props);
		}
		break;

	default:
		ASSERT(false);
		break;
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
			DynProcessor ccgbuilder("otawa::ccg::Builder");
			ccgbuilder.process(fw, props);

			// Build ccg contraint
			DynProcessor decomp("otawa::ccg::ConstraintBuilder");
			decomp.process(fw, props);
		}
		break;

	case icache_cat:
		{
			// build Cat lblocks
			DynProcessor catbuilder("otawa::cat::CATBuilder");
			catbuilder.process(fw, props);

			// Build CAT contraint
			DynProcessor decomp("otawa::cat::CATConstraintBuilder");
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
		break;
	}

	// Build the object function to maximize
	BasicObjectFunctionBuilder fun_builder;
	fun_builder.process(fw, props);

	// Delta processing
	if(bbtime_option == bbtime_delta) {
		tsim::Delta::LEVELS(props) = delta;
		tsim::Delta delta;
		delta.process(fw, props);
	}

	// Resolve the system
	WCETComputation wcomp;
	wcomp.process(fw, props);

	// Get the result
	ilp::System *sys = SYSTEM(fw);
	//vcfg.use<ilp::System *>(SYSTEM);
	cout << "WCET [" << file << ":" << fun << "] = "
		 << WCET(fw) << io::endl;

	// Dump the ILP system
	switch(dump_constraints) {
	case NO_OUTPUT:
		break;
	case LP: {
			String out_file = _ << output_prefix.value() << fun << ".lp";
			cerr << "INFO: outputting constraints to " << out_file << "...\n";
			io::OutFileStream stream(&out_file);
			if(!stream.isReady())
				throw MessageException(_ << "cannot create file \"" << out_file << "\".");
			sys->dump(stream);
		}
		break;
	case HTML: {
	  elm::system::Path path(_ << output_prefix.value() << fun << ".html");
			cerr << "INFO: outputting constraints to " << path << "...\n";
			display::ILPSystemDisplayer::PATH(props) = path;
			display::ILPSystemDisplayer disp;
			disp.process(fw, props);
		}
		break;
	}

	// Dump the CFG
	if(dump_graph != display::OUTPUT_ANY) {
		cerr << "INFO: outputting graph...\n";

		// Record results
		WCETCountRecorder recorder;
		recorder.process(fw, props);

		// Generates output
		display::CFGOutput::KIND(props) = display::kind_t(dump_graph.value());
		display::CFGOutput output;
		output.process(fw, props);
	}

	// Dump the ratio
	if(dump_time) {
	  elm::system::Path path(_ << output_prefix.value() << fun << ".time");
		cerr << "INFO: outputting time information to " << path << "...\n";
		BBRatioDisplayer::PATH(props) = path;
		BBRatioDisplayer displayer;
		displayer.process(fw, props);
	}

	// dump statistics
	if(stats) {
		for(StatInfo::Iter coll(fw); coll; coll++) {
			cout << "****** STATISTICS FOR " << coll->name() << " (" << coll->unit() << ") *****\n";
			MyCollector collector(coll);
			coll->collect(collector);
		}
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
	if(::proc) {
		PROCESSOR_PATH(props) = ::proc.value();
#		ifdef HAS_SYSTEMC
			SIMULATOR(props) = &sim;
#		endif
	}
	if(::cache)
		CACHE_CONFIG_PATH(props) = elm::system::Path(::cache);
	otawa::Processor::VERBOSE(props) = verbose;
	fw = manager.load(&file, props);

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

	// cleanup all
	if(fw)
		delete fw;
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

