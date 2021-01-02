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
#include <elm/data/VectorQueue.h>
#include <elm/data/Vector.h>
#include <otawa/cfg/CFG.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/cfg/Dominance.h>
#include <otawa/cfg/Edge.h>
#include <otawa/cfg/features.h>
#include <otawa/prop/Identifier.h>
#include <otawa/prog/WorkSpace.h>
#ifdef	HAI_JSON
#	include <otawa/dfa/Debug.h>
#endif


// debugging
#if defined(NDEBUG) || !defined(HAI_DEBUG)
#	define HAI_TRACE(x)
#else
#	define HAI_TRACE(x) cerr << x << io::endl
#endif

namespace otawa { namespace dfa { namespace hai {

using namespace elm;

#ifdef HAI_JSON
#	ifndef HAI_JSON_LOG
#		define HAI_JSON_LOG "log.json"
#	endif
#	ifndef HAI_JSON_NAME
#		define HAI_JSON_NAME
#	endif
#	define HAI_BASE		debug##HAI_JSON_NAME
	Debug *HAI_BASE = 0;
#endif

// context definition
typedef enum hai_context_t {
	CTX_LOOP = 0,
	CTX_FUNC = 1
} hai_context_t;

// configuration
extern Identifier<bool> FIXED;
extern Identifier<bool> FIRST_ITER;
extern Identifier<bool> HAI_DONT_ENTER;
extern Identifier<Block*> HAI_BYPASS_SOURCE;
extern Identifier<Block*> HAI_BYPASS_TARGET;
extern Identifier<bool> HAI_INFINITE_LOOP;

// abstract interpretater
template <class FixPoint>
class HalfAbsInt {
public:
	inline HalfAbsInt(FixPoint& _fp, WorkSpace& _fw);
	inline ~HalfAbsInt(void);
  	inline typename FixPoint::FixPointState *getFixPointState(Block *bb);
	int solve(otawa::CFG *main_cfg = 0,
		typename FixPoint::Domain *entdom = 0, Block *start_bb = 0);
	inline typename FixPoint::Domain backEdgeUnion(Block *bb);
	inline typename FixPoint::Domain entryEdgeUnion(Block *bb);
	template <class GC> inline void collect(const GC* gc) const;

private:
	FixPoint& fp;
	WorkSpace &ws;
	CFG& entry_cfg;
	CFG *cur_cfg;
	Vector<Block*> *workList;
	Vector<Edge*> *callStack;
	Vector<CFG*> *cfgStack;
	Block *current;
	typename FixPoint::Domain in,out;
	Edge *next_edge;
	bool enter_call; /* enter_call == true: we need to process this call. enter_call == false: already processed (call return) */
	bool fixpoint;
	bool mainEntry;
	static Identifier<typename FixPoint::FixPointState*> FIXPOINT_STATE;
	inline bool isEdgeDone(Edge *edge);
	inline bool tryAddToWorkList(Block *bb);
	Edge *detectCalls(bool &enter_call, Vector<Edge*> &call_edges, Block *bb);
	void inputProcessing(typename FixPoint::Domain &entdom);
	void outputProcessing(void);
	void addSuccessors(void);
};



template <class FixPoint> template<class GC>
inline void HalfAbsInt<FixPoint>::collect(const GC *gc) const {
	in.collect(gc, 0);
	out.collect(gc, 0);

	// need to collect fix point state for BB who has it
	const CFGCollection *col = INVOLVED_CFGS(ws);
	for (int ci = 0; ci < col->count();  ci++) {
		CFG *cfg = col->get(ci);
		for (int j = 0; j < cfg->count(); j++) {
			Block *b = cfg->at(j);
			typename FixPoint::FixPointState* fps = FIXPOINT_STATE(b);
			if(fps != 0) {
				(fps->headerState).collect(gc);
			}

		} // for each BB
	} // for each CFG
}

template <class FixPoint>
Identifier<typename FixPoint::FixPointState*> HalfAbsInt<FixPoint>::FIXPOINT_STATE("", 0);

template <class FixPoint>
inline HalfAbsInt<FixPoint>::HalfAbsInt(FixPoint& _fp, WorkSpace& _fw)
:	fp(_fp),
 	ws(_fw),
 	entry_cfg(**ENTRY_CFG(_fw)),
 	cur_cfg(0),
 	current(0),
 	in(_fp.bottom()),
 	out(_fp.bottom()),
 	next_edge(0),
 	enter_call(0),
 	fixpoint(0),
 	mainEntry(false)
{
	workList = new Vector<Block*>();
	callStack = new Vector<Edge*>();
	cfgStack = new Vector<CFG*>();
	fp.init(this);
}


template <class FixPoint>
inline HalfAbsInt<FixPoint>::~HalfAbsInt(void) {

	// clean worklist
	delete workList;
	delete callStack;
	delete cfgStack;

	// clean remaining states
	List<CFG* > cfgs;
	cfgs.add(&entry_cfg);
	while(cfgs.count()) { // for(int i = 0; i < cfgs.count(); i++)
		CFG* cfg = cfgs.pop();
		for(CFG::BlockIter bb = cfg->blocks(); bb(); bb++)
			for(BasicBlock::EdgeIter out = bb->outs(); out(); out++) {
				this->fp.unmarkEdge(*out);
				if(out->sink()->isSynth() && out->sink()->toSynth()->callee() && !cfgs.contains(out->sink()->toSynth()->callee()))
					cfgs.addLast(out->sink()->toSynth()->callee());
			}
	}
}



template <class FixPoint>
inline typename FixPoint::FixPointState *HalfAbsInt<FixPoint>::getFixPointState(Block *bb) {
	return(FIXPOINT_STATE(bb));
}

/**
 * This function computes the IN state, and unmark the not-needed-anymore edges.
 * @param entdom
 */
template <class FixPoint>
void HalfAbsInt<FixPoint>::inputProcessing(typename FixPoint::Domain &entdom) {
	fp.assign(in, fp.bottom());

	// exit with remaining back-end
    // if unknown vertex, look for dead-end blocks
    if(current->isExit() && current->cfg()->unknown() != 0) {

    	// find remaining block
    	int rem = -1;
    	for(int i = workList->length() - 1; i >= 0; i--)
    		if(workList->get(i)->cfg() == current->cfg()) {
    			rem = i;
    			break;
    		}

    	// re-schedule remaining
    	if(rem >= 0) {
    		Block *bb = workList->get(rem);
    		workList->set(rem, current);
    		current = bb;
    		HAI_TRACE("\tDELAYED BY " << current);
    	}
    }

	// main entry case
	// TODO case not really needed: put entry domain in the fp directly -> aggregate with next case
	if(mainEntry) {
		fp.assign(in, entdom);
		mainEntry = false;
		HAI_TRACE("\t\tentry state = " << in);
	}

	// function-call entry case, state is on the called CFG's entry
	else if(current->isEntry()) {
		fp.assign(in, *fp.getMark(current));
		fp.unmarkEdge(current);
	}

	// loop header case: launch fixPoint()
	else if(LOOP_HEADER(current)) {

		// Compute the IN thanks to the fixpoint handler
		if(FIRST_ITER(current))
        	FIXPOINT_STATE(current) = fp.newState();

		// compute if fix-point is reached
    	fp.fixPoint(current, fixpoint, in, FIRST_ITER(current));
    	HAI_TRACE("\t\tat loop header " << current << ", fixpoint reached = " << fixpoint);
    	// TODO ever perform it
    	if(FIRST_ITER(current)) {
    		ASSERTP(!fixpoint, "ERROR: achieving fixpoint at the first iteration"); // achieving fixpoint at the first iteration is impossible, hence the ASSERT
    		FIRST_ITER(current) = false;
    	}
        FIXED(current) = fixpoint;

        /* Unmark edges depending on the fixpoint status.
         * If fixpoint, unmark all in-edges, else unmark only back-edges
         */

    	// In any case, the values of the back-edges are not needed anymore
    	for(Block::EdgeIter inedge = current->ins(); inedge(); inedge++) {
    		// TODO no more needed case (isn't it?)
    		// REMOVED: if the header of the loop is a SynthBlock, this will not remove the states from the back edges.
    		//if(inedge->sink()->isSynth())
    		//	continue;
    		if(Dominance::dominates(current, inedge->source()))
    			fp.unmarkEdge(*inedge);
    	}

		if(fixpoint) {

			// cleanups associated with end of the processing of a loop
			fp.fixPointReached(current);
			delete FIXPOINT_STATE(current);
			// TODO it's better to remove the property here
			// FIXPOINT_STATE(current) = 0;
			FIXPOINT_STATE(current).remove();
			// TODO FIRST_ITER should work in reverse direction
			FIRST_ITER(current) = true;

			// TODO duplication with case above?
			// The values of the entry edges are not needed anymore
	    	for(Block::EdgeIter inedge = current->ins(); inedge(); inedge++) {
	    		// TODO no more needed
//	    		if(inedge->sink()->isSynth())
//					continue;
				/* TODO fix when inlined virtualization will be re-activated
	    		if(HAI_BYPASS_TARGET(current) && (inedge->kind() == Edge::VIRTUAL_RETURN))
					continue;*/
//	    		else if(!Dominance::dominates(current, inedge->source()))
				fp.unmarkEdge(*inedge);
	    	}
	    	if (HAI_BYPASS_TARGET(current))
	    		fp.unmarkEdge(current);
		}
	}

	// Case of the simple basic block: IN = union of the OUTs of the predecessors
	else {

		// un-mark all the in-edges since the values are not needed.
		for(Block::EdgeIter inedge = current->ins(); inedge(); inedge++) {
			/* TODO fix when inlined virtualization will be re-activated
			if (HAI_BYPASS_TARGET(current) && (inedge->kind() == Edge::VIRTUAL_RETURN))
				continue;*/
			typename FixPoint::Domain *edgeState = fp.getMark(*inedge);
			ASSERTP(edgeState, "no state for " << *inedge  << " (" << inedge->source()->cfg() << ")");
			fp.updateEdge(*inedge, *edgeState);
			fp.lub(in, *edgeState);
			fp.unmarkEdge(*inedge);
		}
		if(HAI_BYPASS_TARGET(current)) {
			typename FixPoint::Domain *bypassState = fp.getMark(current);
			ASSERT(bypassState);
			fp.lub(in, *bypassState);
			fp.unmarkEdge(current);
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
		Vector<Block*> alreadyAdded;
		if((!EXIT_LIST(current)) || EXIT_LIST(current)->isEmpty()) {
			cerr << "WARNING: infinite loop found at CFG " << current->cfg()->index() << " BB " << current << io::endl;
			HAI_INFINITE_LOOP(current) = true;
		}
		else
	        for (Vector<Edge*>::Iter iter(**EXIT_LIST(current)); iter(); iter++) {
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
#		ifdef HAI_JSON
			json::Saver& saver = HAI_BASE->addState(current);
#		endif
       	fp.update(out, in, current);
#		ifdef HAI_JSON
        	fp.dumpJSON(out, saver);
#		endif

		// restore previous context
		Edge *edge = callStack->pop();
		cur_cfg = cfgStack->pop();
		HAI_TRACE("\t\treturning to CFG " << cur_cfg->label());
		fp.leaveContext(out, cur_cfg->entry(), CTX_FUNC);

#		ifdef WITHOUT_SLICING
		// record function output state
		fp.markEdge(edge, out);
		tryAddToWorkList(edge->sink()); //workList->push(edge->sink());
#		else
		// the output state of a block should be assigned to all the out-going edges of the block
		// such edges could be created as a result of slicing
		Block* b = edge->source();

		for(Block::EdgeIter bei = b->outs(); bei(); bei++) {
			fp.markEdge(*bei, out);

			tryAddToWorkList(bei->sink());
		}
#		endif
	}

	// from synthetic block
	else if(current->isSynth()) {
		// TODO		Support for multiple successors in leaving edge
		Edge *return_edge = *current->outs();

		// unknown CFG
		if(!current->toSynth()->callee()) {
			fp.assign(out, fp.top());
			fp.update(out, fp.top(), current); // the unknown block will have top state since we can not make any assumption on its behavior
			fp.markEdge(return_edge, out);
			workList->push(return_edge->sink());
		}

		// the CFG contains an endless loop such that it never returns, make the output of the function to be bottom
		// as it will not contribute any states to its return edge
		else if(!current->toSynth()->callee()->exit()->ins()) {
			fp.assign(out, fp.bottom());
			fp.markEdge(return_edge, out);
			tryAddToWorkList(return_edge->target());
		}
		// push call context
		else {
			// push call context
			callStack->push(return_edge);
	        cfgStack->push(cur_cfg);
	        cur_cfg = current->toSynth()->callee();

	        fp.enterContext(out, cur_cfg->entry(), CTX_FUNC);
			HAI_TRACE("\tcalling CFG " << cur_cfg->label());

	        // propagate to function entry
			fp.assign(out, in);
			fp.markEdge(cur_cfg->entry(), out); // the state of the block = the output state = the input state
			workList->push(cur_cfg->entry());
		}
	}

	// Standard case, update and propagate state to successors
	else {
		HAI_TRACE("\t\tupdating " << current);
#		ifdef HAI_JSON
			json::Saver& saver = HAI_BASE->addState(current);
#		endif
        fp.update(out, in, current);
        fp.blockInterpreted(current, in, out, cur_cfg, callStack);
#		ifdef HAI_JSON
        	fp.dumpJSON(out, saver);
#		endif

        addSuccessors();
	}
}

template <class FixPoint>
void HalfAbsInt<FixPoint>::addSuccessors() {

	for(Block::EdgeIter outedge = current->outs(); outedge(); outedge++) {

		/*if(outedge->sink()->isSynth())
	    	continue;*/

		/* TODO fix it when inline virtualization will be re-activated
		if (HAI_BYPASS_SOURCE(current) && outedge->kind() == Edge::VIRTUAL_CALL)
			continue;*/
		if (HAI_DONT_ENTER(outedge->target()))
			continue;

		fp.markEdge(*outedge, out);
	    tryAddToWorkList(outedge->target());
	}

	if (HAI_BYPASS_SOURCE(current)) {
		fp.markEdge(HAI_BYPASS_SOURCE(current), out);
	    HAI_TRACE("marking bypass out-edge " << current << " -> " << HAI_BYPASS_SOURCE(current));
		tryAddToWorkList(HAI_BYPASS_SOURCE(current));
	}
}


/**
 * This function build the list of call edges and return the next call edge to process.
 * @param enter_call	True if all calls have been processed, false else.
 * @param call_edges	List of call edges (out).
 * @param bb			Current block.
 * @return				Next call to process.
 */
template <class FixPoint>
Edge *HalfAbsInt<FixPoint>::detectCalls(bool &enter_call, Vector<Edge*> &call_edges, Block *bb) {
	// TODO in CFGv1, even with a non-inlined graph, call computations are performed separately.
	// In v2, input must accumulated and output propagated to all call site. To fix.
	Edge *next_edge = 0;
	enter_call = true;
	call_edges.clear();
    for(Block::EdgeIter outedge = bb->outs(); outedge(); outedge++)
    	// TODO special support with unknown sink is needed here
    	if(outedge->sink()->isSynth() && !HAI_DONT_ENTER(outedge->sink()->toSynth()->callee())) {
    		call_edges.add(*outedge);
    		if(fp.getMark(*outedge))
    			enter_call = false;
    		else if(!next_edge)
    			next_edge = *outedge;
    	}
    return (next_edge);
}


template <class FixPoint>
int HalfAbsInt<FixPoint>::solve(otawa::CFG *main_cfg, typename FixPoint::Domain *entdom, Block *start_bb) {
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

    // json debugging initialization
#	ifdef HAI_JSON
    	HAI_BASE = new Debug(&ws, HAI_JSON_LOG);
#	endif

	// work list initialization
	cur_cfg = main_cfg;
	workList->push(start_bb);

	// main loop
	while(!workList->isEmpty()) {

		// next step
		iterations++;
		fixpoint = false;
		current = workList->pop();

		// update the state
		//next_edge = detectCalls(enter_call, call_edges, current);
		HAI_TRACE("\n\tPROCESSING " << current << " (" << current->cfg() << ")");
		inputProcessing(*entdom);
		HAI_TRACE("\t\tunion of inputs = " << in);
		outputProcessing();
		HAI_TRACE("\t\toutput = " << out);

		if(!current->isExit() && workList->isEmpty()) {
			// now we need to check the out-going edge
			for(Block::EdgeIter bei=current->outs(); bei(); bei++) {
				if(bei->target() != current)
					if(!HAI_INFINITE_LOOP(current))
						ASSERTP(false, "HalfAbsInt finishes at CFG " << current->cfg()->index() << ", " << current << ", does not end with the exit block. (iteration = " << iterations << ")");
			} // end of each output edge of the current block
		} // end of the current block is not an exit block, and workList is empty
	}

    // json debugging finalization
#	ifdef HAI_JSON
    	delete HAI_BASE;
    	HAI_BASE = 0;
#	endif

	return(iterations);
}


template <class FixPoint>
inline typename FixPoint::Domain HalfAbsInt<FixPoint>::backEdgeUnion(Block *bb) {
        typename FixPoint::Domain result(fp.bottom());

        HAI_TRACE("\t\tunion back edges");
        if (FIRST_ITER(bb)) {
        	/* If this is the first iteration, the back edge union is Bottom. */
        	return(result);
        }
        for(Block::EdgeIter inedge = bb->ins(); inedge(); inedge++) {
        		if(inedge->sink()->isSynth())
            			continue;
        		else if (Dominance::dominates(bb, inedge->source())) {
                        typename FixPoint::Domain *edgeState = fp.getMark(*inedge);
                        ASSERT(edgeState);
                        HAI_TRACE("\t\t\twith " << *inedge << " = " << *edgeState);
#						ifdef FILTERING_BEFORE_WIDENING
                        fp.updateEdge(*inedge, *edgeState);
#						endif
                        fp.lub(result, *edgeState);
                }

        }
        return(result);
}



template <class FixPoint>
inline typename FixPoint::Domain HalfAbsInt<FixPoint>::entryEdgeUnion(Block *bb) {
	typename FixPoint::Domain result(fp.bottom());

	HAI_TRACE("\t\tunion of entry edges");
	for(Block::EdgeIter inedge = bb->ins(); inedge(); inedge++) {
		// REMOVED: this statement needs to be removed otherwise the state from the input edge to a SynthBlock will not be used.
		//if (inedge->sink()->isSynth())
		//	continue;
		/* TODO fix it when Inlining will be  re-activated.
		if(HAI_BYPASS_TARGET(bb) && (inedge->kind() == Edge::VIRTUAL_RETURN))
			continue;*/
		if(!Dominance::dominates(bb, inedge->source())) {
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
	return(result);
}


template <class FixPoint>
inline bool HalfAbsInt<FixPoint>::tryAddToWorkList(Block *bb) {

	if(HAI_BYPASS_TARGET(bb)) {
		typename FixPoint::Domain *bypassState = fp.getMark(bb);
		if(!bypassState) {
			return false;
		}
    }

	else { // for the other kinds of blocks
		for (Block::EdgeIter inedge = bb->ins(); inedge(); inedge++) {
			// 1. with program slicing, it is possible to have a synth block connecting to a synth block directly
			// to be safe, we check every incoming edge to the block
			// 2. the function isEdgeDone checks if the edge is the exit edge of a loop header, and see if the loop header reaches the fix-point. if it doesnt, then return false
			if(!isEdgeDone(*inedge)) { // if any of the incoming edge is not associated with a state, then we don't process the block
				return false;
			}
		}
		workList->push(bb);
		return true;
	}

	// compute if the block must be added
	bool add = true;
	if(HAI_BYPASS_TARGET(bb)) {
		typename FixPoint::Domain *bypassState = fp.getMark(bb);
		if(!bypassState) {
			add = false;
		}
    }
	if(add)
		for (Block::EdgeIter inedge = bb->ins(); inedge(); inedge++) {
#		ifdef WITHOUT_SLICING
			if(inedge->sink()->isSynth())
				continue;
			/* TODO		fix with (a) virtualization
				 if (HAI_BYPASS_TARGET(bb) && (inedge->kind() == Edge::VIRTUAL_RETURN))
				continue;*/
			else if(!isEdgeDone(*inedge))
				add = false;
#		else
			// 1. with program slicing, it is possible to have a synth block connecting to a synth block directly
			// to be safe, we check every incoming edge to the block
			// 2. the function isEdgeDone checks if the edge is the exit edge of a loop header, and see if the loop header reaches the fix-point. if it doesnt, then return false
			if(!isEdgeDone(*inedge))
				add = false;
#		endif
		}

	// if required, add the block
	if (add) {
#		ifdef DEBUG
			if (LOOP_HEADER(bb)) {
				if (FIRST_ITER(bb) == true)
					cerr << "Ignoring back-edges for loop header " << bb->number() << " because it's the first iteration.\n";
			}
#		endif
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

