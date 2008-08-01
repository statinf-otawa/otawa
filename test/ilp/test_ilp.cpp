/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	test/ipet/test_hard.cpp -- test for hardware features.
 */

#include <stdlib.h>
#include <elm/io.h>
#include <otawa/otawa.h>
#include <otawa/ipet.h>
#include <otawa/display/ILPSystemDisplayer.h>

using namespace elm;
using namespace otawa;
using namespace otawa::ipet;

int main(int argc, char **argv) {
	
	PropList props;
	Processor::VERBOSE(props) = true;
	ipet::EXPLICIT(props) = true;
	
	try {
		
		// Load program
		if(argc < 2) {
			cerr << "ERROR: no argument.\n"
				 << "Syntax is : test_ipet <executable>\n";
			exit(2);
		}
		WorkSpace *fw = MANAGER.load(argv[1], props);
		ASSERT(fw);
		
		// Compute the WCET
		WCETComputation wcet;
		wcet.process(fw, props);
		cout << "WCET = " << WCET(fw) << io::endl;
		
		// Dump the result
		display::ILPSystemDisplayer displayer;
		displayer.process(fw, props);
		
	}
	catch(elm::Exception& e) {
		cerr << "ERROR: " << e.message() << '\n';
		exit(1);
	}
	return 0;
}

