/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/dumpcfg/Displayer.h -- Displayer class interface.
 */
#ifndef OTAWA_DUMPCFG_DISPLAYER_H
#define OTAWA_DUMPCFG_DISPLAYER_H

#include <otawa/prog/WorkSpace.h>
#include <otawa/cfg.h>

using namespace otawa;

// Displayer class
class Displayer {
public:
	bool display_assembly;
	bool source_info;
	virtual ~Displayer(void) { }
	virtual void onProgramBegin(WorkSpace *ws) { }
	virtual void onProgramEnd(WorkSpace *ws) { }
	virtual void onCFGBegin(otawa::CFG *cfg) = 0;
	virtual void onCFGEnd(otawa::CFG *cfg) = 0;
	virtual void onBBBegin(otawa::BasicBlock *bb, int index) = 0;
	virtual void onEdge(otawa::CFGInfo *info, otawa::BasicBlock *source,
		int source_index, otawa::edge_kind_t kind, otawa::BasicBlock *target,
		int target_index) = 0;
	virtual void onBBEnd(otawa::BasicBlock *bb, int index) = 0;
	virtual void onCall(otawa::Edge *edge) = 0;
	virtual void onInlineBegin(otawa::CFG *cfg) = 0;
	virtual void onInlineEnd(otawa::CFG *cfg) = 0;
};

#endif	// OTAWA_DUMPCFG_DISPLAYER_H

