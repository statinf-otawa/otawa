/*
 *	$Id$
 *	LoopInfoBuilder class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2006-08, IRIT UPS.
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

//#define OTAWA_IDFA_DEBUG
#include <elm/data/Vector.h>
#include <elm/data/SortedList.h>
#include <otawa/cfg.h>
#include <otawa/cfg/features.h>
#include <otawa/dfa/BitSet.h>
#include <otawa/dfa/IterativeDFA.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/proc/BBProcessor.h>

using namespace elm;
using namespace otawa;
using namespace otawa::dfa;

namespace otawa {

/*
 * Cleaner used to clear the ENCLOSING_LOOP_HEADER identifier when the
 * LOOP_INFO_FEATURE is invalidated.
 */
class LoopInfoCleaner: public elm::Cleaner {
public:
	 LoopInfoCleaner(WorkSpace *_ws): ws(_ws) { }

protected:
	virtual void clean() {
		const CFGCollection* cfgc = INVOLVED_CFGS(ws);
		for(CFGCollection::Iter cfg(cfgc); cfg(); cfg++) {
			for(CFG::BlockIter bb = cfg->blocks(); bb(); bb++) {
				for (BasicBlock::EdgeIter outedge = bb->outs(); outedge(); outedge++) {
					if (LOOP_EXIT_EDGE(*outedge)) {
						if (EXIT_LIST(LOOP_EXIT_EDGE(*outedge))) {
							delete EXIT_LIST(LOOP_EXIT_EDGE(*outedge));
							EXIT_LIST(LOOP_EXIT_EDGE(*outedge)).remove();
						}
						LOOP_EXIT_EDGE(*outedge).remove();
					}
				} // for each outedge
				if(ENCLOSING_LOOP_HEADER(*bb).exists())
					ENCLOSING_LOOP_HEADER(*bb).remove();
			} // for each bb
		}
	}
private:
	WorkSpace* ws;
};

/**
 * This processor produces loop informations:
 * For each basic block, provides the loop which the basicblock belongs to.
 * For each edge exiting from a loop, provides the header of the exited loop.
 *
 * @par Configuration
 * none
 *
 * @par Required Features
 * @li @ref DOMINANCE_FEATURE
 * @li @ref LOOP_HEADERS_FEATURE
 *
 * @par Provided Features
 * @li @ref LOOP_INFO_FEATURE
 *
 * @par Statistics
 * none
 */
class LoopInfoBuilder: public CFGProcessor {
public:
	static p::declare reg;
	LoopInfoBuilder();
protected:
	void processCFG(otawa::WorkSpace*, otawa::CFG*) override;
private:
	void buildLoopExitList(otawa::CFG* cfg);
	bool fst;
};


/**
 * @class LoopInfoProblem
 *
 * This class defines an Iterative DFA problem for detecting which loop
 * contains a BB.
 * @par
 * Let a CFG, in OTAWA meaning, is <V, E, e>, tuple made of block vertices, edges
 * and entry point. The iterative DFA problem is defined by:
 * @li OUT(v) = { } initially
 * @li GEN(v) = { } if exist (w, v) in E /\ v dom w
 * @li GEN(v) = { w / (v, w) in E /\ w dom v } else
 * @li KILL(v) = { v / exists (w, v) in E /\ v dom w }
 *
 * And represents the set of
 */

/**
 * This feature asserts that the loop info of the task is available in
 * the framework.
 *
 * @par Properties
 * @li @ref ENCLOSING_LOOP_HEADER (@ref Block)
 * @li @ref LOOP_ENTRY (@ref Block)
 * @li @ref LOOP_EXIT (@ref Edge)
 * @li @ref EXIT_LIST (@ref Block)
 */
p::feature LOOP_INFO_FEATURE("otawa::LOOP_INFO_FEATURE", new Maker<LoopInfoBuilder>());

/**
 * Defined for any BasicBlock that is part of a loop.
 * Contains the header of the loop immediately containing the block
 * If the block is a loop header, then, the property contains the header of the parent loop.
 *
 * @par Hooks
 * @li @ref BasicBlock
 */
p::id<Block*> ENCLOSING_LOOP_HEADER("otawa::ENCLOSING_LOOP_HEADER", 0);

/**
 * Is defined for an Edge if this Edge is the entry-edge of any loop.
 * If this is the case, then, the property contains the header of the loop exited by the edge.
 *
 * @par Hooks
 * @li @ref Edge
 * @ingroup cfg
 */
p::id<Block*> LOOP_ENTRY("otawa::LOOP_ENTRY", nullptr);

