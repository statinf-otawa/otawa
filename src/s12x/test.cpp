/*
 *	$Id$
 *	Copyright (c) 2003-06, IRIT UPS.
 *
 *	test/ipet/test_ipet.cpp -- test for IPET feature.
 */

#include <elm/io.h>
#include <otawa/otawa.h>
#include <otawa/ipet.h>
#include <otawa/ipet/BBTimeSimulator.h>
#include <otawa/display/CFGDrawer.h>
#include <otawa/cfg/Virtualizer.h>

using namespace elm;
using namespace otawa;
using namespace otawa::ipet;

int main(int argc, char **argv) {
	
	// Configuration
	PropList props;
	EXPLICIT(props) = true;
	otawa::Processor::VERBOSE(props) = true;
	
	try {
		
		// Load program
		if(argc < 2) {
			cerr << "ERROR: no argument.\n"
				 << "Syntax is : test_s12x <executable>\n";
			return 2;
		}
		WorkSpace *ws = MANAGER.load(argv[1], props);
		
		// WCET computation
		Virtualizer virt;
		virt.process(ws, props);
		BBTimeSimulator bbts;
		bbts.process(ws, props);		
		WCETComputation wcomp;
		wcomp.process(ws, props);
		
		// Record and display result
		CFG *cfg = ENTRY_CFG(ws);
		WCETCountRecorder recorder;
		recorder.process(ws, props);
		display::CFGDrawer drawer(cfg, props);
		drawer.display();				

		// Display the result
		ilp::System *sys = SYSTEM(ws);
		cout << "SUCCESS\nWCET = " << WCET(ws) << '\n';
		
	}
	catch(elm::Exception& e) {
		cerr << "ERROR: " << e.message() << '\n';
	}
	return 0;
}

