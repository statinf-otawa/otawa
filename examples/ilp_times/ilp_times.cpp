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

using namespace elm;
using namespace otawa;
using namespace otawa::ipet;

int main(int argc, char **argv) {
	
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
	Manager manager;
	PropList props;
	props.set<Loader *>(Loader::ID_Loader, &Loader::LOADER_Gliss_PowerPC);
	props.set<CacheConfiguration *>(Platform::ID_Cache, &cache_conf);
	
	try {
		elm::system::StopWatch main_sw;
		main_sw.start();
		
		// Load program
		if(argc < 2) {
			cerr << "ERROR: no argument.\n"
				 << "Syntax is : test_ipet <executable>\n";
			exit(2);
		}
		FrameWork *fw = manager.load(argv[1], props);
		
		// Find main CFG
		CFG *cfg = fw->getCFGInfo()->findCFG("main");
		if(cfg == 0) {
			cerr << "ERROR: cannot find main !\n";
			return 1;
		}
		/*else
			cout << "main found at 0x" << cfg->address() << '\n';*/
		
		// Removing __eabi call if available
		for(CFG::BBIterator bb(cfg); bb; bb++)
			for(BasicBlock::OutIterator edge(bb); edge; edge++)
				if(edge->kind() == Edge::CALL
				&& edge->target()
				&& edge->calledCFG()->label() == "__eabi") {
					delete(*edge);
					break;
				}
		
		// Now, use a VCFG
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
		
		// build ccg graph
		CCGBuilder ccgbuilder(fw);
		ccgbuilder.processCFG(fw, &vcfg );
			
		// Build ccg contraint
		CCGConstraintBuilder decomp(fw);
		decomp.processCFG(fw, &vcfg );
			
		//Build the objectfunction
		CCGObjectFunction ofunction(fw);
		ofunction.processCFG(fw, &vcfg );
		
		// Load flow facts
		ipet::FlowFactLoader loader(props);
		loader.processCFG(fw, &vcfg);
		
		// Resolve the system
		elm::system::StopWatch ilp_sw;
		ilp_sw.start();
		WCETComputation wcomp(props);
		wcomp.processCFG(fw, &vcfg);
		ilp_sw.stop();
		
		// Get the result
		main_sw.stop();
		ilp::System *sys = vcfg.use<ilp::System *>(IPET::ID_System);
		cout << argv[1] << '\t'
			 << sys->countVars() << '\t'
			 << sys->countConstraints() << '\t'
			 << (int)(main_sw.delay() / 1000) << '\t'
			 << (int)(ilp_sw.delay() / 1000) << '\n';
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

