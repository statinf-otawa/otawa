
/**
 * @class WideningListener
 *
 * This listener gathers in an array the LUB of the in-states for all analyzed basic blocks.
 * At the end of the analysis, you can access result[CFGNUMBER][BBNUMBER] to obtain
 * the in-state of the basic block BBNUMBER of cfg CFGNUMBER.
 * 
 */
 
/**
 * @fn void WideningListener::blockInterpreted (const DefaultFixPoint< WideningListener > *fp, BasicBlock *bb, const typename Problem::Domain &in, const typename Problem::Domain &out, CFG *cur_cfg) const
 * This is called whenever a block is processed. In the WideningListener, we do the LUB of all the in-states
 * of each block.
 * @param fp The FixPoint object
 * @param bb The Basic Block
 * @param in In-state
 * @param out Out-state (not used in this class)
 * @param cur_cfg Current CFG   
 */
 
/**
 * @fn void WideningListener::fixPointReached (const DefaultFixPoint< WideningListener > *fp, BasicBlock *bb)
 * This is called whenever a loop's fixpoint is reached.
 * @param fp The FixPoint object
 * @param bb The loop header. 
 */

/**
  * Problem & WideningListener::getProb ()
  * This function is for getting the Problem associated with this listener.
  * @return The problem.
 */
 