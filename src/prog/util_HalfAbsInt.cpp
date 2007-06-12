/*
 * $Id$
 * Copyright (c) 2006 IRIT-UPS
 *
 * include/otawa/util/HalfAbsInt.h -- "Half" abstract interpretation class interface.
 */

#include <otawa/util/HalfAbsInt.h>
#include <elm/genstruct/Vector.h>

namespace otawa { namespace util {



/**
 * @class HalfAbsInt
 *
 * Implements abstract interpretation.
 *
 * @par
 * 
 * @param FixPoint Class used to manage loops.
 * 
 * @par FixPoint
 * 
 * This parameter must match the following signature:
 * <code><pre>
 * class FixPoint {
 * 		typedef ... Domain;
 * 		class FixPointState {
 * 			...
 * 		};
 * 
 * 		void init(CFG*, HalfAbsInt<FixPoint>*);
 * 		FixPointState *newState(void);
 * 		void fixPoint(BasicBlock*, bool&, Domain&, bool) const;
 * 		void markEdge(Edge*, Domain&);
 * 		void unmarkEdge(Edge*);
 * 		Domain* getMark(Edge*);
 * 
 * 		const Domain& bottom(void);
 *		void lub(Domain &, const Domain &) const;
 *		void assign(Domain &, const Domain &) const;
 *		bool equals(const Domain &, const Domain &) const;
 *		void update(Domain &, const Domain &, BasicBlock*) const;
 *		inline void blockInterpreted(BasicBlock*, const Domain&, const Domain&) const ;
 *		inline void fixPointReached(BasicBlock*) const;
 * 
 * }
 * </pre></code>
 * 
 * @par Signature explanation 
 * 
 * @li The Domain type is the Domain of the abstract values used in the abstract interpretation.
 * 
 * @li The FixPointState class can holds, for each loop, status informations needed by FixPoint. 
 * This status information can be accessed and modified by calling the halfAbsInt::getFixPointState() method.
 * 
 * @li void init(CFG*, HalfAbsInt<FixPoint>*) : Method called at HalfAbsInt initialization. Gives FixPoint the 
 * chance to perform initialization tasks that were impossible to do in FixPoint's constructor because HalfAbsInt was not
 * instantiated yet.
 * 
 * @li FixPointState *newState(void) : Create a new (empty) FixPointState.
 * 
 * @li markEdge(Edge*, Domain&) : Mark the edge with the value.
 * 
 * @li unmarkEdge(Edge*) : Delete the mark from the edge.
 *  
 * @li Domain* getMark(Edge*) : Retrieve the value from the marked edge.
 * 
 * @li Domain& bottom(void) : Returns the least abstract value (BOTTOM) 
 *
 * @li The lub, assign, equals methods are easy to understand.
 *  
 * @li void update(Domain& out, const Domain& in, BasicBlock*) const : Compute the OUT value, from the IN value and the BasicBlock.
 * 
 * @li void blockInterpreted(BasicBlock*, const Domain& in, const Domain& out) const : Is called when a BasicBlock is interpreted by
 * HalfAbsInt. Informations provided: The BasicBlock, and its IN & OUT values
 * 
 * @li void FixPointReached(BasicBlock*) const : Called by HalfAbsInt when the fixpoint is reached on a particular loop. The loop header
 * is passed in the parameter.
 */
 
/**
 * This property is attached to the loop headers, and is true if
 * the FixPoint for the associated loop has been reached.
 * 
 * @par Hooks
 * @li @ref BasicBlock
 */
Identifier<bool> FIXED("otawa::util::fixed", false);

/**
 * This property is attached for the loop header, and is true if
 * the first iteration of the associated loop is not done yet.
 * This is useful to determine if we can add the loop header to the worklist
 * even if the back edges going to it are not marked yet.
 *  
 * @par Hooks
 * @li @ref BasicBlock
 */
Identifier<bool> FIRST_ITER("otawa::util::first_iter", true);


/**
 * @fn typename FixPoint::FixPointState *HalfAbsInt::getFixPointState(BasicBlock *bb);
 * Get the FixPointState of a loop.
 * 
 * @param bb The header of the loop which we want to get the state.
 * @return The requested FixPointState info.
 */
 
/**
 * @fn int HalfAbsInt::solve(void)
 * Do the abstract interpretation of the CFG.
 * 
 * @return The number of iterations of the algorithm.
 */
 
/**
 * @fn typename FixPoint::Domain HalfAbsInt::backEdgeUnion(BasicBlock *bb)
 * Given a loop header, returns the union of the value of its back edges.
 *  
 * @param bb The loop header.
 * @return The unionized value.
 */
 
/**
 * @fn typename FixPoint::Domain HalfAbsInt::entryEdgeUnion(BasicBlock *bb)
 * Given a loop header, returns the union of the value of is entry edges (the entry edges
 * are the in-edges that are not back-edges.) 
 * 
 * @param bb The loop header
 * @return The unionized value.
 */
 
/**
 * @fn void HalfAbsInt::tryAddToWorkList(BasicBlock *bb)
 * Try to add the BasicBlock to the worklist.
 * In order to be added, the BasicBlock's in-edges 
 * must verify some conditions. These conditions are verified by isEdgeDone()
 * 
 * @param bb The BasicBlock to try to add.
 */
 
/**
 * @fn inline bool HalfAbsInt::isEdgeDone(Edge *edge)
 * Tests if an edge verify the conditions needed to add its target BasicBlock.
 * 
 * @param edge The edge to test
 * @return Returns false if the edge's target cannot be added to the worklist. True otherwise.  
 */
 

} } // otawa::util
