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
			cout << (bb->number() - 1)
				 << '(' << fmt::address(bb->address()) << ')';
	}
	cout << "}\n";
	
	// Display children
	for(ContextTree::ChildrenIterator child(tree); child; child++)
		displayContextTree(child, space + 1);
}

int main(int argc, char **argv) {

	//Manager manager;
	PropList props;
	//NO_SYSTEM(props) = true;
	//Processor::VERBOSE(props) = true;
	String fun = "main";
	if(argc > 2)
		fun = argv[2];
	try {
		WorkSpace *fw = MANAGER.load(argv[1], props);
		
		// Find searched CFG
		cout << "Looking for CFG " << fun << '\n';
		CFG *cfg = fw->getCFGInfo()->findCFG(fun);
		if(cfg == 0) {
			cerr << "ERROR: cannot find " << fun << " !\n";
			return 1;
		}
		else
			cout << fun << " found at 0x" << fmt::address(cfg->address()) << '\n';

		// Removing __eabi call if available
		bool found = false;
		for(CFG::BBIterator bb(cfg); bb; bb++)
			for(BasicBlock::OutIterator edge(bb); edge; edge++)
				if(edge->kind() == Edge::CALL
				&& edge->target()
				&& edge->calledCFG()->label() == "__eabi") {
					delete(*edge);
					//cerr << "__eabi call deleted !\n";
					found = true;
					break;
				}
		if(!found)
			cerr << "__eabi call not found !\n";

		// Build the context
		PropList conf;
		//Processor::VERBOSE(conf) = true;
		RECURSIVE(conf) = true;
		TASK_ENTRY(conf) = fun.toCString();
		ContextTreeBuilder builder;
		builder.process(fw, conf);
		ContextTree *ct = CONTEXT_TREE(fw);
		
		// Display dominance information
		cout << "\nDOMINANCE\n";
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
		
		// Display headers
		cout << "\nLOOP HEADERS\n";
		for(CFG::BBIterator bb(cfg); bb; bb++)
			if(LOOP_HEADER(bb))
				cout << "- " << bb->number()
					 << " (" << fmt::address(bb->address()) << ")\n";
				
		
		// Display context tree
		cout << "\nCONTEXT TREE\n";
		displayContextTree(ct);
		
		// Display the result
		cout << "SUCCESS\n";
	}
	catch(elm::Exception& e) {
		cerr << "ERROR: " << e.message() << '\n';
		exit(1);
	}
	return 0;
}

