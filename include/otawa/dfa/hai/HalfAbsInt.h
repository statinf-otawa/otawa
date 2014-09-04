/*
 *	HalfAbsInt class interface.
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


#ifndef OTAWA_DFA_HAI_HALFABSINT_H
#define OTAWA_DFA_HAI_HALFABSINT_H


#include <elm/assert.h>
#include <elm/genstruct/VectorQueue.h>
#include <elm/genstruct/Vector.h>
#include <otawa/cfg/CFG.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/cfg/Edge.h>
#include <otawa/util/Dominance.h>
#include <otawa/util/LoopInfoBuilder.h>
#include <otawa/prop/Identifier.h>
#include <otawa/prog/WorkSpace.h>


// debugging
#if defined(NDEBUG) || !defined(HAI_DEBUG)
#	define HAI_TRACE(x)
#else
#	define HAI_TRACE(x) cerr << x << io::endl
#endif

namespace otawa { namespace dfa { namespace hai {

// context definition
typedef enum hai_context_t {
	CTX_LOOP = 0,
	CTX_FUNC = 1
} hai_context_t;

// configuration
extern Identifier<bool> FIXED;
extern Identifier<bool> FIRST_ITER;
extern Identifier<bool> HAI_DONT_ENTER;
extern Identifier<BasicBlock*> HAI_BYPASS_SOURCE;
extern Identifier<BasicBlock*> HAI_BYPASS_TARGET;

// abstract interpretater
template <class FixPoint>
class HalfAbsInt {
private:
	FixPoint& fp;
	WorkSpace &fw;
	CFG& entry_cfg;
	CFG *cur_cfg;
	elm::genstruct::Vector<BasicBlock*> *workList;
	elm::genstruct::Vector<Edge*> *callStack;
	elm::genstruct::Vector<CFG*> *cfgStack;
	BasicBlock *current;
	typename FixPoint::Domain in,out;
	elm::genstruct::Vector<Edge*> call_edges; /* call_edge == current node's call-edge to another CFG */
	Edge *next_edge;
	bool enter_call; /* enter_call == true: we need to process this call. enter_call == false: already processed (call return) */
	bool fixpoint;
	bool mainEntry;

	static Identifier<typename FixPoint::FixPointState*> FIXPOINT_STATE;
	inline bool isEdgeDone(Edge *edge);
	inline bool tryAddToWorkList(BasicBlock *bb);
	Edge *detectCalls(bool &enter_call, elm::genstruct::Vector<Edge*> &call_edges, BasicBlock *bb);
	void inputProcessing(typename FixPoint::Domain &entdom);
	void outputProcessing();
	void addSuccessors();

public:
  	inline typename FixPoint::FixPointState *getFixPointState(BasicBlock *bb);
	int solve(otawa::CFG *main_cfg = 0,
		typename FixPoint::Domain *entdom = 0, BasicBlock *start_bb = 0);
	inline HalfAbsInt(FixPoint& _fp, WorkSpace& _fw);
	inline ~HalfAbsInt(void);
	inline typename FixPoint::Domain backEdgeUnion(BasicBlock *bb);
	inline typename FixPoint::Domain entryEdgeUnion(BasicBlock *bb);
};

template <class FixPoint>
Identifier<typename FixPoint::FixPointState*> HalfAbsInt<FixPoint>::FIXPOINT_STATE("", 0);

template <class FixPoint>
inline HalfAbsInt<FixPoint>::HalfAbsInt(FixPoint& _fp, WorkSpace& _fw)
:	fp(_fp),
 	fw(_fw),
 	entry_cfg(**ENTRY_CFG(_fw)),
 	cur_cfg(0),
 	in(_fp.bottom()),
 	out(_fp.bottom()),
 	next_edge(0),
 	fixpoint(0),
 	current(0),
 	enter_call(0),
 	mainEntry(false)
{
	workList = new elm::genstruct::Vector<BasicBlock*>();
	callStack = new elm::genstruct::Vector<Edge*>();
	cfgStack = new elm::genstruct::Vector<CFG*>();
	fp.init(this);
}


