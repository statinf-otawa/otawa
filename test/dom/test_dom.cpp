/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	test/dom/test_dom.cpp -- test for domination feature.
 */

#include <stdlib.h>
#include <elm/io.h>
#include <otawa/otawa.h>
#include <otawa/util/Dominance.h>

using namespace otawa;
using namespace elm;

int main(int argc, char **argv) {

	Manager manager;
	PropList props;
//	LOADER(props) = &Loader::LOADER_Gliss_PowerPC;
	try {
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
		
		// Build dominance
		Dominance dom;
		dom.processCFG(fw, cfg);
		
		// Display dominance information
		for(CFG::BBIterator bb1(cfg); bb1; bb1++) {
			bool first = true;
			cout << bb1->number() << " dominates {";
			for(CFG::BBIterator bb2(cfg); bb2; bb2++)
				if(Dominance::dominates(bb1, bb2)) {
					if(first)
						first = false;
					else
						cout << ", ";
					cout << bb2->number();
				}
			cout << "}\n";
		}
		
		// Display the result
		cout << "SUCCESS\n";
	}
	catch(LoadException e) {
		cerr << "ERROR: " << e.message() << '\n';
		exit(1);
	}
	return 0;
}

