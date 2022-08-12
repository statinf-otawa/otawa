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
 * The methods if customized class are separated in 3 families:

 * @li build() methods perform several customized build of blocks, edge or CFG.
 * @li clone() methods may be overridden and are called each time a block, an edge
 * or a CFG is built.
 * @li transform() methods are called to duplicate blocks, edges and CFGs and maintain
 * a map between old CFG and new CFG; they may be overridden.
 *
 * As a basic, CFGTransformer performs a duplication of CFGs. transform() is called on
 * the CFG that, in turn, calls transform on blocks and edge. These methods record
 * mapping between old and new entities of the CFG and call clone() methods to actually
 * build CFG, blocks and edges.
 *
 * The clone methods just perform a call to build methods that performs the actual work.
 * Yet, they may be overriden to provide customized work.
 *
 * @ingroup cfg
 */

/**
 */
p::declare CFGTransformer::reg = p::init("otawa::CFGTransformer", Version(1, 1, 0))
	.make<CFGTransformer>()
	.extend<CFGProvider>()
	.require(otawa::COLLECTED_CFG_FEATURE)
	.invalidate(otawa::COLLECTED_CFG_FEATURE);


/**
 */
CFGTransformer::CFGTransformer(p::declare& r)
	: CFGProvider(r), _entry(nullptr), cur(nullptr), no_unknown(false)/*, coll(nullptr)*/
	{ }


/**
 * Build a basic block starting at the given instruction
 * and spanning over n instructions and add it to the built CFG.
 * @param i		First basic block instruction.
 * @param n		Number of instruction.
 * @return		Built basic block.
 */
