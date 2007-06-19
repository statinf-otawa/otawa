/*
 * $Id$
 * Copyright (c) 2005 IRIT-UPS
 *
 * include/otawa/util/HalfAbsint.h -- "Half" abstract interpretation class interface.
 */
#ifndef OTAWA_UTIL_HALFABSINT_H
#define OTAWA_UTIL_HALFABSINT_H


#include <assert.h>
#include <elm/genstruct/Table.h>
#include <elm/genstruct/VectorQueue.h>
#include <elm/genstruct/Vector.h>
#include <otawa/cfg.h>
#include <otawa/cfg/Edge.h>
#include <otawa/dfa/BitSet.h>
#include <otawa/otawa.h>
#include <otawa/util/Dominance.h>
#include <otawa/util/LoopInfoBuilder.h>

namespace otawa { namespace util {

extern Identifier<bool> FIXED;
extern Identifier<bool> FIRST_ITER;
extern Identifier<bool> HAI_DONT_ENTER;

template <class FixPoint>
class HalfAbsInt {
	
  private:
	FixPoint& fp;
	FrameWork &fw;
	CFG& entry_cfg;	
	CFG *cur_cfg;
	elm::genstruct::VectorQueue<BasicBlock*> *workList;	
	elm::genstruct::Vector<Edge*> *callStack;
	elm::genstruct::Vector<CFG*> *cfgStack;
	BasicBlock *current;
	typename FixPoint::Domain in,out;
	Edge *call_edge; /* call_edge == current node's call-edge to another CFG */
	bool call_node; /* call_node == true: we need to process this call. call_node == false: already processed (call return) */
	bool fixpoint;
	
	Identifier<typename FixPoint::FixPointState*> FIXPOINT_STATE;	
	inline bool isEdgeDone(Edge *edge);	
	inline bool tryAddToWorkList(BasicBlock *bb);
	Edge *detectCalls(bool &call_node, BasicBlock *bb);
	void inputProcessing();
	void outputProcessing();
	
