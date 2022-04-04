/*
 *	$Id$
 *	Dominance class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-07, IRIT UPS.
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
#include <elm/deprecated.h>
#include <otawa/cfg.h>
#include <otawa/cfg/Dominance.h>
#include <otawa/cfg/features.h>
#include <otawa/dfa/BitSet.h>
//#define OTAWA_IDFA_DEBUG
#include <otawa/dfa/IterativeDFA.h>
#include <otawa/proc/BBProcessor.h>
#include <otawa/prog/WorkSpace.h>

using namespace otawa::dfa;

namespace otawa {

class DominanceCleaner: public BBCleaner {
public:
	DominanceCleaner(WorkSpace *ws): BBCleaner(ws) { }

protected:
	virtual void clean(WorkSpace *ws, CFG *cfg, Block *bb) {
		if(REVERSE_DOM(bb).exists()) {
			delete REVERSE_DOM(bb);
			bb->removeProp(REVERSE_DOM);
		}
		if(LOOP_HEADER(bb)) {
			bb->removeProp(LOOP_HEADER);
			for(BasicBlock::EdgeIter edge(bb->ins()); edge(); edge++)
				bb->removeProp(BACK_EDGE);
		}
	}
};

/**
 * Identifier of annotation containing reverse-dominance information.
 *
 * @par Hooks
 * @li @ref BasicBlock
 */
Identifier<const BitSet *> REVERSE_DOM("otawa::REVERSE_DOM", 0);


/**
 * Identifier for marking basic blocks that are entries of loops.
 *
 * @par Hooks
 * @li @ref BasicBlock
 * @ingroup cfg
 */
Identifier<bool> LOOP_HEADER("otawa::LOOP_HEADER", false);

/**
 * Identifier for marking back edges.
 *
 * @par Hooks
 * @li @ref BasicBlock
 * @ingroup cfg
 */
Identifier<bool> BACK_EDGE("otawa::BACK_EDGE", false);


/**
 * @class DomInfo
 * Interface providing information on the computed dominance relation.
 * @see DOM_INFO, DOMINANCE_FEATURE
 * @ingroup cfg
 */

/**
 */
DomInfo::~DomInfo(void) {
}

/**
 * @fn bool DomInfo::dom(Block *b1, Block *b2);
 * Test if the block b1 dominates block b2.
 * @param b1	Dominating block.
 * @param b2	Dominated block.
 * @return		True if b1 dominates b2, false else.
 */

/**
 * @fn bool DomInfo::isBackEdge(Edge *e);
 * Test if the given edge is a back edge.
 * @param e		Edge to test.
 * @return		True if it is a back edge, false else.
 */


/**
 * This is the Problem used to instanciate DFAEngine for computing the
 * reverse domination relation. For each basic block, the set of dominators
 * is computed and hooked to the basic block. Then, a simple bit test is
 * used for testing the relation.
 */

class DominanceProblem {
	CFG *cfg;
	int size;
public:
	DominanceProblem(CFG *_cfg) {
		cfg = _cfg;
		size = _cfg->count();
	}

	BitSet *empty(void) {
		BitSet *result = new BitSet(size);
		result->fill();
		return result;
	}

	BitSet *gen(Block *bb) {
		BitSet *result = new BitSet(size);
		result->add(bb->index());
		return result;
	}

	BitSet *kill(Block *bb) {
		BitSet *result = new BitSet(size);
		if(bb->isEntry())
			result->fill();
		return(result);
	}
	bool equals(BitSet *set1, BitSet *set2) {
		return(set1->equals(*set2));
	}
	void reset(BitSet *set) {
		set->fill();
	}
	void merge(BitSet *set1, BitSet *set2) {
		set1->mask(*set2);
	}
	void set(BitSet *dset, BitSet *tset) {
		*dset = *tset;
	}
	void add(BitSet *dset, BitSet *tset) {
		dset->add(*tset);
	}
	void diff(BitSet *dset, BitSet *tset) {
		dset->remove(*tset);
	}
	void free(BitSet *set) { delete set; }

	void dump(Output& out, BitSet *set) const {
		bool fst = true;
		out << "{ ";
		for(auto v: *cfg)
			if(set->contains(v->index())) {
				if(fst)
					fst = false;
				else
					out << ", ";
				out << v;
			}
		out << " }";
	}
};


/**
 * @class Dominance
 * This CFG processor computes and hook to the CFG the dominance relation
 * that, then, may be tested with @ref Dominance::dominate() function.
 *
 * @p Provided Features
 * @li @ref DOMINANCE_FEATURE
 * @li @ref LOOP_HEADERS_FEATURE
 *
 * @p Required Features
 * @li @ref CHECKED_CFG_FEATURE
 */

/**
 */
p::declare Dominance::reg = p::init("otawa::Dominance", Version(1, 2, 0))
	.provide(DOMINANCE_FEATURE)
	.provide(LOOP_HEADERS_FEATURE)
	.base(ConcurrentCFGProcessor::reg)
	.maker<Dominance>();

/**
 * The dominance processors computes dominance relation and the loop headers
 * on the current CFG.
 */
Dominance::Dominance(void): ConcurrentCFGProcessor(reg) {
}


/**
 * Test if the first basic block dominates the second one.
 * @param bb1	Dominator BB.
 * @param bb2	Dominated BB.
 * @return		True if bb1 dominates bb2.
 */
