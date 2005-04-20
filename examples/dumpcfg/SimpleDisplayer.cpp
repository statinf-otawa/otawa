/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	examples/dumpcfg/SimpleDisplayer.cpp -- SimpleDisplayer class implementation.
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
void SimpleDisplayer::onBBBegin(BasicBlock *bb, int index) {
	cout << index
		<< ' ' << bb->address()
		<< ' ' << (bb->address() + bb->getBlockSize() - 4);
}


/**
 */
void SimpleDisplayer::onEdge(BasicBlock *source, int source_index,
edge_kind_t kind, BasicBlock *target, int target_index) {
	cout << ' ' << target_index;
}


/**
 */
void SimpleDisplayer::onBBEnd(BasicBlock *bb, int index) {
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

