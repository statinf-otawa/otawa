/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	src/dumpcfg/SimpleDisplayer.cpp -- SimpleDisplayer class implementation.
 */

#include <elm/io.h>
#include <otawa/cfg/features.h>
#include "SimpleDisplayer.h"

using namespace elm;
using namespace otawa;

/**
 */
Identifier<int> SimpleDisplayer::OFFSET("", -1);

/**
 */
SimpleDisplayer::SimpleDisplayer(void): Displayer("SimpleDisplayer", Version(1, 0, 0)), cfg_cnt(0) {
	require(otawa::COLLECTED_CFG_FEATURE);
}

/**
 */
void SimpleDisplayer::processWorkSpace(WorkSpace *ws) {
	const CFGCollection& coll = **otawa::INVOLVED_CFGS(ws);

	// process CFGs
	for(int i = 0; i < coll.count(); i++) {
		if(!display_all && i > 0)
			break;

		// get next CFG
		CFG *cfg = coll[i];
		int off = offset(cfg);

		// put header
		cout << "!" << cfg->name() << io::endl;

		// generate blocks
		for(CFG::BlockIter v(cfg->blocks()); v(); v++)
			if(v->isBasic()) {
				BasicBlock *b = **v;
				cout << (b->index() + off - 1) << ' ' << b->address() << ' ' << b->last()->address();

				// look leaving edges
				for(Block::EdgeIter e = b->outs(); e(); e++) {
					Block *w = e->sink();
					if(w->isBasic())
						cout << ' ' << (w->index() + off - 1);
					else if(w->isSynth())
						cout << ' ' << offset(w->toSynth()->callee());
				}

				cout << " -1\n";
			}
	}
}

/**
 * Get offset (in basic block count) of the CFG.
 * @param cfg	CFG to get offset for.
 * @return		CFG offset.
 */
int SimpleDisplayer::offset(CFG *cfg) {
	int off = OFFSET(cfg);
	if(off < 0) {

		// count real BB
		int cnt = 0;
		for(CFG::BlockIter v(cfg->blocks()); v(); v++)
			if(v->isBasic())
				cnt++;

		// compute offset
		off = cfg_cnt;
		OFFSET(cfg) = off;
		cfg_cnt += cnt;
	}
	return off;
}


#if	0
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
void SimpleDisplayer::onBBBegin(BasicBlock *bb) {
	if(!bb->isEntry() && !bb->isExit())
		cout << bb->index()
			 << ' ' << Address(bb->address())
			 << ' ' << Address(bb->last()->address());
}


/**
 */
void SimpleDisplayer::onEdge(Edge *edge) {
	int target_index = edge->target()->index();
	if(!edge->source()->isEntry()
	&& !edge->source()->isExit()
	&& target_index >= 0)
			cout << ' ' << target_index;
}


/**
 */
void SimpleDisplayer::onBBEnd(BasicBlock *bb) {
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
#endif

