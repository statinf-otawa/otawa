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

#ifndef UTIL_FIRSTUNROLLINGFIXPOINT_H_
#define UTIL_FIRSTUNROLLINGFIXPOINT_H_

#include <otawa/util/HalfAbsInt.h>
#include <otawa/cfg/CFG.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/cfg/Edge.h>
#include <otawa/prop/PropList.h>
#include <elm/genstruct/Vector.h>


namespace otawa {

template <class Listener>
class FirstUnrollingFixPoint {
	
	// Types
	public:
	typedef typename Listener::Problem Problem;
	typedef typename Problem::Domain Domain;
		
	private:	
	

	 
	// Fields
	Identifier<Domain*> STATE;	
	Problem& prob;
	Listener  &list;
	util::HalfAbsInt<FirstUnrollingFixPoint> *ai;
	
	public:
	// FixPointState class
	class FixPointState {
		public:
		Domain headerState;
		Domain firstIterState;
		int numIter;
		inline FixPointState(const Domain &bottom): headerState(bottom), firstIterState(bottom) {
			numIter = 0;
		}
	};
	
	inline FixPointState *newState(void) {
		return(new FixPointState(bottom()));
	}

	
	inline FirstUnrollingFixPoint(Listener & _list)
	: STATE("", NULL), prob(_list.getProb()), list(_list), ai(NULL)
	{
	}	
	// Destructor
	inline ~FirstUnrollingFixPoint() {
	}
	
	// Accessors
	inline int getIter(BasicBlock *bb) const;
	
	// Mutators 
	inline void init(util::HalfAbsInt<FirstUnrollingFixPoint> *_ai);
	
	// FixPoint function
	void fixPoint(BasicBlock *bb, bool &fixpoint, Domain &in, bool firstTime) const;
	
	// Edge marking functions
	inline void markEdge(PropList *e, const Domain &s);
	inline void unmarkEdge(PropList *e);
	inline Domain *getMark(PropList *e);
	
	// Problem wrapper functions
	inline const Domain& bottom(void) const;
	inline const Domain& entry(void) const;
	inline void lub(Domain &a, const Domain &b) const;
	inline void assign(Domain &a, const Domain &b) const;
	inline bool equals(const Domain &a, const Domain &b) const;
	inline void update(Domain &out, const Domain &in, BasicBlock* bb);
	inline void blockInterpreted(BasicBlock* bb, const Domain& in, const Domain& out, CFG *cur_cfg, elm::genstruct::Vector<Edge*> *callStack) const;
	inline void fixPointReached(BasicBlock* bb) const;
	inline void enterContext(Domain &dom, BasicBlock* bb, util::hai_context_t ctx) const;
	inline void leaveContext(Domain &dom, BasicBlock* bb, util::hai_context_t ctx) const;
	
};
	
template < class Listener >	
inline void FirstUnrollingFixPoint<Listener >::init(util::HalfAbsInt<FirstUnrollingFixPoint> *_ai) {
		ai = _ai;
}

template < class Listener >	
inline int FirstUnrollingFixPoint<Listener>::getIter(BasicBlock *bb) const {
		return(ai->getFixPointState(bb)->numIter);
}
	
template < class Listener >	
void FirstUnrollingFixPoint<Listener >::fixPoint(BasicBlock *bb, bool &fixpoint, Domain &in, bool firstTime) const {
		
		FixPointState *fpstate = ai->getFixPointState(bb);
		Domain newHeaderState(bottom());
		fixpoint = false;
		
				
		/*
 		* The fixPoint() method actions depends on the current iteration.
 		*/
		switch(fpstate->numIter) {
			case 0:
				/* We never did any iteration, we begin the first:
				 * Use for IN the union of entry edges. Fixpoint is always false.
				 */
				assign(newHeaderState, ai->entryEdgeUnion(bb));
				break;
			case 1:
				/* We finished the first iteration, we begin the 2nd:
				 * Use for IN the union of back edges. Fixpoint may be reached at this point
				 * but even if it is, we still want to do another iteration anyway.
				 */
				assign(newHeaderState, ai->backEdgeUnion(bb));
				assign(fpstate->firstIterState, newHeaderState);
				
				break;				
			default:
				/* We finished the 2..n^th iteration:
				 * Use for IN the union of back edges + the union of entry edges 
				 * of the first iteration.
				 * Test for fixpoint.
				 */
				
				assign(newHeaderState, ai->backEdgeUnion(bb));
				prob.lub(newHeaderState, fpstate->firstIterState);
				
				if (prob.equals(newHeaderState, fpstate->headerState))
					fixpoint = true;
				
				break;
		}
		fpstate->numIter ++;
		
		/* Store the new loop header state in the FixPointState */
		assign(fpstate->headerState, newHeaderState);
		
		/* Pass the new loop header state to the caller (HalfAbsInt) */
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
		if (STATE(e) == NULL)
			STATE(e) = new Domain(bottom());
			
		prob.lub(*STATE(e), s);
	}
	
template < class Listener >	
inline void FirstUnrollingFixPoint<Listener >::unmarkEdge(PropList *e) {
		delete STATE(e);
		STATE(e) = NULL;
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
inline void FirstUnrollingFixPoint<Listener>::update(Domain &out, const typename Problem::Domain &in, BasicBlock* bb)  {
		prob.update(out,in,bb);
}
	
template < class Listener >	
inline void FirstUnrollingFixPoint<Listener>::blockInterpreted(BasicBlock* bb, const typename Problem::Domain& in, const typename Problem::Domain& out, CFG *cur_cfg, elm::genstruct::Vector<Edge*> *callStack) const {
		list.blockInterpreted(this, bb, in, out, cur_cfg, callStack);
}

template < class Listener >	
inline void FirstUnrollingFixPoint<Listener >::fixPointReached(BasicBlock* bb) const {
		list.fixPointReached(this, bb);
}
	
template < class Listener >	
inline void FirstUnrollingFixPoint<Listener >::enterContext(Domain &dom, BasicBlock* bb, util::hai_context_t ctx) const {
		prob.enterContext(dom, bb, ctx);
}
	
template < class Listener >	
inline void FirstUnrollingFixPoint<Listener>::leaveContext(Domain &dom, BasicBlock* bb, util::hai_context_t ctx) const {
		prob.leaveContext(dom, bb, ctx);
}
	
	
}

#endif /*UTIL_FIRSTUNROLLINGFIXPOINT_H_*/
