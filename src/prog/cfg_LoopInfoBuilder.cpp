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
#include <otawa/cfg/features.h>
#include <otawa/util/Dominance.h>
#include <elm/genstruct/Vector.h>
#include <elm/genstruct/SortedSLList.h>
#include <otawa/cfg.h>
#include <otawa/dfa/IterativeDFA.h>
#include <otawa/dfa/BitSet.h>

using namespace elm;
using namespace otawa;
using namespace otawa::dfa;

namespace otawa {


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
        LoopInfoBuilder(void);
        virtual void processCFG(otawa::WorkSpace*, otawa::CFG*);

private:
		/**
		 * Builds the EXIT_LIST property for all the loop headers.
		 */
		void buildLoopExitList(otawa::CFG* cfg);
};


/**
 * @class LoopInfoProblem
 *
 * This class defines a DFA problem for detecting which loop
 * contains a BB.
 * @par
 * We consider the CFG as pair <N, E>, N the set
 * of basic blocks and E the set of edges. We use a DFA that works on a
 * rever sed CFG using the following set :
 * <dl>
 * 	<dt>LOOP</dt><dd>set of loop headers</dd>
 * </dl>
 * @par
 * The gen set is built is a follow :
 * gen(n) =<br>
 * 		if n in LOOP, { }<br>
 * 		else { m / (n, m) in E and m dom n }
 * @par
 * And the kill set is as follow :<br>
 *		kill(n) = { n / n in LOOP}
 */

/**
 * This feature asserts that the loop info of the task is available in
 * the framework.
 *
 * @par Properties
 * @li @ref ENCLOSING_LOOP_HEADER (@ref BasicBlock)
 * @li @ref LOOP_EXIT_EDGE (@ref Edge)
 * @li @ref EXIT_LIST (@ref BasicBlock)
 */
p::feature LOOP_INFO_FEATURE("otawa::LOOP_INFO_FEATURE", new Maker<LoopInfoBuilder>());

/**
 * Defined for any BasicBlock that is part of a loop.
 * Contains the header of the loop immediately containing the basicblock
 * If the basicblock is a loop header, then, the property contains the header of the parent loop.
 *
 * @par Hooks
 * @li @ref BasicBlock
 */
Identifier<BasicBlock*> ENCLOSING_LOOP_HEADER("otawa::ENCLOSING_LOOP_HEADER", 0);

/**
 * Is defined for an Edge if this Edge is the exit-edge of any loop.
 * If this is the case, then, the property contains the header of the loop exited by the edge.
 * If the edge exits more than one loop, the property contains the header of the outer loop.
 *
 * @par Hooks
 * @li @ref Edge
 */
Identifier<BasicBlock*> LOOP_EXIT_EDGE("otawa::LOOP_EXIT_EDGE", 0);
/**
 * Defined for any BasicBlock that is a loop header.
 * Contain a list of the exit edges associated with the loop
 * (more clearly, EXIT_LIST(h) is the list of the edges for which LOOP_EXIT_EDGE(edge) == h)
 *
 * @par Hooks
 * @li @ref BasicBlock
 */
Identifier<elm::genstruct::Vector<Edge*>*> EXIT_LIST("otawa::EXIT_LIST", 0);


// LoopInfoProblem class
class LoopInfoProblem {
	class DominanceOrder {
 		public:
 		static inline int compare(BasicBlock *bb1, BasicBlock *bb2) {
 			if(Dominance::dominates(bb1, bb2))
 				return +1;
 			else if(Dominance::dominates(bb2, bb1))
 				return -1;
 			else
 				return 0;
 		}
 	};
	CFG& _cfg;
	genstruct::SortedSLList<BasicBlock *, DominanceOrder> headersLList;
	genstruct::Vector<BasicBlock *> hdrs;

 	public:
 	LoopInfoProblem(CFG& cfg);
 	inline dfa::BitSet *empty(void) const;
 	dfa::BitSet *gen(BasicBlock *bb) const;
 	dfa::BitSet *kill(BasicBlock *bb) const;
 	bool equals(dfa::BitSet *set1, dfa::BitSet *set2) const;
 	void reset(dfa::BitSet *set) const;
 	void merge(dfa::BitSet *dst, dfa::BitSet *src) const;
 	void set(dfa::BitSet *dst, dfa::BitSet *src) const;
 	void add(dfa::BitSet *dst, dfa::BitSet *src) const;
 	void diff(dfa::BitSet *dst, dfa::BitSet *src);
 	inline int count(void) const;
 	inline BasicBlock *get(int index) const;
 	inline void free(dfa::BitSet *set) { delete set; }
#ifndef NDEBUG
 	void dump(elm::io::Output& out, dfa::BitSet *set);
#endif

};

