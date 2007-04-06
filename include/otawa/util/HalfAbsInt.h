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
	Identifier<typename FixPoint::FixPointState*> FIXPOINT_STATE;	
	inline bool isEdgeDone(Edge *edge);	
	inline void tryAddToWorkList(BasicBlock *bb);
	
  public:
  	inline typename FixPoint::FixPointState *getFixPointState(BasicBlock *bb);
	int solve(void);
	inline HalfAbsInt(FixPoint& _fp, FrameWork& _fw);
	inline ~HalfAbsInt(void);
	inline typename FixPoint::Domain backEdgeUnion(BasicBlock *bb);
	inline typename FixPoint::Domain entryEdgeUnion(BasicBlock *bb);
	
	
};


template <class FixPoint>
inline HalfAbsInt<FixPoint>::HalfAbsInt(FixPoint& _fp, FrameWork& _fw)
 : entry_cfg(*ENTRY_CFG(_fw)), cur_cfg(ENTRY_CFG(_fw)), fw(_fw), fp(_fp), FIXPOINT_STATE("", NULL, otawa::NS) {
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
int HalfAbsInt<FixPoint>::solve(void) {        
        BasicBlock *current;
        bool fixpoint, call_node;
        int iterations = 0;
        Edge *call_edge;
        typename FixPoint::Domain in(fp.bottom());
        typename FixPoint::Domain out(fp.bottom());
        
        
        /*
         * First initialize the worklist with the Entry BasicBlock
         */
    
        workList->reset();
        callStack->clear();
        workList->put(entry_cfg.entry());
#ifdef DEBUG
		cout << "==== Beginning of the HalfAbsInt solve() ====\n";
#endif        
        while (!workList->isEmpty()) {

        	iterations++;
			fixpoint = false;
			call_node = false;
			call_edge = NULL;
      		current = workList->get();      		
        	for (BasicBlock::OutIterator outedge(current); outedge; outedge++) {
        		if (outedge->kind() == Edge::CALL) {
        			call_edge = *outedge;
        			if (!fp.getMark(call_edge)) {
        				call_node = true;
        			}
        		}
        	}
        
#ifdef DEBUG
        	cout << "\n=== HalfAbsInt Iteration ==\n";
        	cout << "Processing BB: " << current->number() << " \n";
#endif
        	
        	if (current->isEntry()) {
        		if (fp.getMark(current)) {
        			fp.assign(in, *fp.getMark(current));
        			fp.unmarkEdge(current);
#ifdef DEBUG
        			cout << "Just after function call.\n";
#endif
        		} else {	        		
	        		fp.assign(in, fp.entry());
        		}
			} else if (call_edge && !call_node) {
				/* Call return. */
#ifdef DEBUG
				cout << "Just after function return.\n";
#endif
				fp.assign(in, *fp.getMark(call_edge));
				fp.unmarkEdge(call_edge);
        	} else if (Dominance::isLoopHeader(current)) {
        		/*
        		 * The current block is a loop header: launch fixPoint() to determine IN value, and
        		 * to know if the fixpoint is reached.
        		 * If this is the first time we do this loop, initialize the FixPointState.
        		 */
        		if (FIRST_ITER(current))
        			FIXPOINT_STATE(current) = fp.newState();
        		
            	fp.fixPoint(current, fixpoint, in, FIRST_ITER(current));
#ifdef DEBUG
            	cout << "Loop header " << current->number() << ", fixpoint reached = " << fixpoint << "\n";
#endif            	
            	/* This is not the first iteration for this loop anymore. */
            	if (FIRST_ITER(current)) {
            		FIRST_ITER(current) = false;
            		assert(!fixpoint); /* The fixpoint can never be reached during the first iteration of the loop. */
            	}
            	 
            	FIXED(current) = fixpoint;
            	
            	/* The values of the loop-header's back-edges are not needed anymore, so we unmark them */
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
            		/* If fixpoint reached, we will not do another iteration, so the
            		 * values of the entry edges are not needed anymore. We also delete the unneeded FixPointState.
            		 */
            		fp.fixPointReached(current);
            		delete FIXPOINT_STATE(current);
            		FIXPOINT_STATE(current) = NULL;
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
                	
                	/* 
                	 * Reset FIRST_ITER to true, in case we need to re-do this loop later.
                	 */
            		FIRST_ITER(current) = true;            	        		
            	}               	
            } else {
				/* The current BasicBlock is a standard non-loop-header block: simple case.
				 * Build IN set from the predecessors, then 
				 * Un-mark all the in-edges since the values are not needed. 
				 */ 
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
            
            /*
             * The current BasicBlock is processed. Because of this, we can
             * add blocks to the WorkList. 
             */
            
            if (Dominance::isLoopHeader(current) && fixpoint) {
            	/*
            	 * if we just processed a LoopHeader and we just found the fixpoint for the associated loop,
            	 * we can try to add the successors of the loop-exit-edges to the worklist. The loop exit edges
            	 * are already marked.
            	 */
            	elm::genstruct::Vector<Edge*> *vec;
            	vec = EXIT_LIST(current);
      
            	for (elm::genstruct::Vector<Edge*>::Iterator iter(*EXIT_LIST(current)); iter; iter++) {
#ifdef DEBUG            		
            		cout << "Activating edge: " << iter->source()->number() << "->" << iter->target()->number() << "\n";
#endif            		
            		tryAddToWorkList(iter->target());
            	}            		
            	 
            } else {            	
            	/*
            	 * If we just processed a simple BasicBlock, we can try to add its successors to the worklist.
            	 */
            	 
            	if (call_node || !call_edge)
	            	fp.update(out, in, current);
#ifdef DEBUG            	
            	cout << "Updating for basicblock: " << current->number() << "\n"; 
#endif            	
            	/* Call the Listener here ... */ 
            	fp.blockInterpreted(current, in, out, cur_cfg);
            	
            	if (current->isExit() && (callStack->length() > 0)) {
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
            		/*
            		 * Unprocessed function call:
            		 * Put current node in stack, add function's entry to worklist
            		 */
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
        	    	/* Mark out-edges, and try to add successors to worklist */
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
        return(result);
}


template <class FixPoint>
inline void HalfAbsInt<FixPoint>::tryAddToWorkList(BasicBlock *bb) {
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
			/* It is no longer the first time that this Loop Header is added to the WorkList. */
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

