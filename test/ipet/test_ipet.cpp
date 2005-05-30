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
#include <otawa/ilp.h>

using namespace otawa;
using namespace elm;

int main(int argc, char **argv) {

	Manager manager;
	PropList props;
	props.set<Loader *>(Loader::ID_Loader, &Loader::LOADER_Gliss_PowerPC);
	try {
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
			for(Iterator<Edge *> edge(bb->outEdges()); edge; edge++)
				if(edge->kind() == Edge::CALL
				&& edge->target()
				&& edge->calledCFG()->label() == "__eabi") {
					delete(*edge);
					break;
				}
		
		// Now, use a VCFG
		VirtualCFG vcfg(cfg);
		
		// Compute BB times
		cout << "Timing the BB\n";
		TrivialBBTime tbt(5);
		tbt.processCFG(fw, &vcfg);
		
		// Assign variables
		cout << "Numbering the main\n";
		VarAssignment assign;
		assign.processCFG(fw, &vcfg);
		
		// Build the system
		cout << "Building the ILP system\n";
		BasicConstraintsBuilder builder;
		builder.processCFG(fw, &vcfg);
		
		// Load flow facts
		cout << "Loading flow facts\n";
		ipet::FlowFactLoader loader;
		loader.processCFG(fw, &vcfg);
		
		// Resolve the system
		cout << "Resolve the system\n";
		WCETComputation wcomp;
		wcomp.processCFG(fw, &vcfg);
		
		// Display the result
		ilp::System *sys = vcfg.use<ilp::System *>(IPET::ID_System);
		sys->dump();
		cout << "SUCCESS\nWCET = " << vcfg.use<int>(IPET::ID_WCET) << '\n';
	}
	catch(LoadException e) {
		cerr << "ERROR: " << e.message() << '\n';
		exit(1);
	}
	return 0;
}

