
/**
 * @class FirstUnrollingFixPoint
 *
 * FixPoint class for HalfAbsInt
 * The WideningFixPoint manages loops in a simple way. When a loop header is reached,
 * it iterates over the loop until a fixpoint is reached.
 * 
 * @par
 * 
 * @param FixPoint Class describing the ProbleM.
 * 
 * @param Listener Class used to retrieve the informations computed by the analysis
 * 
 * @par Problem
 * 
 * This parameter must match the following signature (the method's meaning are the same as in WideningFixPoint class):
 * <code><pre>
 * class Problem {
 * 		
 * 		class Domain; (May be defined by a typedef)
 * 
 * 		const Domain& bottom();
 * 		const Domain& entry()
 * 		void lub(Domain &a, const Domain &b) const;
 * 		void assign(Domain &a, const Domain &b) const;
 * 		bool equals(const Domain &a, const Domain &b) const;
 * 		void update(Domain& out, Domain& in, BasicBlock *bb);
 * 		void enterContext(Domain &dom, BasicBlock *header)
 * 		void leaveContext(Domain &dom, BasicBlock *header)
 * }
 * </pre></code>
 * @par Listener
 * 
 * This parameter must match the following signature (the method's meaning are the same as in WideningFixPoint class):
 * <code><pre>
 * class Listener {
 * 		class Problem; (Will usually be defined by a typedef )
 *	 	void blockInterpreted(const WideningFixPoint< DefaultListener >  *fp, BasicBlock* bb, const typename Problem::Domain& in, const typename Problem::Domain& out, CFG *cur_cfg) const;
 * 		void fixPointReached(const WideningFixPoint<DefaultListener > *fp, BasicBlock*bb );
 * 		Problem& getProb();
 * }
 * </pre></code>

 */
 
/**
 * @fn void WideningFixPoint::init (util::HalfAbsInt< WideningFixPoint > *_ai)
 * Is called by HalfAbsInt for fixpoint object initializeation.
 * @param _ai HalfAbsint object. 
 */
 
/**
 * @fn void WideningFixPoint::fixPoint (BasicBlock *bb, bool &fixpoint, Domain &in, bool firstTime) const
 * Main fixPoint function: is called by HalfAbsInt whenever the analysis reaches a loop header. Its
 * purpose is to detect if the fixpoint's reached, and compute the new loop entry state. It may
 * store state information into the HalfAbsInt's FixPointState object.
 * @param bb loop header basic block
 * @param fixpoint fixPoint() must store True here when the fixpoint is reached.
 * @param in fixPoint() must store here the new loop entry state.
 * @param fistTime is True if it's the first time we call fixPoint for this loop.  
 */
 
/**
 * @fn void WideningFixPoint::markEdge (PropList *e, const Domain &s)
 * Marks a proplist (typically an edge) e with state s.
 * @param e Proplist to mark
 * @param s State 
 */
 
/**
 * @fn void WideningFixPoint::unmarkEdge (PropList *e)
 * Unmark a proplist (typically an edge)
 * @param e Proplist to unmark
 */
 
/**
 * @fn Domain* WideningFixPoint::getMark (PropList *e)
 * Get the mark of a proplist (typically an edge)
 * @param e The proplist to getmark
 */
/**
 * @fn const Domain& WideningFixPoint::bottom (void) const
 * This function gets the bottom of the problem's lattice
 * This is usually delegated to the Problem.
 * @return Bottom
 */
/**
 * @fn const Domain& WideningFixPoint::entry (void) const
 * This function returns the entry state of the whole program to be analyzed.
 * This is usually delegated to the Problem.
 * @return Entry state
 */
 
/**
 * @fn void WideningFixPoint::lub (Domain &a, const Domain &b) const
 * Performs operation: a = a lub b
 * This is delegated to the Problem.
 * with lub = Lowest Upper Bound. 
 * @param a State a
 * @param b State b
 */
 
/**
 * @fn void WideningFixPoint::assign (Domain &a, const Domain &b) const
 * Performs operation a = b
 * This is delegated to the Problem.
 * @param a State a
 * @param b State b
 */
/**
 * @fn bool WideningFixPoint::equals (const Domain &a, const Domain &b) const
 * Tests a and b for equality. 
 * This is delegated to the Problem.
 * @param a State a
 * @param b State b
 * @return True if equal
 */
 
/**
 * @fn void WideningFixPoint::update (Domain &out, const Domain &in, BasicBlock *bb)
 * Is called whenever the analysis needs to get the Output state for a basic block, given
 * its input state. 
 * This is delegated to the Problem.
 * @param out Output state
 * @param in Input state
 * @param bb Basic Block 
 */
 
/**
 * @fn void WideningFixPoint::blockInterpreted (BasicBlock *bb, const Domain &in, const Domain &out, CFG *cur_cfg) const
 * This listener is called whenever HalfAbsInt has processed a basic block. The listener must gather 
 * and store the information for the user. 
 * This is delegated to the Listener. 
 * @param bb The basic block
 * @param in The input state
 * @param out The output state
 * @param cur_cfg Current CFG.
 */
 
/**
 * @fn void WideningFixPoint::fixPointReached (BasicBlock *bb) const
 * This listener is called whenever a fixpoint is reached for a loop.
 * This is delegated to the Listener.
 * @param bb The loop header.
 */
 
/** 
 * @fn void WideningFixPoint::enterContext (Domain &dom, BasicBlock *bb) const
 * This is called whenever we enter a loop.
 * @param dom State (optionnally to be modified according to the new context)
 * @param bb Loop header
 */
 
/**
 * @fn void WideningFixPoint::leaveContext (Domain &dom, BasicBlock *bb) const
 * This is called whenever we leave a loop.
 * @param dom State (optionally to be modified according to the new context)
 * @param bb Loop header
 */

/**
 * @class FirstUnrollingFixPoint::FixPointState
 *
 * State info class for FirstUnrollingFixPoint
 * This FixPoint needs to remember:
 * - The loop header state (headerState)
 * - The current iteration number for the loop (numIter)
 * - The resulting state of the first iteration (firstIterState), that is, the union of the back edge states
 *    at the end of the first iteration.
 */
