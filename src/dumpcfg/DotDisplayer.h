/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/dumpcfg/DotDisplayer.h -- DotDisplayer class interface.
 */
#ifndef OTAWA_DUMPCFG_DOT_DISPLAYER_H
#define OTAWA_DUMPCFG_DOT_DISPLAYER_H

#include "Displayer.h"

// SimpleDisplayer class
class DotDisplayer: public Displayer {
public:
	virtual void onProgramBegin(WorkSpace *_ws) { ws = _ws; }
	virtual void onCFGBegin(otawa::CFG *cfg);
	virtual void onCFGEnd(otawa::CFG *cfg);
	virtual void onBBBegin(otawa::BasicBlock *bb, int index);
	virtual void onEdge(otawa::CFGInfo *info, otawa::BasicBlock *source,
		int source_index, otawa::edge_kind_t kind, otawa::BasicBlock *target,
		int target_index);
	virtual void onBBEnd(otawa::BasicBlock *bb, int index);
	virtual void onInlineBegin(otawa::CFG *cfg);
	virtual void onInlineEnd(otawa::CFG *cfg);
	virtual void onCall(otawa::Edge *edge);

private:
	WorkSpace *ws;
	void displayLabel(otawa::BasicBlock *bb, int index);
};

#endif	// OTAWA_DUMPCFG_DOT_DISPLAYER_H

