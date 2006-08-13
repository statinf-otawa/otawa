/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	test/ct/test_ct.cpp -- test for context tree feature.
 */

#include <stdlib.h>
#include <elm/io.h>
#include <otawa/otawa.h>
#include <otawa/util/Dominance.h>
#include <otawa/util/ContextTree.h>

using namespace otawa;
using namespace elm;

static void displayContextTree(ContextTree *tree, int space = 0) {
	
	// Display spaces
	for(int i = 0; i < space; i++)
		cout << "  ";
	
	// Display tree
	switch(tree->kind()){
	case ContextTree::LOOP:
		cout << "LOOP at " << fmt::address(tree->bb()->address()) << " ";
		break;
	case ContextTree::ROOT:
		cout << "ROOT ";
		break;
	case ContextTree::FUNCTION:
		cout << "FUNCTION to " << tree->cfg()->label() << " ";
		break;
	default:
		assert(false);
	}
	
	// Display BB
	cout << "{";
	bool first = true;
	for(Iterator<BasicBlock *> bb(tree->bbs()); bb; bb++) {
		if(first)
			first = false;
		else
			cout << ", ";
		if(bb->isEntry())
			cout << "ENTRY";
		else if(bb->isExit())
			cout << "EXIT";
		else
			cout << (bb->use<int>(CFG::ID_Index) - 1)
				 << '(' << fmt::address(bb->address()) << ')';
	}
	cout << "}\n";
	
	// Display children
	for(ContextTree::ChildrenIterator child(tree); child; child++)
		displayContextTree(child, space + 1);
}

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
			cout << "main found at 0x" << fmt::address(cfg->address()) << '\n';
		
		// Removing __eabi call if available
		for(CFG::BBIterator bb(cfg); bb; bb++)
			for(BasicBlock::OutIterator edge(bb); edge; edge++)
				if(edge->kind() == Edge::CALL
				&& edge->target()
				&& edge->calledCFG()->label() == "__eabi") {
					delete(*edge);
					break;
				}
		
		// Build dominance
		Dominance dom;
		dom.processCFG(fw, cfg);
		
		// Display dominance information
		cout << "\nDOMINANCE\n";
		for(CFG::BBIterator bb1(cfg); bb1; bb1++) {
			bool first = true;
			cout << bb1->use<int>(CFG::ID_Index) << " dominates {";
			for(CFG::BBIterator bb2(cfg); bb2; bb2++)
				if(Dominance::dominates(bb1, bb2)) {
					if(first)
						first = false;
					else
						cout << ", ";
					cout << bb2->use<int>(CFG::ID_Index);
				}
			cout << "}\n";
		}
		
		// Display headers
		Dominance::markLoopHeaders(cfg);
		cout << "\nLOOP HEADERS\n";
		for(CFG::BBIterator bb(cfg); bb; bb++)
			if(bb->get<bool>(Dominance::ID_LoopHeader, false))
				cout << "- " << bb->use<int>(CFG::ID_Index)
					 << " (" << fmt::address(bb->address()) << ")\n";
				
		
		// Build the context
		ContextTree *ct = new ContextTree(cfg);
		cout << "\nCONTEXT TREE\n";
		displayContextTree(ct);
		
		// Display the result
		cout << "SUCCESS\n";
	}
	catch(LoadException e) {
		cerr << "ERROR: " << e.message() << '\n';
		exit(1);
	}
	return 0;
}