template <class FixPoint>
inline HalfAbsInt<FixPoint>::~HalfAbsInt(void) {
	delete workList;

	// clean remaining states
	genstruct::Vector<CFG *> cfgs;
	cfgs.add(&entry_cfg);
	for(int i = 0; i < cfgs.count(); i++)
		for(CFG::BBIterator bb(cfgs[i]); bb; bb++)
			for(BasicBlock::OutIterator out(bb); out; out++) {
				this->fp.unmarkEdge(*out);
				if(out->kind() == Edge::CALL && !cfgs.contains(out->calledCFG()))
					cfgs.add(out->calledCFG());
			}
}



template <class FixPoint>
inline typename FixPoint::FixPointState *HalfAbsInt<FixPoint>::getFixPointState(BasicBlock *bb) {
	return(FIXPOINT_STATE(bb));
}

/* This function computes the IN state, and unmark the not-needed-anymore edges.
 */
template <class FixPoint>
void HalfAbsInt<FixPoint>::inputProcessing(typename FixPoint::Domain &entdom) {
	fp.assign(in, fp.bottom());

	// main entry case
	if (mainEntry) {
		fp.assign(in, entdom);
		mainEntry = false;
	}

	// function-call entry case, state is on the called CFG's entry
	else if (current->isEntry()) {
		fp.assign(in, *fp.getMark(current));
		fp.unmarkEdge(current);
	}

	// call return case, merge return-state for all functions called from this node
	else if (!next_edge && !call_edges.isEmpty()) {

		// join return edges of functions
		for (elm::genstruct::Vector<Edge*>::Iterator iter(call_edges); iter; iter++) {
			fp.lub(in, *fp.getMark(*iter));
			fp.unmarkEdge(*iter);
		}

		// cleanup entry edges
		for(BasicBlock::InIterator in(current); in; in++)
			fp.unmarkEdge(*in);
	}

	// loop header case: launch fixPoint()
	else if(LOOP_HEADER(current)) {

		// Compute the IN thanks to the fixpoint handler
		if(FIRST_ITER(current))
        	FIXPOINT_STATE(current) = fp.newState();
    	fp.fixPoint(current, fixpoint, in, FIRST_ITER(current));
    	HAI_TRACE("\t\tat loop header " << current << ", fixpoint reached = " << fixpoint);
    	if (FIRST_ITER(current)) {
    		ASSERT(!fixpoint);
    		FIRST_ITER(current) = false;
    	}
        FIXED(current) = fixpoint;

        /* Unmark edges depending on the fixpoint status.
         * If fixpoint, unmark all in-edges, else unmark only back-edges
         */

    	/* In any case, the values of the back-edges are not needed anymore */
    	for (BasicBlock::InIterator inedge(current); inedge; inedge++) {
    		if (inedge->kind() == Edge::CALL)
    			continue;
    		if (Dominance::dominates(current, inedge->source())) {
    			fp.unmarkEdge(*inedge);
    			//HAI_TRACE("unmarking back-edge: " << inedge->source() << " -> " << inedge->target());
    		}
    	}

		if (fixpoint) {
			/* Cleanups associated with end of the processing of a loop */
			fp.fixPointReached(current);
			delete FIXPOINT_STATE(current);
			FIXPOINT_STATE(current) = 0;
			FIRST_ITER(current) = true;

			/* The values of the entry edges are not needed anymore */
	    	for (BasicBlock::InIterator inedge(current); inedge; inedge++) {
	    		if (inedge->kind() == Edge::CALL)
					continue;
				if (HAI_BYPASS_TARGET(current) && (inedge->kind() == Edge::VIRTUAL_RETURN))
					continue;
	    		if (!Dominance::dominates(current, inedge->source())) {
	    			//HAI_TRACE("unmarking entry-edge " << inedge->source() << " -> " << inedge->target());
					fp.unmarkEdge(*inedge);
	    		}
	    	}
	    	if (HAI_BYPASS_TARGET(current)) {
	    		fp.unmarkEdge(current);
	    	}
		}
	}

	/* Case of the simple basic block: IN = union of the OUTs of the predecessors. */
	else {

		/* Un-mark all the in-edges since the values are not needed.  */
		for (BasicBlock::InIterator inedge(current); inedge; inedge++) {
			ASSERT(inedge->kind() != Edge::CALL);
			if (HAI_BYPASS_TARGET(current) && (inedge->kind() == Edge::VIRTUAL_RETURN))
				continue;
			typename FixPoint::Domain *edgeState = fp.getMark(*inedge);
			//HAI_TRACE("got state: " << *edgeState << "\n");
			
			ASSERT(edgeState);
			fp.updateEdge(*inedge, *edgeState);
			fp.lub(in, *edgeState);
			if(!next_edge)
				fp.unmarkEdge(*inedge);
            //HAI_TRACE("unmarking in-edge " << inedge->source() << " -> " << inedge->target());

		}
		if (HAI_BYPASS_TARGET(current)) {
			typename FixPoint::Domain *bypassState = fp.getMark(current);
			ASSERT(bypassState);
			fp.lub(in, *bypassState);
			fp.unmarkEdge(current);
            //HAI_TRACE("unmarking bypass in-edge " << HAI_BYPASS_TARGET(current) << " -> " << current);
		}
	}
}

