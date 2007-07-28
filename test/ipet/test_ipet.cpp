/*
 *	$Id$
 *	Copyright (c) 2003-06, IRIT UPS.
 *
 *	test/ipet/test_ipet.cpp -- test for IPET feature.
 */

#include <elm/io.h>
#include <otawa/otawa.h>
#include <otawa/ipet.h>
#include <otawa/proc/ProcessorException.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/ilp.h>
#include <otawa/ipet/BBTimeSimulator.h>
#include <otawa/gensim/GenericSimulator.h>
#include <otawa/proc/Registry.h>

//#define WITH_VIRTUAL
//#define REGISTRY

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
			return 2;
		}
		WorkSpace *fw = manager.load(argv[1], props);
		
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
		
		// WCET computation
		PropList props;
		EXPLICIT(props) = true;
		otawa::Processor::VERBOSE(props) = true;
		ipet::ILP_PLUGIN_NAME(props) = "lp_solve5";		
		
		TrivialDataCacheManager dcache;
		dcache.process(fw, props);
		
		WCETComputation wcomp;
		wcomp.process(fw, props);
		
		// Display the result
		cfg = ENTRY_CFG(fw);
		ilp::System *sys = SYSTEM(fw);
		sys->dump();
		cout << sys->countVars() << " variables and "
			 << sys->countConstraints() << " constraints.\n";
		cout << "SUCCESS\nWCET = " << WCET(fw) << '\n';
		
		// Display registrations
		#ifdef REGISTRY
			cout << "\nREGISTRATIONS\n";
			for(Registry::Iter reg; reg; reg++) {
				cout << reg->name() << " " << reg->version()
					 << " (" << reg->help() << ")" << io::endl;
				for(Registration::ConfigIter conf(reg); conf; conf++)
					cout << "\t" << conf->id().name() << " (" << conf->help() << ")\n"; 
			}
		#endif
	}
	catch(elm::Exception& e) {
		cerr << "ERROR: " << e.message() << '\n';
	}
	return 0;
}

