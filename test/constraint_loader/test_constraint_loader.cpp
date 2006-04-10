/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	test/ipet/test_constraint_builder.cpp -- test for ConstraintBuilder feature.
 */

#include <stdlib.h>
#include <elm/io.h>
#include <otawa/otawa.h>
#include <otawa/ipet.h>

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
			
		// Compute BB times
		cout << "Timing the BB\n";
		TrivialBBTime tbt(5);
		tbt.processCFG(fw, cfg);
		
		// Assign variables
		cout << "Numbering the main\n";
		VarAssignment assign;
		assign.processCFG(fw, cfg);
		
		// Build the system
		cout << "Building the ILP system\n";
		BasicConstraintsBuilder builder;
		builder.processCFG(fw, cfg);
		
		// Get external constraints
		cout << "Loading external constraints\n";
		ConstraintLoader loader;
		loader.processCFG(fw, cfg);
		
		// Resolve the system
		cout << "Resolve the system\n";
		WCETComputation wcomp;
		wcomp.processCFG(fw, cfg);
		cfg->use<ilp::System *>(IPET::ID_System)->dump();
		
		// Display the result
		cout << "SUCCESS\nWCET = " << cfg->use<int>(IPET::ID_WCET) << '\n';
	}
	catch(LoadException e) {
		cerr << "ERROR: " << e.message() << '\n';
		exit(1);
	}
	return 0;
}