/**
 * Is defined for an Edge if this Edge is the exit-edge of any loop.
 * If this is the case, then, the property contains the header of the loop exited by the edge.
 * If the edge exits more than one loop, the property contains the header of the outer loop.
 *
 * @par Hooks
 * @li @ref Edge
 * @ingroup cfg
 */
p::id<Block*> LOOP_EXIT("otawa::LOOP_EXIT", nullptr);


/**
 * @deprecated	Use LOOP_EXIT instead.
 * @ingroup cfg
 */
p::id<Block*>& LOOP_EXIT_EDGE = LOOP_EXIT;


/**
 * Defined for any BasicBlock that is a loop header.
 * Contain a list of the exit edges associated with the loop
 * (more clearly, EXIT_LIST(h) is the list of the edges for which LOOP_EXIT_EDGE(edge) == h)
 *
 * @par Hooks
 * @li @ref BasicBlock
 * @ingroup cfg
 */
p::id<elm::Vector<Edge *> *> EXIT_LIST("otawa::EXIT_LIST", 0);


// LoopInfoProblem class
class LoopInfoProblem {

	class DominanceOrder {
 	public:
		typedef Block *t;

		DominanceOrder(void): _info(0) { }
		DominanceOrder(DomInfo& info): _info(&info) { }
		void set(DomInfo& info) { _info = &info; }

 		inline int doCompare(Block *bb1, Block *bb2) const {
 			if(_info->dom(bb1, bb2))
 				return +1;
 			else if(_info->dom(bb2, bb1))
 				return -1;
 			else
 				return 0;
 		}
 	private:
 		DomInfo *_info;
 	};

public:

	LoopInfoProblem(CFG& cfg, DomInfo& info);
 	inline dfa::BitSet *empty(void) const;
 	dfa::BitSet *gen(Block *bb) const;
 	dfa::BitSet *kill(Block *bb) const;
 	bool equals(dfa::BitSet *set1, dfa::BitSet *set2) const;
 	void reset(dfa::BitSet *set) const;
 	void merge(dfa::BitSet *dst, dfa::BitSet *src) const;
 	void set(dfa::BitSet *dst, dfa::BitSet *src) const;
 	void add(dfa::BitSet *dst, dfa::BitSet *src) const;
 	void diff(dfa::BitSet *dst, dfa::BitSet *src);
 	inline int count(void) const;
 	inline Block *get(int index) const;
 	inline void free(dfa::BitSet *set) { delete set; }
#ifndef NDEBUG
 	void dump(elm::io::Output& out, dfa::BitSet *set);
#endif

private:
	DomInfo& d;
	/*DominanceOrder dorder;
	CompareManager<Block *, DominanceOrder> dman;*/
	SortedList<Block *, DominanceOrder > headersLList;
	Vector<Block *> hdrs;

};

/* Constructors/Methods for LoopInfoProblem */
LoopInfoProblem::LoopInfoProblem(CFG& cfg, DomInfo& info): d(info) {
	headersLList.comparator().set(info);

	/*
	 * Find all the headers of the CFG
	 * Adds them in a SORTED list
	 */
	for (CFG::BlockIter bb = cfg.blocks(); bb(); bb++)
		if(!bb->isEntry() && LOOP_HEADER(*bb))
			headersLList.add(*bb);

	/* Converting to Vector, because a linked list is not very practical ... */
	for(const auto h: headersLList)
		hdrs.add(h);
}

inline dfa::BitSet* LoopInfoProblem::empty(void) const {
		return new dfa::BitSet(hdrs.length());
}

dfa::BitSet* LoopInfoProblem::gen(Block *bb) const {
		dfa::BitSet *result = empty();
		for(Block::EdgeIter edge = bb->outs(); edge(); edge++)
			if(bb != edge->target()			// not for single BB loop
			&& d.dom(edge->target(), bb))
				result->add(hdrs.indexOf(edge->target()));
		return result;
}

dfa::BitSet* LoopInfoProblem::kill(Block *bb) const {
		dfa::BitSet *result = empty();
		if(LOOP_HEADER(bb))
			result->add(hdrs.indexOf(bb));
		return result;
}

bool LoopInfoProblem::equals(dfa::BitSet *set1, dfa::BitSet *set2) const {
		return set1->equals(*set2);
}

void LoopInfoProblem::reset(dfa::BitSet *set) const {
		set->empty();
}

void LoopInfoProblem::merge(dfa::BitSet *dst, dfa::BitSet *src) const {
		*dst += *src;
}

void LoopInfoProblem::set(dfa::BitSet *dst, dfa::BitSet *src) const {
		*dst = *src;
}

void LoopInfoProblem::add(dfa::BitSet *dst, dfa::BitSet *src) const {
		*dst += *src;
}

