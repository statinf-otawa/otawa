/*
 *	CFGTransformer processor interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2016, IRIT UPS.
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
#ifndef OTAWA_CFG_TRANSFORMER_H
#define OTAWA_CFG_TRANSFORMER_H

#include <elm/data/ListQueue.h>
#include <elm/genstruct/HashTable.h>
#include <otawa/cfg/features.h>
#include <otawa/proc/Processor.h>


namespace otawa {

using namespace elm;

class CFGTransformer: public Processor {
public:
	static p::declare reg;
	CFGTransformer(p::declare& r = reg);

protected:
	virtual void processWorkSpace(WorkSpace *ws);
	virtual void cleanup(WorkSpace *ws);
	virtual void makeCFG(CFG *cfg, CFGMaker *maker);
	virtual Block *makeBlock(CFG *cfg, Block *b);
	virtual Edge *makeEdge(CFG *cfg, Edge *edge);

	Block *clone(Block *b);
	Edge *clone(Edge *b);
	void clone(CFG *cfg);

	BasicBlock *make(Inst *inst, int n);
	BasicBlock *make(genstruct::Table<Inst *> insts);
	SynthBlock *make(CFGMaker *callee);
	SynthBlock *make(CFG *callee);

	CFGMaker *get(CFG *cfg);
	inline void setCurrent(CFGMaker *maker) { cur = maker; }

private:
	CFG *entry;
	ListQueue<Pair<CFG *, CFGMaker *> > wl;
	CFGMaker *cur;
	genstruct::HashTable<CFG *, CFGMaker *> cmap;
	genstruct::HashTable<Block *, Block *> bmap;
};

}	// otawa

#endif // OTAWA_CFG_TRANSFORMER_H
