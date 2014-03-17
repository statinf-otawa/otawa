/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	src/dumpcfg/SimpleDisplayer.cpp -- SimpleDisplayer class implementation.
 */

#include <elm/io.h>
#include "SimpleDisplayer.h"

using namespace elm;
using namespace otawa;

/**
 */
void SimpleDisplayer::onCFGBegin(CFG *cfg) {
	cout << '!' << cfg->label() << '\n';
}


/**
 */
void SimpleDisplayer::onCFGEnd(CFG *cfg) {
	cout << '\n';
}

/**
 */
void SimpleDisplayer::onCall(Edge *edge) {
}

/**
 */
void SimpleDisplayer::onBBBegin(BasicBlock *bb, int index) {
	if(!bb->isEntry() && !bb->isExit())
		cout << index - 1
			 << ' ' << ot::address(bb->address())
			 << ' ' << ot::address(bb->address() + bb->getBlockSize() - 4);
}


/**
 */
void SimpleDisplayer::onEdge(CFGInfo *info, BasicBlock *source, int source_index,
edge_kind_t kind, BasicBlock *target, int target_index) {
	if(!source->isEntry() && !source->isExit())
		if(target_index >= 0)
			cout << ' ' << target_index - 1;
}


/**
 */
void SimpleDisplayer::onBBEnd(BasicBlock *bb, int index) {
	if(!bb->isEntry() && !bb->isExit())
		cout << " -1\n";
}


/**
 * Handle an inline of a program call.
 */
void SimpleDisplayer::onInlineBegin(CFG *cfg) {
	cout << "# Inlining " << cfg->label() << '\n';
}


/**
 * Handle an end of inline.
 */
void SimpleDisplayer::onInlineEnd(CFG *cfg) {
}

