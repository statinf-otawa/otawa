/*
 *	CFGTransformer processor implementation
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

#include <otawa/cfg/CFGTransformer.h>
#include <otawa/prog/WorkSpace.h>

namespace otawa {

using namespace elm;

/**
 * @class CFGTransformer
 * CFGtranformer is not a transformer by itself but an helper for processor
 * that transform the CFG. It proposes a framework for, starting from
 * an existing CFG, build another with lot of automatic facilities.
 * Basically, it builds a clone of the current CFGs but this process
 * may be customized in different ways by overriding a set of
 * dedicated methods.
 *
 * @ingroup cfg
 */

p::declare CFGTransformer::reg = p::init("otawa::CFGTransformer", Version(1, 0, 0))
	.maker<CFGTransformer>()
	.require(otawa::COLLECTED_CFG_FEATURE)
	.invalidate(otawa::COLLECTED_CFG_FEATURE)
	.provide(otawa::COLLECTED_CFG_FEATURE);

/**
 */
CFGTransformer::CFGTransformer(p::declare& r): entry(0), cur(0) {
}


/**
 * Get the maker representing the given CFG (possibly creating one).
 * @param cfg	CFG to look a maker for.
 * @return		Maker representing the CFG.
 */
CFGMaker *CFGTransformer::get(CFG *cfg) {
	CFGMaker *m = cmap.get(cfg, 0);
	if(!m) {
		m = new CFGMaker(cfg->first());
		cmap.put(cfg, m);
		wl.put(pair(cfg, m));
	}
	return m;
}


/**
 */
void CFGTransformer::processWorkSpace(WorkSpace *ws) {
	const CFGCollection *coll = otawa::INVOLVED_CFGS(ws);
	ASSERT(coll);

	// initialize working list
	entry = coll->entry();
	wl.put(pair(entry, get(entry)));

	// process each CFG in turn
	while(wl) {
		Pair<CFG *, CFGMaker *> p = wl.get();
		makeCFG(p.fst, p.snd);
	}
}


/**
 * Build a CFG. May be overridden to control the way the CFG
 * is built.
 * @notice 		When this method is called, a current CFG maker
 * 				is stored and will be used by block and edge building methods.
 * @param cfg	CFG to build.
 * @param maker	Maker to use.
 */
void CFGTransformer::makeCFG(CFG *cfg, CFGMaker *maker) {
	setCurrent(maker);
	clone(cfg);
}

/**
 * Build a clone of the current CFG.
 * @param cfg	CFG to clone.
 * @param maker	Maker to use.
 */
void CFGTransformer::clone(CFG *cfg) {
	bmap.clear();

	// clone blocks
	for(CFG::BlockIter b = cfg->blocks(); b; b++) {
		Block *nb = makeBlock(cfg, b);
		if(nb)
			bmap.put(b, nb);
	}

	// clone edges
	for(CFG::BlockIter src = cfg->blocks(); src; src++) {
		Block *nsrc = bmap.get(src, 0);
		if(nsrc) {
			for(Block::EdgeIter e = src->outs(); e; e++) {
				Block *nsnk = bmap.get(e->sink(), 0);
				if(nsnk)
					makeEdge(cfg, e);
			}
		}
	}
}


/**
 * Called to build a block.
 * @param cfg	Original CFG.
 * @param b		Original block.
 * @return		New block or null (if the block has been removed).
 */
Block *CFGTransformer::makeBlock(CFG *cfg, Block *b) {
	return clone(b);
}


/**
 * Build a new edge from an original edge.
 * @param cfg	Owner CFG.
 * @param edge	Original edge.
 * @return		New edge or null if the edge must be removed.
 */
Edge *CFGTransformer::makeEdge(CFG *cfg, Edge *edge) {
	return clone(edge);
}


/**
 * Build a new edge with the same characteristics as the given one
 * with new source and sink.
 * @param src	Source vertex.
 * @param edge	Edge to copy.
 * @param snk	Sink vertex.
 * @return		Built edge.
 */
Edge *CFGTransformer::makeEdge(Block *src, Edge *edge, Block *snk) {
	Edge *r = new Edge(edge->flags());
	cur->add(src, snk, r);
	return r;
}


/**
 * Clone the given block and it to the built CFG.
 * @param b		Block to clone.
 * @return		Cloned block.
 */
Block *CFGTransformer::clone(Block *b) {

	// end case
	if(b->isEnd()) {
		if(b->isEntry())
			return cur->entry();
		else if(b->isExit())
			return cur->exit();
		else if(b->isUnknwon())
			return cur->unknown();
		else {
			ASSERT(false);
			return 0;
		}
	}

	// synthetic block case
	else if(b->isSynth()) {
		return make(b->toSynth()->callee());
	}

	// basic block
	else {
		BasicBlock *bb = b->toBasic();
		genstruct::Vector<Inst *> insts(bb->count());
		for(BasicBlock::InstIter i = bb; i; i++)
			insts.add(*i);
		return make(insts.detach());
	}
}


/**
 * Clone an edge and add it to the built CFG.
 * @param e		Edge to clone.
 * @return		Cloned edge.
 */
Edge *CFGTransformer::clone(Edge *e) {
	Edge *ne = new Edge(e->flags());
	cur->add(bmap.get(e->source()), bmap.get(e->sink()), e);
	return ne;
}


/**
 * Build a basic block starting at the given instruction
 * and spanning over n instructions and add it to the built CFG.
 * @param i		First basic block instruction.
 * @param n		Number of instruction.
 * @return		Built basic block.
 */
BasicBlock *CFGTransformer::make(Inst *i, int n) {
	genstruct::Vector<Inst *> is(n);
	while(n) {
		is.add(i);
		n--;
		i = i->nextInst();
	}
	BasicBlock *bb = new BasicBlock(is.detach());
	cur->add(bb);
	return bb;
}


/**
 * Build a basic from a table of instructions and add it to the built CFG.
 * Notice that owner of the table is now the basic block.
 * @param is	Instruction table.
 * @return		Built basic block.
 */
BasicBlock *CFGTransformer::make(genstruct::Table<Inst *> is) {
	BasicBlock *bb = new BasicBlock(is);
	cur->add(bb);
	return bb;
}


/**
 * Build a synthetic block calling the given CFG
 * and it to the built CFG.
 * @param callee	Callee CFG maker.
 * @return			Built block.
 */
SynthBlock *CFGTransformer::make(CFGMaker *callee) {
	SynthBlock *nb = new SynthBlock();
	cur->call(nb, *callee);
	return nb;

}


/**
 * Build a synthetic block calling the given CFG and add it to the built CFG.
 * @param callee	Callee CFG or null (for unknown call).
 * @return			Built block.
 */
SynthBlock *CFGTransformer::make(CFG *callee) {
	CFGMaker *m;
	if(!callee)
		m  = 0;
	else
		m = get(callee);
	SynthBlock *nb = new SynthBlock();
	cur->call(nb, *m);
	return nb;

}


/**
 */
void CFGTransformer::cleanup(WorkSpace *ws) {
	CFGCollection *coll = new CFGCollection();
	CFGMaker *em = cmap.get(entry);
	CFG *nentry = em->build();
	coll->add(nentry);
	for(genstruct::HashTable<CFG *, CFGMaker *>::PairIterator c(cmap); c; c++) {
		if((*c).fst != entry)
			coll->add((*c).snd->build());
		delete (*c).snd;
	}

	addRemover(COLLECTED_CFG_FEATURE, ENTRY_CFG(ws) = nentry);
	track(COLLECTED_CFG_FEATURE, INVOLVED_CFGS(ws) = coll);
}

}	// otawa