/* This function computes the out-state, propagates it to the needed edges, and
 * try to add the next node(s) to the worklist.
 */
template <class FixPoint>
void HalfAbsInt<FixPoint>::outputProcessing(void) {

	// Fixpoint reached: activate the associated loop-exit-edges
	if(LOOP_HEADER(current) && fixpoint) {
        elm::genstruct::Vector<Edge*> *vec;
        vec = EXIT_LIST(current);

		genstruct::Vector<BasicBlock*> alreadyAdded;
		if ((!EXIT_LIST(current)) || EXIT_LIST(current)->isEmpty())
			cerr << "WARNING: infinite loop found at " << current << io::endl;
		else
	        for (elm::genstruct::Vector<Edge*>::Iterator iter(**EXIT_LIST(current)); iter; iter++) {
	           	HAI_TRACE("\t\tpushing edge " << iter->source() << " -> " << iter->target());
				if(!alreadyAdded.contains(iter->target()) && tryAddToWorkList(iter->target()))
	           		alreadyAdded.add(iter->target());
	           	fp.leaveContext(*fp.getMark(*iter), current, CTX_LOOP);
	        }
	}

	// Exit from function: pop callstack, mark edge with return state for the caller
	else if (current->isExit() && (callStack->length() > 0)) {

		// apply current block
		HAI_TRACE("\t\tupdating exit block while returning from call at " << current);
       	fp.update(out, in, current);

       	// restore previous context
       	Edge *edge = callStack->pop();
       	cur_cfg = cfgStack->pop();
       	HAI_TRACE("\t\treturning to CFG " << cur_cfg->label());
		fp.leaveContext(out, cur_cfg->entry(), CTX_FUNC);

		// record function output state
       	fp.markEdge(edge, out);
       	workList->push(edge->source());
	}

	// process call node, and there is still function calls to process
	else if (next_edge) {
		/* If first time, mark entry points. */
		if (enter_call) {
			HAI_TRACE("\t\tupdating for BB " << current << " while visiting call-node for the first time");
			fp.update(out, in, current);
		    for (elm::genstruct::Vector<Edge*>::Iterator iter(call_edges); iter; iter++) {
				// Mark all the function entries
				BasicBlock *func_entry = iter->calledCFG()->entry();
				fp.markEdge(func_entry, out);
		    }
		}

		// save current context
        callStack->push(next_edge);
        cfgStack->push(cur_cfg);

        // setup new context
        cur_cfg = next_edge->calledCFG();
        HAI_TRACE("\tcalling CFG " << cur_cfg->label());
        workList->push(cur_cfg->entry());
        fp.enterContext(out, cur_cfg->entry(), CTX_FUNC);
	}

	// visit call-node for the last-time, propagate state to successors
	else if (!call_edges.isEmpty()) {
		HAI_TRACE("\t\treturning from function calls at " << current);
		fp.assign(out, in);
		addSuccessors();
	}

	// Standard case, update and propagate state to successors
	else {
		HAI_TRACE("\t\tupdating " << current);
        fp.update(out, in, current);
        fp.blockInterpreted(current, in, out, cur_cfg, callStack);
        addSuccessors();
	}
}

