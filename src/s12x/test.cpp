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
#include <otawa/util/Dominance.h>
#include <otawa/dfa/BitSet.h>

using namespace elm;
using namespace otawa;
using namespace otawa::ipet;

int main(int argc, char **argv) {
	
	// Configuration
	PropList props;
	//EXPLICIT(props) = true;
	otawa::Processor::VERBOSE(props) = true;
	if(argc > 2)
		TASK_ENTRY(props) = argv[2];
	
	try {
		
		// Load program
		if(argc < 2) {
			cerr << "ERROR: no argument.\n"
				 << "Syntax is : test_s12x <executable>\n";
			return 2;
		}
		WorkSpace *ws = MANAGER.load(argv[1], props);
		
		// WCET computation
		/*Virtualizer virt;
		virt.process(ws, props);*/

		// !!DEBUG!!
		/*Dominance dom;
		dom.process(ws, props);
		for(CFG::BBIterator bb(ENTRY_CFG(ws)); bb; bb++) {
			cerr << "CHECK " << bb->number() << " " << REVERSE_DOM(bb)->size() << io::endl;
			for(BasicBlock::InIterator edge(bb); edge; edge++)
	 			if(bb != edge->source()
	 			&& Dominance::dominates(bb, edge->source())
	 			&& Dominance::dominates(edge->source(), bb)) {
	 				cerr << *REVERSE_DOM(bb) << io::endl;
	 				cerr << *REVERSE_DOM(edge->source()) << io::endl;
	 				ASSERT(false);
	 			}
		}*/

		BBTimeSimulator bbts;
		bbts.process(ws, props);		
		WCETComputation wcomp;
		wcomp.process(ws, props);
		
		// Record and display result
		/*CFG *cfg = ENTRY_CFG(ws);
		WCETCountRecorder recorder;
		recorder.process(ws, props);
		display::CFGDrawer drawer(cfg, props);
		drawer.display();*/			

		// Display the result
		ilp::System *sys = SYSTEM(ws);
		cout << "SUCCESS\nWCET = " << WCET(ws) << '\n';
		
	}
	catch(elm::Exception& e) {
		cerr << "ERROR: " << e.message() << '\n';
	}
	return 0;
}

