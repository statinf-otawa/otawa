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

#include <elm/data/FragTable.h>
#include <elm/data/ListQueue.h>
#include <elm/data/HashMap.h>
#include <otawa/cfg/features.h>
#include <otawa/proc/Processor.h>


namespace otawa {

using namespace elm;

class CFGTransformer: public Processor {
public:
	static p::declare reg;
	CFGTransformer(p::declare& r = reg);
	void *interfaceFor(const AbstractFeature& f) override;

protected:
	void processWorkSpace(WorkSpace *ws) override;
	void cleanup(WorkSpace *ws) override;
	void destroy(WorkSpace *ws) override;
	void commit(WorkSpace *ws) override;

	BasicBlock *build(Inst *inst, int n);
	BasicBlock *build(Array<Inst *> insts);
	SynthBlock *build(CFGMaker *callee);
	SynthBlock *build(CFG *callee);
	PhonyBlock *build();
	Edge *build(Block *src, Block *snk, t::uint32 flags);

	virtual Block *clone(Block *b);
	virtual Edge *clone(Block *src, Edge *edge, Block *snk);

	virtual void transform(CFG *g, CFGMaker& m);
	virtual Block *transform(Block *b);
	virtual Edge *transform(Edge *e);
	void map(Block *ob, Block *nb);
	bool isMapped(Block *b);
	CFGMaker *get(CFG *cfg);
	Block *get(Block *b);
	void install(CFG *cfg, CFGMaker& maker);
	CFGMaker& add(CFG *cfg);
	void add(CFG *cfg, CFGMaker& maker);

	inline CFG *entry(void) const { return _entry; }
	inline void setNoUnknown(bool v) { no_unknown = v; }
	inline bool getNoUnknown(void) const { return no_unknown; }

private:
	CFG *_entry;
	ListQueue<Pair<CFG *, CFGMaker *> > wl;
	CFGMaker *cur;
	FragTable<CFGMaker *> makers;
	HashMap<CFG *, CFGMaker *> cmap;
	HashMap<Block *, Block *> bmap;
	bool no_unknown;
	CFGCollection *coll;
};

}	// otawa

#endif // OTAWA_CFG_TRANSFORMER_H