void LoopInfoProblem::diff(dfa::BitSet *dst, dfa::BitSet *src) {
		*dst -= *src;
}

inline int LoopInfoProblem::count(void) const {
		return hdrs.length();
}

inline Block* LoopInfoProblem::get(int index) const {
		return hdrs[index];
}

#ifndef NDEBUG
/* Dump the content of a bit set.
*/
	void LoopInfoProblem::dump(elm::io::Output& out, dfa::BitSet *set) {
			bool first = true;
			out << "{ ";
			for(int i = 0; i < hdrs.length(); i++)
				if(set->contains(i)) {
					if(first)
						first = false;
					else
						out << ", ";
					out << hdrs[i]->index();
				}
			out << " }";
	}
#endif


p::declare LoopInfoBuilder::reg =
	p::init("otawa::LoopInfoBuilder", Version(2, 0, 0))
	.extend<CFGProcessor>()
	.require(DOMINANCE_FEATURE)
	.require(LOOP_HEADERS_FEATURE)
	.provide(LOOP_INFO_FEATURE)
	.make<LoopInfoBuilder>();


/* Constructors/Methods for LoopInfoBuilder */

LoopInfoBuilder::LoopInfoBuilder(): CFGProcessor(reg), fst(true) {
}


/*
 * Annotate each loop-header with the list of edges exiting the loop.
 */
 void LoopInfoBuilder::buildLoopExitList(otawa::CFG* cfg) {
 	for(CFG::BlockIter bb = cfg->blocks(); bb(); bb++) {
 		for (BasicBlock::EdgeIter outedge = bb->outs(); outedge(); outedge++) {
 			if (LOOP_EXIT_EDGE(*outedge)) {
 				if (!EXIT_LIST(LOOP_EXIT_EDGE(*outedge))) {
 					EXIT_LIST(LOOP_EXIT_EDGE(*outedge)) = new elm::Vector<Edge*>();
 				}
				EXIT_LIST(LOOP_EXIT_EDGE(*outedge))->add(*outedge);
 			}
 		}
 	}
 }

void LoopInfoBuilder::processCFG(otawa::WorkSpace* fw, otawa::CFG* cfg) {
	int i;

	// computes set of headers
	LoopInfoProblem prob(*cfg, *DOMINANCE_FEATURE.get(fw));
	if (prob.count() == 0)
		return;
	IterativeDFA<LoopInfoProblem, dfa::BitSet, CFG, Successor> dfa(prob, cfg, cfg->exit());
	dfa.compute();

	// compute enclosing loop header for each BB
	for(CFG::BlockIter bb = cfg->blocks(); bb(); bb++) {

		// enclosing loop header is the first element of set
		// (headers are sorted according by increasing domination)
		dfa::BitSet::Iterator bit(*dfa.outSet(*bb));
		if (bit()) {
			ENCLOSING_LOOP_HEADER(*bb) = prob.get(*bit);
			if (logFor(LOG_BLOCK))
				cerr << "\t\t\tloop of " << *bb << " is " << ENCLOSING_LOOP_HEADER(*bb) << io::endl;
		}
	}

	// add blocks themselves to the outing sets
	for (i = 0; i < prob.count(); i++)
		dfa.outSet(prob.get(i))->add(i);

	// compute loop exit edges
	for (CFG::BlockIter bb = cfg->blocks(); bb(); bb++) {

		// compute loop entry
		if(LOOP_HEADER(*bb)) {
            EXIT_LIST(*bb) = new elm::Vector<Edge*>();
            for (auto e: bb->inEdges())
                if (!BACK_EDGE(e))
                    LOOP_ENTRY(e) = *bb;
        }

		// process block in loops
		if (ENCLOSING_LOOP_HEADER(*bb) || LOOP_HEADER(*bb))
			for(Block::EdgeIter outedge = bb->outs(); outedge(); outedge++) {
				// result = out set of edge source - out set of edge target
				// result = { headers dominating source but not target }
				dfa::BitSet *targetSet = dfa.outSet(outedge->target());
				dfa::BitSet result(*dfa.outSet(*bb));
				result.remove(*targetSet);

				// last bit is the left outermost header
				int h = -1;
				for(dfa::BitSet::Iterator bit(result); bit(); bit++)
					h = *bit;
				if(h >= 0)
					LOOP_EXIT_EDGE(*outedge) = prob.get(h);
			}
	}

	// build loop exit lists
	buildLoopExitList(cfg);

	if(fst) { // only add the cleaner for the first time
		addCleaner(LOOP_INFO_FEATURE, new LoopInfoCleaner(fw));
		fst = false;
	}
}

}	// otawa

