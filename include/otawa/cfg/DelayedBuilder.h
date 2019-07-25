/*
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

#include <otawa/cfg/CFGTransformer.h>
#include <otawa/cfg/Edge.h>
#include <otawa/cfg/features.h>

namespace otawa {

class DelayedCleaner;
class Inst;
class Process;
class VirtualCFG;

class DelayedBuilder: public CFGTransformer {
public:
	static p::declare reg;
	DelayedBuilder(void);

protected:
	virtual void processWorkSpace(WorkSpace *ws);
	virtual void setup(WorkSpace *ws);
	virtual void cleanup(WorkSpace *ws);
	virtual void transform(CFG *g, CFGMaker& m);

private:
	void cloneEdge(Edge *edge, Block *source, t::uint32 flags);
	void insert(Edge *edge, BasicBlock *ibb);
	BasicBlock *makeBB(Inst *inst, int n = 1);
	BasicBlock *makeNOp(Inst *inst, int n = 1);
	void buildBB(CFG *cfg, CFGMaker& maker);
	void buildEdges(CFG *cfg, CFGMaker& maker);
	delayed_t type(Inst *inst);
	int count(Inst *inst);
	ot::size size(Inst *inst, int n = 1);
	Inst *next(Inst *inst, int n = 1);
	Edge *makeEdge(Block *src, Block *tgt, t::uint32 flags);
	void mark(CFG *cfg);

	DelayedCleaner *cleaner;
	DelayedInfo *info;
};

} // otawa


#endif /* OTAWA_DELAYEDBUILDER_H_ */
