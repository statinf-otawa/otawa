/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	examples/dumpcfg/DotDisplayer.cpp -- DotDisplayer class implementation.
 */

#include <elm/io.h>
#include "DotDisplayer.h"

using namespace elm;
using namespace otawa;


/**
 * Display the name of a basic block.
 */
void DotDisplayer::displayRef(int index) {
	/*if(bb->isEntry())
		cout << "ENTRY";
	else if(bb->isExit())
		cout << "EXIT";
	else*/
		cout << '"' << index << '"';
}


/**
 */
void DotDisplayer::onCFGBegin(CFG *cfg) {
	cout << "digraph " << cfg->label() << "{\n";
}


/**
 */
void DotDisplayer::onCFGEnd(CFG *cfg) {
	cout << "}\n";
}


/**
 */
void DotDisplayer::onBBBegin(BasicBlock *bb, int index) {
	cout << "\t";
	displayRef(index);
	cout << " [label=\"" << index
		 << " (" << bb->address() << ")\"]\n";
}


/**
 */
void DotDisplayer::onEdge(CFGInfo *info, BasicBlock *source, int source_index,
edge_kind_t kind, BasicBlock *target, int target_index) {
	if(target_index >= 0) {
		cout << '\t';
		displayRef(source_index);
		cout << " -> ";
		displayRef(target_index);
		switch(kind) {
		case Edge::VIRTUAL:
		case Edge::CALL:
			cout << " [style=dashed]";
			break;
		}
		cout << ";\n";
	}
}


/**
 */
void DotDisplayer::onBBEnd(BasicBlock *bb, int index) {
}


/**
 * Handle an inline of a program call.
 */
void DotDisplayer::onInlineBegin(CFG *cfg) {
}


/**
 * Handle an end of inline.
 */
void DotDisplayer::onInlineEnd(CFG *cfg) {
}