/* Constructors/Methods for LoopInfoProblem */
LoopInfoProblem::LoopInfoProblem(CFG& cfg): _cfg(cfg), headersLList() {

		/*
		 * Find all the headers of the CFG
		 * Adds them in a SORTED list
		 */
		for (CFG::BBIterator bb(&cfg); bb; bb++) {
			if (!bb->isEntry() && LOOP_HEADER(bb)) {
				headersLList.add(bb);
			}
		}
		 /* Converting to Vector, because a linked list is not very practical ... */
		for (genstruct::SortedSLList<BasicBlock*, DominanceOrder>::Iterator iter(headersLList); iter; iter++) {
			hdrs.add(*iter);
		}
	}

inline dfa::BitSet* LoopInfoProblem::empty(void) const {
		return new dfa::BitSet(hdrs.length());
}

dfa::BitSet* LoopInfoProblem::gen(BasicBlock *bb) const {
		dfa::BitSet *result = empty();
		for(BasicBlock::OutIterator edge(bb); edge; edge++) {
			if(edge->kind() != Edge::CALL
			&& bb != edge->target()		// required for single BB loop
			&& Dominance::dominates(edge->target(), bb))
				result->add(hdrs.indexOf(edge->target()));
		}
		return result;
}

dfa::BitSet* LoopInfoProblem::kill(BasicBlock *bb) const {
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

inline BasicBlock* LoopInfoProblem::get(int index) const {
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
					out << hdrs[i]->number();
				}
			out << " }";
	}
#endif




/* Constructors/Methods for LoopInfoBuilder */

LoopInfoBuilder::LoopInfoBuilder(void): CFGProcessor("otawa::LoopInfoBuilder", Version(1, 0, 0)) {
	require(DOMINANCE_FEATURE);
	require(LOOP_HEADERS_FEATURE);
	provide(LOOP_INFO_FEATURE);
}


/*
 * Annotate each loop-header with the list of edges exiting the loop.
 */
 void LoopInfoBuilder::buildLoopExitList(otawa::CFG* cfg) {
 	for (CFG::BBIterator bb(cfg); bb; bb++) {
 		for (BasicBlock::OutIterator outedge(*bb); outedge; outedge++) {
 			if (LOOP_EXIT_EDGE(*outedge)) {
 				if (!EXIT_LIST(LOOP_EXIT_EDGE(*outedge))) {
 					EXIT_LIST(LOOP_EXIT_EDGE(*outedge)) = new elm::genstruct::Vector<Edge*>();
 				}
				EXIT_LIST(LOOP_EXIT_EDGE(*outedge))->add(outedge);
 			}
 		}
 	}
 }


void LoopInfoBuilder::processCFG(otawa::WorkSpace* fw, otawa::CFG* cfg) {
        int i;
    	LoopInfoProblem prob(*cfg);

    	if (prob.count() == 0)
    		return;
        IterativeDFA<LoopInfoProblem, dfa::BitSet, Successor> dfa(prob, *cfg);
        dfa.compute();


        /* Iterate to find the enclosing loop headers */
        for (CFG::BBIterator bb(cfg); bb; bb++) {

			/*
			 * Detects the enclosing loop header of this bb by selecting the last element (that is, the lowest in the
			 * order defined by the Dominance relation)
			 */
        	dfa::BitSet::Iterator bit(*dfa.outSet(bb));

        	if (bit) {
        		ENCLOSING_LOOP_HEADER(bb) = prob.get(*bit);
        		if(logFor(LOG_BLOCK))
        			cerr << "\t\t\tloop of " << *bb << " is " << ENCLOSING_LOOP_HEADER(bb) << io::endl;
        	}

        }

        /*
         * For the loop-exit-edge analysis, we need to have each loop header bitset
         * containing itself.
         */
        for (i = 0; i < prob.count(); i++)
                dfa.outSet(prob.get(i))->add(i);

        /* Iterate to find loop exit edges */
        for (CFG::BBIterator bb(cfg); bb; bb++) {
                if (ENCLOSING_LOOP_HEADER(bb) || LOOP_HEADER(bb)) {
                        /* If this basicBlock is in a loop, then we try to detect
                         * the loop-exit edges starting from it.
                         * We use LOOP_HEADER() to not forget the loop-header
                         * of the most outer loop.
                         */
        		for (BasicBlock::OutIterator outedge(*bb); outedge; outedge++) {
        		        if (outedge->kind() != Edge::CALL) {
        		                /* The outer exited loop header is the greatest element
                                         * (in the Domination order) of sourceSet - targetSet
                                         */

                                        dfa::BitSet *targetSet = dfa.outSet(outedge->target());
                                        dfa::BitSet result(*dfa.outSet(bb));

                                        result.remove(*targetSet);
                                        dfa::BitSet::Iterator bit(result);
                                        if (bit)
                                                LOOP_EXIT_EDGE(outedge) = prob.get(*bit);
                                }
                        }
                }

        }

        buildLoopExitList(cfg);


}



}
