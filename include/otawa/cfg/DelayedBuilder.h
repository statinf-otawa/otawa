/*
 *	$Id$
 *	DelayedBuilder
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2008, IRIT UPS.
 *
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef OTAWA_DELAYEDBUILDER_H_
#define OTAWA_DELAYEDBUILDER_H_

#include <elm/genstruct/SLList.h>
#include <otawa/proc/CFGProcessor.h>
#include <otawa/cfg/Edge.h>
#include <otawa/cfg/features.h>

namespace otawa {

class DelayedCleaner;
class Inst;
class Process;
class VirtualCFG;

class DelayedBuilder: public CFGProcessor {
public:
	static Registration<DelayedBuilder> reg;
	DelayedBuilder(void);

protected:
	virtual void setup(WorkSpace *ws);
	virtual void processWorkSpace(WorkSpace *ws);
	virtual void processCFG(WorkSpace *ws, CFG *cfg);
	virtual void cleanup(WorkSpace *ws);

private:
	typedef genstruct::HashTable<BasicBlock *, BasicBlock *> map_t;
	void fix(Edge *oedge, Edge *nedge);
	void cloneEdge(Edge *edge, BasicBlock *source, Edge::kind_t kind);
	void insert(Edge *edge, BasicBlock *ibb);
	BasicBlock *makeBB(Inst *inst, int n = 1);
	BasicBlock *makeNOp(Inst *inst, int n = 1);
	void buildBB(CFG *cfg);
	void buildEdges(CFG *cfg);
	delayed_t type(Inst *inst);
	int count(Inst *inst);
	ot::size size(Inst *inst, int n = 1);
	Inst *next(Inst *inst, int n = 1);
	Edge *makeEdge(BasicBlock *src, BasicBlock *tgt, Edge::kind_t kind);
	void mark(CFG *cfg);

	CFGCollection *coll;
	DelayedCleaner *cleaner;
	genstruct::HashTable<CFG *, VirtualCFG *> cfg_map;
	map_t map;
	VirtualCFG *vcfg;
	DelayedInfo *info;
};

} // otawa


#endif /* OTAWA_DELAYEDBUILDER_H_ */