  public:
  	inline typename FixPoint::FixPointState *getFixPointState(BasicBlock *bb);
	int solve(otawa::CFG *main_cfg = NULL);
	inline HalfAbsInt(FixPoint& _fp, FrameWork& _fw);
	inline ~HalfAbsInt(void);
	inline typename FixPoint::Domain backEdgeUnion(BasicBlock *bb);
	inline typename FixPoint::Domain entryEdgeUnion(BasicBlock *bb);
	
	
};


template <class FixPoint>
inline HalfAbsInt<FixPoint>::HalfAbsInt(FixPoint& _fp, FrameWork& _fw)
 : entry_cfg(*ENTRY_CFG(_fw)), cur_cfg(ENTRY_CFG(_fw)), in(_fp.bottom()), out(_fp.bottom()), fw(_fw), fp(_fp), FIXPOINT_STATE("", NULL, otawa::NS) {
		workList = new elm::genstruct::VectorQueue<BasicBlock*>();
		callStack = new elm::genstruct::Vector<Edge*>();
		cfgStack = new elm::genstruct::Vector<CFG*>();
        fp.init(this);
}


template <class FixPoint>
inline HalfAbsInt<FixPoint>::~HalfAbsInt(void) {
	delete workList;
}



template <class FixPoint>
inline typename FixPoint::FixPointState *HalfAbsInt<FixPoint>::getFixPointState(BasicBlock *bb) {
	return(FIXPOINT_STATE(bb));
}

template <class FixPoint>
void HalfAbsInt<FixPoint>::inputProcessing() {
	
	fp.assign(in, fp.bottom());
	
	if (current->isEntry()) {
		if (fp.getMark(current)) {
			/* Function-call entry case */
			fp.assign(in, *fp.getMark(current));
			fp.unmarkEdge(current);
		}  else { 
			/* Main entry case */
			fp.assign(in, fp.entry());
		}
	} else if (call_edge && !call_node) {
		/* Call return case */
		fp.assign(in, *fp.getMark(call_edge));
		fp.unmarkEdge(call_edge);
	} else if (Dominance::isLoopHeader(current)) {
		/* Loop header case: launch fixPoint() */
		if (FIRST_ITER(current))
        		FIXPOINT_STATE(current) = fp.newState();
            	fp.fixPoint(current, fixpoint, in, FIRST_ITER(current));            	
#ifdef DEBUG
            	cout << "Loop header " << current->number() << ", fixpoint reached = " << fixpoint << "\n";
#endif            	
            	if (FIRST_ITER(current)) {
            		assert(!fixpoint);
            		FIRST_ITER(current) = false;
            	}
            	 
            	FIXED(current) = fixpoint;
            	
            	/* In any case, the values of the back-edges are not needed anymore */
            	for (BasicBlock::InIterator inedge(current); inedge; inedge++) {
            		if (inedge->kind() == Edge::CALL)
            			continue;
            		if (Dominance::dominates(current, inedge->source())) {
            			fp.unmarkEdge(*inedge);
#ifdef DEBUG        
            			cout << "Unmarking back-edge: " << inedge->source()->number() << "->" << inedge->target()->number() << "\n";
#endif            			
            		}
            	}
            	
            	if (fixpoint) {
            		/* Cleanups associated with end of the processing of a loop */
            		fp.fixPointReached(current);
            		delete FIXPOINT_STATE(current);
            		FIXPOINT_STATE(current) = NULL;
            		FIRST_ITER(current) = true;            	        		

            		/* The values of the entry edges are not needed anymore */
                	for (BasicBlock::InIterator inedge(current); inedge; inedge++) {
                		if (inedge->kind() == Edge::CALL)
            				continue;
                		if (!Dominance::dominates(current, inedge->source())) {
#ifdef DEBUG                			
                			cout << "Unmarking entry-edge: " << inedge->source()->number() << "->" << inedge->target()->number() << "\n";
#endif                			
            				fp.unmarkEdge(*inedge);
                		}
                	}
                	
            	}               	
	} else {
		/* Case of the simple basic block: IN = union of the OUTs of the predecessors. */
		/* Un-mark all the in-edges since the values are not needed.  */
		fp.assign(in,fp.bottom());
		for (BasicBlock::InIterator inedge(current); inedge; inedge++) {
			if (inedge->kind() == Edge::CALL)
				continue;
			typename FixPoint::Domain *edgeState = fp.getMark(*inedge);
			assert(edgeState != NULL);
			fp.lub(in, *edgeState);										
			fp.unmarkEdge(*inedge);
#ifdef DEBUG                    
                   	cout << "Unmarking in-edge: " << inedge->source()->number() << "->" << inedge->target()->number() << "\n";
#endif                     
		}   					
	} 
}

template <class FixPoint>
void HalfAbsInt<FixPoint>::outputProcessing() {
	if (Dominance::isLoopHeader(current) && fixpoint) {
		/* Fixpoint reached: activate the associated loop-exit-edges. */
            	elm::genstruct::Vector<Edge*> *vec;
            	vec = EXIT_LIST(current);
      
		genstruct::Vector<BasicBlock*> alreadyAdded;
            	for (elm::genstruct::Vector<Edge*>::Iterator iter(*EXIT_LIST(current)); iter; iter++) {
#ifdef DEBUG            		
            		cout << "Activating edge: " << iter->source()->number() << "->" << iter->target()->number() << "\n";
#endif            	
			if (!alreadyAdded.contains(iter->target())) {	
            			if (tryAddToWorkList(iter->target()))
            				alreadyAdded.add(iter->target());
			}

            		fp.leaveContext(*fp.getMark(*iter), current);

            	}            		
	} else {            	
            	/* Simple BasicBlock: try to add its sucessors to the worklist */
            	 
            	/* if call_edge && !call_node, then we're at a call return, we don't call update(), because we already
            	 * did it during the call processing */
            	if (call_node || !call_edge)
	            	fp.update(out, in, current);

#ifdef DEBUG            	
            	cout << "Updating for basicblock: " << current->number() << "\n"; 
#endif            	
            	fp.blockInterpreted(current, in, out, cur_cfg);
            	
            	if (current->isExit() && (callStack->length() > 0)) {
            		/* Exit from function: pop callstack, mark edge with return state for the caller */
            		int last_pos = callStack->length() - 1;
            		Edge *edge = callStack->pop();
            		cur_cfg = cfgStack->pop();
#ifdef DEBUG
            		cout << "Returning to CFG: " << cur_cfg->label() << "\n";
#endif
            		fp.markEdge(edge, out);
            		workList->put(edge->source());
            	}

            	if (call_node) {
            		/* Going into sub-function: push callstack, mark function entry with call state for the callee */
			BasicBlock *func_entry = call_edge->calledCFG()->entry();
            		callStack->push(call_edge);
            		cfgStack->push(cur_cfg);
            		cur_cfg = call_edge->calledCFG();
#ifdef DEBUG            		
            		cout << "Going to CFG: " << cur_cfg->label() << "\n";
#endif
            		workList->put(func_entry);
            		fp.markEdge(func_entry, out);
		} else {
        	    	/* Standard case: use out-state to mark out-edges, and try to add successors to worklist */
	            	for (BasicBlock::OutIterator outedge(current); outedge; outedge++) {
	            		if (outedge->kind() == Edge::CALL) 
	            			continue;
	            		fp.markEdge(*outedge, out);
#ifdef DEBUG           		
	            		cout << "Marking edge: " << outedge->source()->number() << "->" << outedge->target()->number() << "\n";
#endif            		
	            		tryAddToWorkList(outedge->target());
	            	}
            	}
	}
}


template <class FixPoint>
Edge *HalfAbsInt<FixPoint>::detectCalls(bool &call_node, BasicBlock *bb) {
	Edge *call_edge = NULL;

	call_node = false;
       	for (BasicBlock::OutIterator outedge(bb); outedge; outedge++) {
        	if ((outedge->kind() == Edge::CALL) && (!HAI_DONT_ENTER(outedge->calledCFG()))) {
        		call_edge = *outedge;
        		if (!fp.getMark(call_edge)) {
        			call_node = true;
        		}
        	}
	}
        return(call_edge);
}


template <class FixPoint>
int HalfAbsInt<FixPoint>::solve(otawa::CFG *main_cfg) {        
	int iterations = 0;
    
        /* workList / callStack initialization */
        workList->reset();
        callStack->clear();
        if (main_cfg != NULL) 
        	cur_cfg = main_cfg;
        workList->put((main_cfg != NULL) ? main_cfg->entry() : entry_cfg.entry());
#ifdef DEBUG
		cout << "==== Beginning of the HalfAbsInt solve() ====\n";
#endif        
        /* HalfAbsInt main loop */
        while (!workList->isEmpty()) {
        
        	iterations++;
		fixpoint = false;
      		current = workList->get();      		
      		call_edge = detectCalls(call_node, current);
#ifdef DEBUG
        	cout << "\n=== HalfAbsInt Iteration ==\n";
        	cout << "Processing BB: " << current->number() << " \n";
#endif
		inputProcessing();
		outputProcessing();        	
	}
#ifdef DEBUG        
        cout << "==== HalfAbsInt solve() completed ====\n";
#endif
        return(iterations);
}


template <class FixPoint>
inline typename FixPoint::Domain HalfAbsInt<FixPoint>::backEdgeUnion(BasicBlock *bb) {
        typename FixPoint::Domain result(fp.bottom());
        
        if (FIRST_ITER(bb)) {
        	/* If this is the first iteration, the back edge union is Bottom. */
        	return(result);
        }
        for (BasicBlock::InIterator inedge(bb); inedge; inedge++) {
        		if (inedge->kind() == Edge::CALL)
            			continue;
                if (Dominance::dominates(bb, inedge->source())) {
                        typename FixPoint::Domain *edgeState = fp.getMark(*inedge);
                        assert(edgeState);                        
                        fp.lub(result, *edgeState);
                }
  
        }
        return(result);
}



template <class FixPoint>
inline typename FixPoint::Domain HalfAbsInt<FixPoint>::entryEdgeUnion(BasicBlock *bb) {
        typename FixPoint::Domain result(fp.bottom());
        
        for (BasicBlock::InIterator inedge(bb); inedge; inedge++) {
        		if (inedge->kind() == Edge::CALL)
            			continue;
                if (!Dominance::dominates(bb, inedge->source())) {
                        typename FixPoint::Domain *edgeState = fp.getMark(*inedge);
                        assert(edgeState);
                        fp.lub(result, *edgeState);
                }
  
        }
        fp.enterContext(result, bb);
        return(result);
}


template <class FixPoint>
inline bool HalfAbsInt<FixPoint>::tryAddToWorkList(BasicBlock *bb) {
	bool add = true;
	for (BasicBlock::InIterator inedge(bb); inedge; inedge++) {
		if (inedge->kind() == Edge::CALL)
            			continue;
		if (!isEdgeDone(*inedge)) {
			add = false;
		}	
	}
	if (add) {
		if (Dominance::isLoopHeader(bb)) {			
#ifdef DEBUG
			if (FIRST_ITER(bb) == true) {
				cout << "Ignoring back-edges for loop header " << bb->number() << " because it's the first iteration.\n";			
			}
#endif
		}
#ifdef DEBUG
		cout << "Adding to worklist BB: " << bb->number() << "\n";;
#endif
		workList->put(bb);
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


} } // end of namespace otawa::util

#endif // OTAWA_UTIL_HALFABSINT_H

