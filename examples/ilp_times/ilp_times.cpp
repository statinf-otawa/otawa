/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	test/ipet/test_ipet.cpp -- test for IPET feature.
 */

#include <elm/io.h>
#include <otawa/otawa.h>
#include <otawa/ipet/IPET.h>
#include <otawa/ipet/TrivialBBTime.h>
#include <otawa/ipet/VarAssignment.h>
#include <otawa/ipet/BasicConstraintsBuilder.h>
#include <otawa/ipet/WCETComputation.h>
#include <otawa/ipet/FlowFactLoader.h>
#include <otawa/ipet/BasicObjectFunctionBuilder.h>
#include <otawa/proc/ProcessorException.h>
#include <otawa/ipet/TrivialDataCacheManager.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/ilp.h>
#include <elm/system/StopWatch.h>
#include <otawa/cache/ccg/CCGConstraintBuilder.h>
#include <otawa/cache/ccg/CCGBuilder.h>
#include <otawa/cache/categorisation/CATConstraintBuilder.h>
#include <otawa/cache/categorisation/CATBuilder.h>

using namespace elm;
using namespace otawa;
using namespace otawa::ipet;
using namespace otawa::hard;

// Option string
CString ccg_option = "-ccg";
CString cat_option = "-cat";

/**
 * Display help message and exit the program.
 */
void help(void) {
	cout << "ERROR: bad arguments.\n";
	cout << "SYNTAX: icache_comp [-ccg|-cat] program\n";
}


/**
 * Program entry point.
 * @return Error code.
 */
int main(int argc, char **argv) {
	
	// Options
	CString file;
	enum {
		NONE = 0,
		CCG,
		CAT
	} method = CCG;

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
		else if(ccg_option == argv[i])
			method = CCG;
		else if(cat_option == argv[i])
			method = CAT;
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
//	LOADER(props) = &Loader::LOADER_Gliss_PowerPC;
	CACHE_CONFIG(props) = &cache_conf;
	try {
		WorkSpace *fw = manager.load(file, props);
		
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
		ENTRY_CFG(fw) = &vcfg;
		
		// Prepare processor configuration
		PropList props;
		props.set(EXPLICIT, true);
		
		// Compute BB times
		TrivialBBTime tbt;
		ipet::PIPELINE_DEPTH(props) = 5;
		tbt.process(fw, props);
		
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
		if(method == CCG) {
			
			// build ccg graph
			CCGBuilder ccgbuilder;
			ccgbuilder.process(fw);
			
			// Build ccg contraint
			CCGConstraintBuilder decomp;
			decomp.process(fw);			
		}
		else {
			if(method == CAT) {
				
				// build Cat lblocks
				CATBuilder catbuilder;
				catbuilder.process(fw);
			
				// Build CAT contraint
				CATConstraintBuilder decomp;
				decomp.process(fw);
			}

			// Build the object function to maximize
			BasicObjectFunctionBuilder fun_builder;
			fun_builder.process(fw);	
		}
		
		// Load flow facts
		ipet::FlowFactLoader loader;
		loader.process(fw, props);
		
		// Resolve the system
		elm::system::StopWatch ilp_sw;
		ilp_sw.start();
		WCETComputation wcomp;
		wcomp.process(fw, props);
		ilp_sw.stop();
		main_sw.stop();

		// Find the code size
		int size = 0;
		for(Process::FileIter ffile(fw->process()); ffile; ffile++)
			for(File::SegIter seg(ffile); seg; seg++)
				if(seg->flags() & Segment::EXECUTABLE)
					size += seg->size();
		
		// Get the result
		ilp::System *sys = vcfg.use<ilp::System *>(SYSTEM);
		cout << file << '\t'
			 << sys->countVars() << '\t'
			 << sys->countConstraints() << '\t'
			 << (int)(main_sw.delay() / 1000) << '\t'
			 << (int)(ilp_sw.delay() / 1000) << '\t'
			 << vcfg.use<int>(WCET) << '\t'
			 << size << '\n';
	}
	catch(elm::Exception& e) {
		cerr << "ERROR: " << e.message() << '\n';
		return 1;
	}
	return 0;
}

