#include "EnhacedAggregationGraph.h"
#include "BBTimeSimulator.h"
#include <stdlib.h>
#include <stdio.h>
#include <elm/io.h>
#include <otawa/otawa.h>
#include <elm/system/StopWatch.h>
#include <otawa/proc/ProcessorException.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/cache/ccg/CCGConstraintBuilder.h>
#include <otawa/cache/ccg/CCGBuilder.h>
#include <otawa/cache/ccg/CCGObjectFunction.h>
#include <otawa/cache/categorisation/CATConstraintBuilder.h>
#include <otawa/cache/categorisation/CATBuilder.h>

using namespace elm;
using namespace otawa;
using namespace otawa::ipet;
using namespace otawa::hard;

// Option string
CString ccg_option = "-ccg";
CString cat_option = "-cat";
CString dot_option = "-dot";
CString dump_option = "-dump";
CString infos_option = "-infos";

/**
 * Display help message and exit the program.
 */
void help(void) {
	cerr << "ERROR: bad arguments.\n";
	cerr << "SYNTAX: test_aggregation program [options]\n";
	cerr << "options are:\n";
	cerr << "  -L#    splits the paths with length > #\n";
	cerr << "  -I#    splits the paths with a total number of instructions > #\n";
	cerr << "  -S#    splits the paths when # path splits have been encountered\n";
	cerr << "  -J#    splits the paths when # path joins  have been encountered\n";
	cerr << "  -infos dumps to the standard output some informations about the graph\n";
	cerr << "  -dot   dumps to the standard output informations to make a dot graph\n";
	cerr << "  -dump  dumps to the standard output the ILP system\n";
	cerr << "  -ccg   uses the CCG method for instruction cache\n";
	cerr << "  -cat   uses the categorisation method for instruction cache\n";
	cerr << "If neither -ccg nor -cat is specified, the program does NOT perform\n";
	cerr << "the Instruction Cache Processor\n";
	exit(1);
}