template <class FixPoint>
void HalfAbsInt<FixPoint>::addSuccessors() {

	for (BasicBlock::OutIterator outedge(current); outedge; outedge++) {
	    if (outedge->kind() == Edge::CALL)
	    	continue;

	if (HAI_BYPASS_SOURCE(current) && outedge->kind() == Edge::VIRTUAL_CALL)
		continue;
	if (HAI_DONT_ENTER(outedge->target()))
		continue;

	    fp.markEdge(*outedge, out);

	    //HAI_TRACE("marking edge " << outedge->source() << " -> " << outedge->target() << "\n");

	    tryAddToWorkList(outedge->target());
	}

	if (HAI_BYPASS_SOURCE(current)) {

		fp.markEdge(HAI_BYPASS_SOURCE(current), out);

	    HAI_TRACE("marking bypass out-edge " << current << " -> " << HAI_BYPASS_SOURCE(current));

		tryAddToWorkList(HAI_BYPASS_SOURCE(current));
	}
}

template <class FixPoint>
Edge *HalfAbsInt<FixPoint>::detectCalls(bool &enter_call, elm::genstruct::Vector<Edge*> &call_edges, BasicBlock *bb) {
	Edge *next_edge = 0;
	enter_call = true;
	call_edges.clear();
    for (BasicBlock::OutIterator outedge(bb); outedge; outedge++) {
    	if ((outedge->kind() == Edge::CALL) && (!HAI_DONT_ENTER(outedge->calledCFG()))) {
    		call_edges.add(*outedge);
    		if (fp.getMark(*outedge)) {
    			enter_call = false;
    		} else if (!next_edge)
    			next_edge = *outedge;
    	}
	}
    return(next_edge);
}


template <class FixPoint>
int HalfAbsInt<FixPoint>::solve(otawa::CFG *main_cfg, typename FixPoint::Domain *entdom, BasicBlock *start_bb) {
	int iterations = 0;
    typename FixPoint::Domain default_entry(fp.entry());

    // initialization
    workList->clear();
    callStack->clear();
    mainEntry = true;
    if(!main_cfg)
    	main_cfg = &entry_cfg;
    if (!start_bb)
    	start_bb = main_cfg->entry();
    if(!entdom)
    	entdom = &default_entry;

    ASSERT(main_cfg);
    ASSERT(start_bb);

	// work list initialization
	cur_cfg = main_cfg;
	workList->push(start_bb);

	// main loop
	while (!workList->isEmpty()) {
		iterations++;
		fixpoint = false;
		current = workList->pop();
		next_edge = detectCalls(enter_call, call_edges, current);

		HAI_TRACE("\n\tPROCESSING " << current);
		inputProcessing(*entdom);
		HAI_TRACE("\t\tunion of inputs = " << in);
		outputProcessing();
		HAI_TRACE("\t\toutput = " << out);
	}
	return(iterations);
}