BasicBlock *CFGTransformer::build(Inst *i, int n) {
	Vector<Inst *> is(n);
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
BasicBlock *CFGTransformer::build(Array<Inst *> is) {
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
SynthBlock *CFGTransformer::build(CFGMaker *callee) {
	SynthBlock *nb = new SynthBlock();
	cur->call(nb, *callee);
	return nb;
}

/**
 * Build a synthetic block calling the given CFG and add it to the built CFG.
 * @param callee	Callee CFG or null (for unknown call).
 * @return			Built block.
 */
SynthBlock *CFGTransformer::build(CFG *callee) {
	SynthBlock *nb = new SynthBlock();
	if(callee == nullptr)
		cur->add(nb);
	else
		cur->call(nb, *get(callee));
	return nb;
}

/**
 * Build a virtual block in the current CFG.
 * @return	Built virtual block.
 */
PhonyBlock *CFGTransformer::build() {
	PhonyBlock *b = new PhonyBlock();
	cur->add(b);
	return b;
}


/**
 * Build a new edge with the same characteristics as the given one
 * with new source and sink.
 * @param src	Source vertex.
 * @param edge	Edge to copy.
 * @param snk	Sink vertex.
 * @return		Built edge.
 */
Edge *CFGTransformer::build(Block *src, Block *snk, t::uint32 flags) {
	Edge *r = new Edge(flags);
	cur->add(src, snk, r);
	return r;
}

/**
 * Clone a block. Notice that, if the block is a special block
 * (entry, exit, unknown), the own block of the current made CFG are returned.
 * Notice that no entry is added to the map between old and new blocks.
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
		else if(b->isUnknown())
			return cur->unknown();
		else {
			Block *b = new PhonyBlock();
			cur->add(b);
			return b;
		}
	}

	// synthetic block case
	else if(b->isSynth())
		return build(b->toSynth()->callee());

	// basic block
	else {
		BasicBlock *bb = b->toBasic();
		Vector<Inst *> insts(bb->count());
		for(BasicBlock::InstIter i = bb; i(); i++)
			insts.add(*i);
		return build(insts.detach());
	}

}

/**
 * Clone an edge with the given new source and sink.
 * @param src		New source block.
 * @param edge		Old edge.
 * @param snk		New sink block.
 * @return			Cloned edge.
 */
Edge *CFGTransformer::clone(Block *src, Edge *edge, Block *snk) {
	return build(src, snk, edge->flags());
}

/**
 * This function is called for each CFG to transform. As a default,
 * it performs a copy of the given CFG using clone() methods
 * keeping matching between old and new blocks.
 * @param g		CFG to clone.
 * @param m		Current CFG maker.
 */
void CFGTransformer::transform(CFG *g, CFGMaker& m) {

	// transform blocks
	for(CFG::BlockIter b = g->blocks(); b(); b++) {
		Block *nb = transform(*b);
		if(nb)
			bmap.put(*b, nb);
	}

	// clone edges
	for(CFG::BlockIter src = g->blocks(); src(); src++) {
		Block *nsrc = bmap.get(*src, 0);
		if(nsrc) {
			for(Block::EdgeIter e = src->outs(); e(); e++) {
				Block *nsnk = bmap.get(e->sink(), 0);
				if(nsnk)
					transform(*e);
			}
		}
	}

	// report context
	if(g->hasProp(CONTEXT))
		CONTEXT(m) = CONTEXT(g);
}

/**
 * Transform a block of the old CFG to a block of
 * the new CFG and record matching between them.
 * @param b		Old CFG block.
 * @return		New CFG block.
 */
Block *CFGTransformer::transform(Block *b) {
	Block *nb = clone(b);
	map(b, nb);
	return nb;
}

/**
 * Transform an edge from the old CFG to an edge
 * of the new CFG. Both sources and sinks must have
 * been transformed before.
 * @param e		Old CFG edge.
 * @return		New CFG edge.
 */
Edge *CFGTransformer::transform(Edge *e) {
	return clone(bmap.get(e->source()), e, bmap.get(e->sink()));
}

/**
 * Create a matching between an old CFG block and
 * a new CFG block. If there were an existing matching
 * involved the old CFg block, it is removed.
 * @param ob	Old CFG block.
 * @param nb	New CFG block.
 */
void CFGTransformer::map(Block *ob, Block *nb) {
	bmap.put(ob, nb);
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
		makers.add(m);
		cmap.put(cfg, m);
		wl.put(pair(cfg, m));
	}
	return m;
}

/**
 * Get the block in new CFG matching the given old CFG block.
 * @param b		Old CFG block.
 * @return		New CFG block.
 */
Block *CFGTransformer::get(Block *b) {
	return bmap.get(b);
}

/**
 * Test if the given block of the original CFG
 * is mapped with a block of the new CFG.
 * @param b		Block to test.
 * @return		True if it is mapped, false else.
 */
bool CFGTransformer::isMapped(Block *b) {
	return bmap.hasKey(b);
}

/**
 */
void CFGTransformer::processWorkSpace(WorkSpace *ws) {
	const CFGCollection *coll = otawa::INVOLVED_CFGS(ws);
	ASSERT(coll);

	// initialize working list
	_entry = coll->entry();
	get(_entry);

	// process each CFG in turn
	while(wl) {
		Pair<CFG *, CFGMaker *> p = wl.get();
		if(logFor(LOG_FUN))
			log << "\ttransform " << p.fst << io::endl;
		install(p.fst, *p.snd);
		transform(p.fst, *p.snd);
	}
}

/**
 */
void CFGTransformer::cleanup(WorkSpace *ws) {
	auto coll = new CFGCollection();
	for(FragTable<CFGMaker *>::Iter m(makers); m(); m++) {
		CFG *cfg = m->build();
		coll->add(cfg);
		delete *m;
	}
	setCollection(coll);
}


/**
 * Prepare a maker to be transformed.
 * @param cfg	CFG to transform.
 * @param maker	Maker of the CFG.
 */
void CFGTransformer::install(CFG *cfg, CFGMaker& maker) {
	cur = &maker;
	bmap.clear();
	bmap.put(cfg->entry(), maker.entry());
	bmap.put(cfg->exit(), maker.exit());
	if(!no_unknown && cfg->unknown())
		bmap.put(cfg->unknown(), maker.unknown());
}

/**
 * Add a CFG to process. A new CFG is always added and no duplication
 * test is performed but no map between old and new CFG is recorded.
 * @param cfg	CFG to add.
 * @return		Matching CFG maker.
 */
CFGMaker& CFGTransformer::add(CFG *cfg) {
	CFGMaker *maker = new CFGMaker(cfg->first());
	add(cfg, *maker);
	return *maker;
}

/**
 * Add a CFG and its maker to process. A new CFG is always added and no duplication
 * test is performed but no map between old and new CFG is recorded.
 * @param cfg		CFG to add.
 * @param maker		Matching CFG maker.
 */
void CFGTransformer::add(CFG *cfg, CFGMaker& maker) {
	makers.add(&maker);
	wl.put(pair(cfg, &maker));
}

/**
 * @fn void CFGTransformer::setNoUnknown(bool v);
 * Configure no-unknown option: this option avoid to automatically
 * generate a new unknown block during CFG transormation.
 * @param v	True for activating, false else.
 */

/**
 * @fn bool CFGTransformer::getNoUnknown(void) const;
 * Get the configuration of no-unknown option.
 * @return	No-unknown configuration.
 */

/**
 * @fn CFG *CFGTransformer::entry(void) const;
 * Get the entry CFG.
 */

}	// otawa
