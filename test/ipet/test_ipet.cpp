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
#include <otawa/ipet/BBTimeSimulator.h>
#include <otawa/gensim/GenericSimulator.h>

//#define WITH_VIRTUAL

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
	LOADER(props) = &Loader::LOADER_Gliss_PowerPC;
	CACHE_CONFIG(props) = &cache_conf;
	RECURSIVE(props) = true;
	NO_SYSTEM(props) = true;
	PROCESSOR_PATH(props) = "../../data/procs/op1.xml";
	SIMULATOR(props) = &gensim_simulator;
	
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
		
		// Prepare processor configuration
		PropList props;
		EXPLICIT(props) = true;
		PROC_VERBOSE(props) = true;
		
		// Compute BB times
		/*TrivialBBTime tbt(5, props);
		tbt.process(fw);*/
		BBTimeSimulator bbts(props);
		bbts.process(fw);
		
		// Trivial data cache
		TrivialDataCacheManager dcache(props);
		dcache.process(fw);
		
		// Assign variables
		VarAssignment assign(props);
		assign.process(fw);
		
		// Build the system
		BasicConstraintsBuilder builder(props);
		builder.process(fw);
		
		// Build the object function to maximize
		BasicObjectFunctionBuilder fun_builder(props);
		fun_builder.process(fw);
		
		// Load flow facts
		ipet::FlowFactLoader loader(props);
		loader.process(fw);
		
		// Resolve the system
		WCETComputation wcomp(props);
		wcomp.process(fw);
		
		// Display the result
		cfg = ENTRY_CFG(fw);
		ilp::System *sys = SYSTEM(cfg);
		sys->dump();
		cout << sys->countVars() << " variables and "
			 << sys->countConstraints() << " constraints.\n";
		cout << "SUCCESS\nWCET = " << WCET(cfg) << '\n';
	}
	catch(elm::Exception& e) {
		cerr << "ERROR: " << e.message() << '\n';
	}
	return 0;
}