int main(int argc, char **argv){
	// Options
	CString file;
	bool dot = false;
	bool dump = false;
	bool infos = false;
	elm::Option<int> max_length;
	elm::Option<int> max_insts;
	elm::Option<int> max_joins;
	elm::Option<int> max_splits;
	enum {
		NONE = 0,
		CCG,
		CAT
	} method = NONE;

	// Cache configuration	
	Cache::info_t inst_cache_info = {
		1,
		10,
		4,
		6,
		0,
		Cache::LRU,
		Cache::WRITE_THROUGH,
		false
	};
	Cache::info_t data_cache_info = {
		1,
		10,
		4,
		6,
		2,
		Cache::LRU,
		Cache::WRITE_THROUGH,
		false
	};
	Cache inst_cache(inst_cache_info);
	Cache data_cache(data_cache_info);
	CacheConfiguration cache_conf(&inst_cache, &data_cache);

	// Processing the arguments
	for(int i = 1; i < argc; i++) {
		if(argv[i][0] != '-')
			file = argv[i];
		else if(argv[i][0] == '-' && argv[i][1] == 'L'){
			int val;
			sscanf(&argv[i][2],"%d",&val);
			max_length = val;
		}else if(argv[i][0] == '-' && argv[i][1] == 'J'){
			int val;
			sscanf(&argv[i][2],"%d",&val);
			max_joins = val;
		}else if(argv[i][0] == '-' && argv[i][1] == 'I'){
			int val;
			sscanf(&argv[i][2],"%d",&val);
			max_insts = val;
		}else if(argv[i][0] == '-' && argv[i][1] == 'S'){
			int val;
			sscanf(&argv[i][2],"%d",&val);
			max_splits = val;
		}else if(ccg_option == argv[i])
			method = CCG;
		else if(cat_option == argv[i])
			method = CAT;
		else if(dot_option == argv[i])
			dot = true;
		else if(dump_option == argv[i])
			dump = true;
		else if(infos_option == argv[i])
			infos = true;
		else
			help();
	}
	if(!file)
		help();

	// Start timer
	elm::system::StopWatch main_sw;
	main_sw.start();
	
	// Load the file
	Manager manager;
	PropList props;
	props.set<Loader *>(Loader::ID_Loader, &Loader::LOADER_Gliss_PowerPC);
	props.set<CacheConfiguration *>(Platform::ID_Cache, &cache_conf);
	try {
		FrameWork *fw = manager.load(file, props);
		
		// Find main CFG
		CFG *cfg = fw->getCFGInfo()->findCFG("main");
		if(cfg == 0) {
			cerr << "ERROR: cannot find main !\n";
			return 1;
		}
		
		// Removing __eabi call if available
		for(CFG::BBIterator bb(cfg); bb; bb++)
			for(BasicBlock::OutIterator edge(bb); edge; edge++)
				if(edge->kind() == Edge::CALL
				&& edge->target()
				&& edge->calledCFG()->label() == "__eabi") {
					delete(*edge);
					break;
				}
		
		// Now, use an inlined VCFG
		VirtualCFG vcfg(cfg);
		
		// Prepare processor configuration
		//PropList props;
		props.set(EXPLICIT, true);
		if(max_length)
			Agg_Max_Length(props) = *max_length;
		if(max_insts)
			Agg_Max_Insts(props) = *max_insts;
		if(max_joins)
			Agg_Max_Joins(props) = *max_joins;
		if(max_splits)
			Agg_Max_Splits(props) = *max_splits;
		
		// Now, create an aggregation graph
		EnhacedAggregationGraph aggregationGraph(&vcfg, props);
		
		// print dot informations if -dot option were passed
		if(dot)
			aggregationGraph.toDot(cout);
		
		
		// Now use the AggregationGraph
		cfg = &aggregationGraph;
		
		
//		cfg = &vcfg;
		
		// Compute BB times
//		TrivialBBTime tbt(5, props);
//		tbt.processCFG(fw, cfg);
		BBTimeSimulator bts(props);
		bts.processCFG(fw, cfg);

		// print informations
		//cerr << "Original CFG contains " << vcfg.bbs().count() << " nodes.\n";
		if(infos)
			aggregationGraph.printStats(cout);
		if(dump)
			aggregationGraph.printEquivalents(cout);
		
		// Trivial data cache
		TrivialDataCacheManager dcache(props);
		dcache.processCFG(fw, cfg);
		
		// Assign variables
		VarAssignment assign(props);
		assign.processCFG(fw, cfg);
		
		// Build the system
		BasicConstraintsBuilder builder(props);
		builder.processCFG(fw, cfg);
		
		// Process the instruction cache
		if(method == CCG) {
			
			// build ccg graph
			CCGBuilder ccgbuilder;
			ccgbuilder.processCFG(fw, cfg );
			
			// Build ccg contraint
			CCGConstraintBuilder decomp(fw);
			decomp.processCFG(fw, cfg );
			
			//Build the objectfunction
			CCGObjectFunction ofunction(fw);
			ofunction.processCFG(fw, cfg );
		}
		else {
			if(method == CAT) {
				
				// build Cat lblocks
				CATBuilder catbuilder;
				catbuilder.processCFG(fw, cfg);
			
				// Build CAT contraint
				CATConstraintBuilder decomp;
				decomp.processCFG(fw, cfg);
			}

			// Build the object function to maximize
			BasicObjectFunctionBuilder fun_builder;
			fun_builder.processCFG(fw, cfg);	
		}
		
		// Load flow facts
		ipet::FlowFactLoader loader(props);
		loader.processCFG(fw, cfg);

		//aggregationGraph.printStats(cerr);

		
		// Resolve the system
		elm::system::StopWatch ilp_sw;
		ilp_sw.start();
		WCETComputation wcomp(props);
		wcomp.processCFG(fw, cfg);
		ilp_sw.stop();
		main_sw.stop();

		// Find the code size
		int size = 0;
		for(Iterator<File *> ffile(*fw->files()); ffile; ffile++)
			for(Iterator<Segment *> seg(ffile->segments()); seg; seg++)
				if(seg->flags() & Segment::EXECUTABLE)
					size += seg->size();
		
		// Get the result
		ilp::System *sys = cfg->use<ilp::System *>(SYSTEM);
		if(dump)
			sys->dump();
		cout << file << '\t'
			 << sys->countVars() << '\t'
			 << sys->countConstraints() << '\t'
			 << (int)(main_sw.delay() / 1000) << '\t'
			 << (int)(ilp_sw.delay() / 1000) << '\t'
			 << cfg->use<int>(WCET) << '\t'
			 << size << '\n';
	}
	catch(LoadException e) {
		cerr << "ERROR: " << e.message() << '\n';
		exit(1);
	}
	catch(ProcessorException e) {
		cerr << "ERROR: " << e.message() << '\n';
		exit(1);
	}
	return 0;
}