bool Dominance::dominates(Block *bb1, Block *bb2) {
	ASSERTP(bb1, "null BB 1");
	ASSERTP(bb2, "null BB 2");
	//ASSERTP(bb1->cfg() == bb2->cfg(), "both BB are not owned by the same CFG");
	int index = bb1->index();
	ASSERTP(index >= 0, "no index for BB 1");
	const BitSet *set = REVERSE_DOM(bb2);
	ASSERTP(set, "no index for BB 2");
	ASSERTP(bb1 == bb2
	||	!REVERSE_DOM(bb1)->contains(bb2->index())
	||  !REVERSE_DOM(bb2)->contains(bb1->index()),
		"CFG with disconnected nodes for CFG " << bb1->cfg()->index() << ": " << bb1->cfg()->name() << "(" << bb1 << ", " << bb2 << ")");
	return set->contains(index);
}


/**
 * @fn bool Dominance::isDominated(BasicBlock *bb1, BasicBlock *bb2);
 * Test if the first block is dominated by the second one.
 * @param bb1	Dominated BB.
 * @param bb2	Dominator BB.
 * @return		True if bb2 dominates bb1.
 */


/**
 * Computes the domination relation.
 */
void Dominance::processCFG(WorkSpace *ws, CFG *cfg) {
	ASSERT(cfg);
	DominanceProblem dp(cfg);
	dfa::IterativeDFA<DominanceProblem, BitSet, CFG> engine(dp, cfg, cfg->entry());
	engine.compute();
	for (CFG::VertexIter bb(cfg->vertices()); bb(); bb++) {
	  BitSet *b = engine.outSet(*bb);
	  b = new BitSet(*b);
	  REVERSE_DOM(*bb) = b;
	}
	markLoopHeaders(cfg);
	addCleaner(DOMINANCE_FEATURE, new DominanceCleaner(ws));
}


/**
 */
void Dominance::cleanup(WorkSpace *ws) {
}


///
void Dominance::dumpCFG(CFG *g, Output& out) {
	out << "CFG " << g << io::endl;
	for(auto v: *g) {
		out << "\t" << v << " dominated by ";
		bool fst = true;
		for(auto d: *g) {
			if(dominates(d, v)) {
				if(!fst)
					out << ", ";
				fst = false;
				out << d;
			}
		}
		out << io::endl;
	}
}


/**
 */
bool Dominance::dom(Block *b1, Block *b2) {
	ASSERT(b1);
	ASSERT(b2);
	const BitSet *set = REVERSE_DOM(b2);
	ASSERT(set);
	return set->contains(b1->index());
}


/**
 */
Block *Dominance::idom(Block* b) {
	ASSERTP(false, "idom not yet implemented"); // TODO
	return NULL;
}


/**
 */
bool Dominance::isBackEdge(Edge *edge) {
	ASSERT(edge);
	return BACK_EDGE(edge);
}


/**
 */
void *Dominance::interfaceFor(const AbstractFeature& feature) {
	if(&feature == &DOMINANCE_FEATURE)
		return DOMINANCE_FEATURE.give(this);
	else
		return nullptr;
}



/**
 * Using a CFG where dominance relation has been computed, mark loop headers.
 * @param cfg		CFG to process.
 * @param headers	Collection filled with found headers.
 */
void Dominance::markLoopHeaders(CFG *cfg) {
	ASSERT(cfg);
	for(CFG::VertexIter bb(cfg->vertices()); bb(); bb++) {
		for(BasicBlock::EdgeIter edge(bb->outs()); edge(); edge++)
			if(dominates(edge->target(), *bb)) {

				// verbose output
				if(logFor(LOG_BLOCK)) {
					if(!LOOP_HEADER(edge->sink()))
						log << "\t\t\t" << edge->sink() << " is loop header\n";
					log << "\t\t\t" << *edge << " is back edge\n";
				}

				// record information
				LOOP_HEADER(edge->target()) = true;
				BACK_EDGE(*edge) = true;
			}
	}
}


/**
 * Test if the given basic block is a loop header.
 * @return True if it is a loop header, false else.
 */
bool Dominance::isLoopHeader(Block *bb) {
	ELM_DEPRECATED
	return(LOOP_HEADER(bb));
}


/**
 * Check if the dominance informance is available. Else compute it.
 * @param cfg	CFG to look at.
 */
void Dominance::ensure(CFG *cfg) {
	if(!REVERSE_DOM(cfg->entry())) {
		Dominance dom;
		dom.processCFG(0, cfg);
	}
}


/**
 * This feature ensures that information about domination between nodes
 * of a CFG is available. Back edges are marked with @ref BACK_EDGE property
 * while dominance may be tested using an interface of type @ref DomInfo
 * and provided by @ref DOM_INFO property.
 *
 * OTAWA provides several implementation of domination calculation:
 * @li @ref otawa::Dominance
 *
 * @par Properties
 * @li @ref BACK_EDGE
 * @li @ref DOM_INFO
 *
 * @ingroup cfg
 */
p::interfaced_feature<DomInfo> DOMINANCE_FEATURE("otawa::DOMINANCE_FEATURE", new Maker<Dominance>());


/**
 * This feature ensures that all loop header are marked with a @ref LOOP_HEADER
 * property, and the backedges are marked with a @ref BACK_EDGE property.
 *
 * @par Include
 * <otawa/cfg/features.h>
 *
 * @Properties
 * @li @ref LOOP_HEADER (BasicBlock).
 * @li @ref BACK_EDGE (Edge).
 */
p::feature LOOP_HEADERS_FEATURE("otawa::LOOP_HEADERS_FEATURE", new Maker<Dominance>());

} // otawa
