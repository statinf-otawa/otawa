/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	test/ipet/test_ipet.cpp -- test for IPET feature.
 */

#include <stdlib.h>
#include <elm/io.h>
#include <otawa/otawa.h>
#include <otawa/ipet.h>
#include <otawa/proc/ProcessorException.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/ilp.h>

using namespace elm;
using namespace otawa;
using namespace otawa::ipet;
using namespace otawa::hard;

int main(int argc, char **argv) {
	
	Cache::info_t info = {
		1,
		10,
		4,
		6,
		2,
		Cache::LRU,
		Cache::WRITE_THROUGH,
		false
	};
	Cache data_cache(info);
	CacheConfiguration cache_conf(0, &data_cache);
	Manager manager;
	PropList props;
	props.set<Loader *>(Loader::ID_Loader, &Loader::LOADER_Gliss_PowerPC);
	props.set<CacheConfiguration *>(Platform::ID_Cache, &cache_conf);
	
	try {
		
		// Load program
		if(argc < 2) {
			cerr << "ERROR: no argument.\n"
				 << "Syntax is : test_ipet <executable>\n";
			exit(2);
		}
		FrameWork *fw = manager.load(argv[1], props);
		
		// Find main CFG
		cout << "Looking for the main CFG\n";
		CFG *cfg = fw->getCFGInfo()->findCFG("main");
		if(cfg == 0) {
			cerr << "ERROR: cannot find main !\n";
			return 1;
		}
		else
			cout << "main found at 0x" << cfg->address() << '\n';
		
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
		cout << "Timing the BB\n";
		TrivialBBTime tbt(5, props);
		tbt.processCFG(fw, &vcfg);
		
		// Trivial data cache
		cout << "Managing the data cache\n";
		TrivialDataCacheManager dcache(props);
		dcache.processCFG(fw, &vcfg);
		
		// Assign variables
		cout << "Numbering the main\n";
		VarAssignment assign(props);
		assign.processCFG(fw, &vcfg);
		
		// Build the system
		cout << "Building the ILP system\n";
		BasicConstraintsBuilder builder(props);
		builder.processCFG(fw, &vcfg);
		
		// Build the object function to maximize
		cout << "Building the ILP object function\n";
		BasicObjectFunctionBuilder fun_builder(props);
		fun_builder.processCFG(fw, &vcfg);
		
		// Load flow facts
		cout << "Loading flow facts\n";
		ipet::FlowFactLoader loader(props);
		loader.processCFG(fw, &vcfg);
		
		// Resolve the system
		cout << "Resolve the system\n";
		WCETComputation wcomp(props);
		wcomp.processCFG(fw, &vcfg);
		
		// Display the result
		ilp::System *sys = vcfg.use<ilp::System *>(IPET::ID_System);
		sys->dump();
		cout << sys->countVars() << " variables and "
			 << sys->countConstraints() << " constraints.\n";
		cout << "SUCCESS\nWCET = " << vcfg.use<int>(IPET::ID_WCET) << '\n';
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

