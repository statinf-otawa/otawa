/*
 *	$Id$
 *	FixPoint which unrolls the first iteration of each loop.
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007, IRIT UPS.
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */

#ifndef OTAWA_DFA_HAI_FIRSTUNROLLINGFIXPOINT_H_
#define OTAWA_DFA_HAI_FIRSTUNROLLINGFIXPOINT_H_

#include "HalfAbsInt.h"
#include <otawa/cfg/CFG.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/cfg/Edge.h>
#include <otawa/prop/PropList.h>
#include <elm/data/Vector.h>


namespace otawa { namespace dfa { namespace hai {

template <class Listener>
class FirstUnrollingFixPoint {
public:
	typedef typename Listener::Problem Problem;
	typedef typename Problem::Domain Domain;
		

protected:
	static Identifier<Domain*> STATE;	
	Problem& prob;
	Listener  &list;
	HalfAbsInt<FirstUnrollingFixPoint> *ai;
	
public:

	class FixPointState {
		public:
		Domain headerState;
		Domain firstIterState;
		int numIter;
		inline FixPointState(const Domain &bottom): headerState(bottom), firstIterState(bottom) { numIter = 0; }
	};
	
	inline FixPointState *newState(void) { return(new FixPointState(bottom())); }
	inline FirstUnrollingFixPoint(Listener & _list):prob(_list.getProb()), list(_list) ,ai(0) { }
	inline ~FirstUnrollingFixPoint(void) { }

	inline int getIter(Block *bb) const { return(ai->getFixPointState(bb)->numIter); }
	inline void init(HalfAbsInt<FirstUnrollingFixPoint> *_ai) { ai = _ai; }
	void fixPoint(Block *bb, bool &fixpoint, Domain &in, bool firstTime) const;
	
	// edge marking functions
	inline void markEdge(PropList *e, const Domain &s);
	inline void unmarkEdge(PropList *e);
	inline Domain *getMark(PropList *e);
	inline void updateEdge(Edge *edge, Domain &dom);
	
