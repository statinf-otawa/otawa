/*
 *	This file is part of OTAWA
 *	Copyright (c) 2006-11, IRIT UPS.
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

#include <otawa/dfa/hai/HalfAbsInt.h>
#include <elm/genstruct/Vector.h>

namespace otawa { namespace dfa { namespace hai {

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
 *		void blockInterpreted(BasicBlock*, const Domain&, const Domain&) const ;
 *		void fixPointReached(BasicBlock*) const;
 * 		const Domain& entry(void);
 * }
 * </pre></code>
 * 
 * @par Signature explanation 
 * 
 * @li Domain -- this type is the Domain of the abstract values used in the abstract interpretation.
 * @li FixPointState -- this class can holds, for each loop, status informations needed by FixPoint.
 * This status information can be accessed and modified by calling the halfAbsInt::getFixPointState() method.
 * @li void init(CFG*, HalfAbsInt<FixPoint>*) -- Method called at HalfAbsInt initialization. Gives FixPoint the
 * chance to perform initialization tasks that were impossible to do in FixPoint's constructor because HalfAbsInt was not
 * instantiated yet.
 * @li FixPointState *newState(void) -- Create a new (empty) FixPointState.
 * @li markEdge(Edge*, Domain&) -- Mark the edge with the value.
 * @li unmarkEdge(Edge*) -- Delete the mark from the edge.
 * @li Domain* getMark(Edge*) -- Retrieve the value from the marked edge.
 * @li Domain& bottom(void) -- Returns the least abstract value (BOTTOM)
 * @li The lub, assign, equals methods are easy to understand.
 * @li void update(Domain& out, const Domain& in, BasicBlock*) const -- Compute the OUT value, from the IN value and the BasicBlock.
 * @li void blockInterpreted(BasicBlock*, const Domain& in, const Domain& out) const -- Is called when a BasicBlock is interpreted by
 * HalfAbsInt. Informations provided: The BasicBlock, and its IN & OUT values
 * @li void FixPointReached(BasicBlock*) const -- Called by HalfAbsInt when the fixpoint is reached on a particular loop. The loop header
 * is passed in the parameter.
 * @li const Domain& bottom(void) -- returns the least element of the domain.
 * @li const Domain& entry(void) -- returns the domain value at the entry of CFG.
 *
 * An important precondition on the implementation of the fixpoint is that the Domain MUST support a complete partial order, we use "<=" to denote this order.
 * The following applies on the different functions handling the domain:
 * @li bottom() must returns the least element of the domain, that is, whatever d in Domain, bottom() <= d.
 * @li update() must be monotonic, that is, if d1 <= d2, update(d'1, BB, d1) and update(d'2, BB, d2), d'1 <= d'2.
 * @li lub() must gives a greater element, that is, if d = d1 and join(d, d2) then d1 <= d and d2 <= d.
 *
 * If these constraints are not ensured, the termination of the analysis performed by HalfAbsInst can not be asserted.
 *
 * @par Inside a code processor
 *
 * To be used inside an OTAWA code processor, the following features must be available
 * (passed in the requirement list of the code processor):
 * @li otawa::LOOP_INFO_FEATURE
 * @li otawa::COLLECTED_CFG_FEATURE
 *
 * @par Debugging the analysis
 * Causes of a failed abstract interpretation performed by HalfAbsInt may be hard to understand.
 * To get details on what does the analyzer for debugging purpose, you may define the constant HAI_DEBUG before
 * including the HalfAbsInt header file. When the analysis will be launched, the processing performed
 * on basic block will be outputted with the values of the domain. To have a readable display of the domain,
 * you have also to provide an overload of "<<" operator to perform the display:
 * @code
 * #define HAI_DEBUG
 * #include <otawa/util/HalfAbsInt.h>
 *
 * // definition of domain, fixpoint, etc
 *
 * elm::io::Output operator<<(elm::io::Output& out, const TheDomain& d) {
 * 		// display the domain to output
 * }
 * @endcode
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
 * This property, when set to TRUE on a BasicBlock or a CFG, 
 * prevents HalfAbsInt from following edges to this BasicBlock
 * or CFG.
 * NOTE: It is deprecated to use this property to prevent HalfAbsint
 * from entering a sub-CFG. Use HAI_BYPASS_EDGE instead. 
 */
Identifier<bool> HAI_DONT_ENTER("otawa::util::hai_dont_enter", false);

/**
 * This property enables the user to create a virtual "bypass" edge from the source
 * block to the target block. Moreover, it ensures that this virtual edge is the only
 * mean to reach target basic block from source basic block.
 * This should be used only to bypass function calls in inlined CFGs.
 */
 
Identifier<BasicBlock*> HAI_BYPASS_SOURCE("otawa::util::HAI_BYPASS_SOURCE", 0);
Identifier<BasicBlock*> HAI_BYPASS_TARGET("otawa::util::HAI_BYPASS_TARGET", 0);

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
 

} } } // otawa::dfa::hai
