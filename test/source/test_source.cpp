/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	test/ipet/test_hard.cpp -- test for hardware features.
 */

#include <stdlib.h>
#include <elm/io.h>
#include <otawa/otawa.h>

using namespace elm;
using namespace otawa;
using namespace otawa::hard;

int main(int argc, char **argv) {
	
	Manager manager;
	PropList props;
	
	try {
		
		// Load program
		if(argc < 2) {
			cerr << "ERROR: no argument.\n"
				 << "Syntax is : test_ipet <executable>\n";
			exit(2);
		}
		WorkSpace *ws = manager.load(argv[1], props);
		ASSERT(ws);
		
		// From address
		Inst *inst = ws->process()->findInstAt("main");
		ASSERT(inst);
		Option<Pair<cstring, int> > line = ws->process()->getSourceLine(inst->address());
		ASSERT(line);
		cout << inst->address() << " = " << (*line).fst << ":" << (*line).snd << io::endl;
		
		// Look back for the address
		Vector<Pair<Address, Address> > addresses;
		ws->process()->getAddresses((*line).fst, (*line).snd, addresses);
		cout << "BACK\n";
		for(int i = 0; i < addresses.length(); i++)
			cout << '\t' << addresses[i].fst << '-' << addresses[i].snd << io::endl; 
		
	}
	catch(elm::Exception& e) {
		cerr << "ERROR: " << e.message() << '\n';
		exit(1);
	}
	return 0;
}