	// problem wrapper functions
	inline const Domain& top(void) const { return prob.top(); }
	inline const Domain& bottom(void) const;
	inline const Domain& entry(void) const;
	inline void lub(Domain &a, const Domain &b) const;
	inline void assign(Domain &a, const Domain &b) const;
	inline bool equals(const Domain &a, const Domain &b) const;
	inline void update(Domain &out, const Domain &in, Block* bb);
	inline void blockInterpreted(Block* bb, const Domain& in, const Domain& out, CFG *cur_cfg, Vector<Edge*> *callStack) const;
	inline void fixPointReached(Block* bb) const;
	inline void enterContext(Domain &dom, Block* bb, hai_context_t ctx) const;
	inline void leaveContext(Domain &dom, Block* bb, hai_context_t ctx) const;
};

template <class Listener>
Identifier<typename Listener::Problem::Domain*> FirstUnrollingFixPoint<Listener>::STATE("", 0);
	
template < class Listener >	
void FirstUnrollingFixPoint<Listener >::fixPoint(Block *bb, bool &fixpoint, Domain &in, bool firstTime) const {
	FixPointState *fpstate = ai->getFixPointState(bb);
	Domain newHeaderState(bottom());
	fixpoint = false;

	switch(fpstate->numIter) {

	// We never did any iteration, we begin the first:
	// Use for IN the union of entry edges. Fixpoint is always false.
	case 0:
		assign(newHeaderState, ai->entryEdgeUnion(bb));
		break;

	// We finished the first iteration, we begin the 2nd:
	// Use for IN the union of back edges. Fixpoint may be reached at this point
	// but even if it is, we still want to do another iteration anyway.
	case 1:
		assign(newHeaderState, ai->backEdgeUnion(bb));
		assign(fpstate->firstIterState, newHeaderState);
		break;

	// We finished the 2..n^th iteration:
	// Use for IN the union of back edges + the union of entry edges
	// of the first iteration.
	// Test for fixpoint.
	default:
		assign(newHeaderState, ai->backEdgeUnion(bb));
		prob.lub(newHeaderState, fpstate->firstIterState);
		if (prob.equals(newHeaderState, fpstate->headerState))
			fixpoint = true;
		break;
	}
	fpstate->numIter ++;

	// Store the new loop header state in the FixPointState
	assign(fpstate->headerState, newHeaderState);

	// Pass the new loop header state to the caller (HalfAbsInt)
	assign(in, newHeaderState);
}
	
template < class Listener >	
inline void FirstUnrollingFixPoint<Listener>::markEdge(PropList *e, const Domain &s) {
	
		/*
		 * Because this FixPoint unrolls the first iteration of each loop, 
		 * the loop-exit-edges will be marked at least 2 times 
		 * (one time for 1st iteration, and one time for others iterations),
		 * so when we mark the edges for the 2nd time we need to merge (lub)
		 * with the existing value from the 1st iteration, instead of overwriting it.
		 */
		if (STATE(e) == 0)
			STATE(e) = new Domain(bottom());
			
		prob.lub(**STATE(e), s);
	}
	
template < class Listener >	
inline void FirstUnrollingFixPoint<Listener >::unmarkEdge(PropList *e) {
		delete STATE(e);
		STATE(e) = 0;
}

template < class Listener >		
inline typename FirstUnrollingFixPoint<Listener>::Domain *FirstUnrollingFixPoint<Listener >::getMark(PropList *e) {
		return(STATE(e));
}
	
	
	/*
	 * Wrappers for the Problem methods and types
	 */
	 
template < class Listener >	
inline const typename FirstUnrollingFixPoint<Listener>::Domain& FirstUnrollingFixPoint<Listener >::bottom(void) const {
		return(prob.bottom());
}

template < class Listener >		
inline const typename FirstUnrollingFixPoint<Listener>::Domain& FirstUnrollingFixPoint<Listener >::entry(void) const {
		return(prob.entry());
}

template < class Listener >	
inline void FirstUnrollingFixPoint<Listener >::lub(typename Problem::Domain &a, const typename Problem::Domain &b) const {
		prob.lub(a,b);
}

template < class Listener >		
inline void FirstUnrollingFixPoint<Listener >::assign(typename Problem::Domain &a, const typename Problem::Domain &b) const {
		prob.assign(a,b);
}

template < class Listener >		
inline bool FirstUnrollingFixPoint<Listener >::equals(const typename Problem::Domain &a, const typename Problem::Domain &b) const {
		return (prob.equals(a,b));
}

template < class Listener >	
inline void FirstUnrollingFixPoint<Listener>::update(Domain &out, const typename Problem::Domain &in, Block* bb)  {
		prob.update(out,in,bb);
}
	
template < class Listener >	
inline void FirstUnrollingFixPoint<Listener>::blockInterpreted(Block* bb, const typename Problem::Domain& in, const typename Problem::Domain& out, CFG *cur_cfg, Vector<Edge*> *callStack) const {
		list.blockInterpreted(this, bb, in, out, cur_cfg, callStack);
}

template < class Listener >	
inline void FirstUnrollingFixPoint<Listener >::fixPointReached(Block* bb) const {
		list.fixPointReached(this, bb);
}
	
template < class Listener >	
inline void FirstUnrollingFixPoint<Listener >::enterContext(Domain &dom, Block* bb, hai_context_t ctx) const {
		prob.enterContext(dom, bb, ctx);
}
	
template < class Listener >	
inline void FirstUnrollingFixPoint<Listener>::leaveContext(Domain &dom, Block* bb, hai_context_t ctx) const {
		prob.leaveContext(dom, bb, ctx);
}

template < class Listener > 
inline void FirstUnrollingFixPoint<Listener>::updateEdge(Edge *edge, Domain &dom) {
}
	
	
} } }	// otawa::dfa::hai

#endif /*OTAWA_DFA_HAI_FIRSTUNROLLINGFIXPOINT_H_*/