template <class FixPoint>
inline typename FixPoint::Domain HalfAbsInt<FixPoint>::backEdgeUnion(BasicBlock *bb) {
        typename FixPoint::Domain result(fp.bottom());

        HAI_TRACE("\t\tjoining back edges");
        if (FIRST_ITER(bb)) {
        	/* If this is the first iteration, the back edge union is Bottom. */
        	return(result);
        }
        for (BasicBlock::InIterator inedge(bb); inedge; inedge++) {
        		if (inedge->kind() == Edge::CALL)
            			continue;
                if (Dominance::dominates(bb, inedge->source())) {
                        typename FixPoint::Domain *edgeState = fp.getMark(*inedge);
                        ASSERT(edgeState);
                        HAI_TRACE("\t\t\twith " << *inedge << " = " << *edgeState);
                        fp.lub(result, *edgeState);
                }

        }
        //HAI_TRACE("\t\tgiving " << result);
        return(result);
}



template <class FixPoint>
inline typename FixPoint::Domain HalfAbsInt<FixPoint>::entryEdgeUnion(BasicBlock *bb) {
        typename FixPoint::Domain result(fp.bottom());

        HAI_TRACE("\t\tunion of input edges");
        for (BasicBlock::InIterator inedge(bb); inedge; inedge++) {
        		if (inedge->kind() == Edge::CALL)
            			continue;
            	if (HAI_BYPASS_TARGET(bb) && (inedge->kind() == Edge::VIRTUAL_RETURN))
            		continue;
                if (!Dominance::dominates(bb, inedge->source())) {
                        typename FixPoint::Domain *edgeState = fp.getMark(*inedge);
                        ASSERT(edgeState);
                        HAI_TRACE("\t\t\twith " << *inedge << " = " << *edgeState);
                        fp.lub(result, *edgeState);
                }

        }
        if (HAI_BYPASS_TARGET(bb)) {
        	typename FixPoint::Domain *bypassState = fp.getMark(bb);
        	ASSERT(bypassState);
        	fp.lub(result, *bypassState);
        }
        fp.enterContext(result, bb, CTX_LOOP);
        //HAI_TRACE("\t\tgiving " << result);
        return(result);
}


template <class FixPoint>
inline bool HalfAbsInt<FixPoint>::tryAddToWorkList(BasicBlock *bb) {
	bool add = true;
	for (BasicBlock::InIterator inedge(bb); inedge; inedge++) {
		if (inedge->kind() == Edge::CALL)
            			continue;
        if (HAI_BYPASS_TARGET(bb) && (inedge->kind() == Edge::VIRTUAL_RETURN))
        	continue;
		if (!isEdgeDone(*inedge)) {
			add = false;
			//HAI_TRACE("edge is not done !\n");
		}
	}
	if (HAI_BYPASS_TARGET(bb)) {
        typename FixPoint::Domain *bypassState = fp.getMark(bb);
        if (!bypassState)
        	add = false;
    }

	if (add) {
		if (LOOP_HEADER(bb)) {
#			ifdef DEBUG
				if (FIRST_ITER(bb) == true)
					cerr << "Ignoring back-edges for loop header " << bb->number() << " because it's the first iteration.\n";
#			endif
		}
		HAI_TRACE("\t\tadding " << bb << " to worklist");
		workList->push(bb);
	}
	return(add);
}


template <class FixPoint>
inline bool HalfAbsInt<FixPoint>::isEdgeDone(Edge *edge) {
	/*
	 * If we have a back edge and the target is a loop header at its first iteration, then we may add the target to
	 * the worklist even if the edge status is not calculated yet.
	 *
	 * If other case, we test if the edge status is calculated. If yes, we need to make another check: if the edge is a loop
	 * exit edge, then the loop's fixpoint must have been reached.
	 *
	 * XXX The evaluation order of the conditions is important XXX
	 */
	return (
		(fp.getMark(edge) && (!LOOP_EXIT_EDGE(edge) || FIXED(LOOP_EXIT_EDGE(edge))))
		|| (Dominance::dominates(edge->target(), edge->source()) && FIRST_ITER(edge->target()))

	);
}


} } } // otawa::dfa:hai

#endif // OTAWA_DFA_HAI_HALFABSINT_H

