/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	test/ipet/test_ipet.cpp -- test for IPET feature.
 */

#include <stdlib.h>
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
#include <otawa/hardware/CacheConfiguration.h>
#include <otawa/ilp.h>
#include <elm/system/StopWatch.h>
#include <otawa/cache/ccg/CCGConstraintBuilder.h>
#include <otawa/cache/ccg/CCGBuilder.h>
#include <otawa/cache/ccg/CCGObjectFunction.h>
#include <otawa/cache/categorisation/CATConstraintBuilder.h>
#include <otawa/cache/categorisation/CATBuilder.h>

using namespace elm;
using namespace otawa;
using namespace otawa::ipet;

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
		PropList props;
		props.set(IPET::ID_Explicit, true);
		
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
		if(method == CCG) {
			
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
		else {
			if(method == CAT) {
				
				// build Cat lblocks
				CATBuilder catbuilder;
				catbuilder.processCFG(fw, &vcfg);
			
				// Build CAT contraint
				CATConstraintBuilder decomp;
				decomp.processCFG(fw, &vcfg);
			}

			// Build the object function to maximize
			BasicObjectFunctionBuilder fun_builder;
			fun_builder.processCFG(fw, &vcfg);	
		}
		
		// Load flow facts
		ipet::FlowFactLoader loader(props);
		loader.processCFG(fw, &vcfg);
		
		// Resolve the system
		elm::system::StopWatch ilp_sw;
		ilp_sw.start();
		WCETComputation wcomp(props);
		wcomp.processCFG(fw, &vcfg);
		ilp_sw.stop();
		main_sw.stop();

		// Find the code size
		int size = 0;
		for(Iterator<File *> ffile(*fw->files()); ffile; ffile++)
			for(Iterator<Segment *> seg(ffile->segments()); seg; seg++)
				if(seg->flags() & Segment::EXECUTABLE)
					size += seg->size();
		
		// Get the result
		ilp::System *sys = vcfg.use<ilp::System *>(IPET::ID_System);
		cout << file << '\t'
			 << sys->countVars() << '\t'
			 << sys->countConstraints() << '\t'
			 << (int)(main_sw.delay() / 1000) << '\t'
			 << (int)(ilp_sw.delay() / 1000) << '\t'
			 << vcfg.use<int>(IPET::ID_WCET) << '\t'
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

