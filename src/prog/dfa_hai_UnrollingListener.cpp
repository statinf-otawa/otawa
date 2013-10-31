
/**
 * @class UnrollingListener
 * This listener gathers in an array the LUB of the in-states for all analyzed basic blocks.
 * At the end of the analysis, you can access result[CFGNUMBER][BBNUMBER] to obtain
 * the in-state of the basic block BBNUMBER of cfg CFGNUMBER.
 * Note: The listener merges the in-states corresonding to various iterations of the same BB. If you
 * want to differentiate individual in-states (for example, BB3 first iteration and BB3 other iterations)
 * you have to modify the blockInterpreted() method to take into account the iteration number, which
 * can be obtained with FirstUnrollingFixPoint::getIter() method.
 *  
 */

/**
 * @fn blockInterpreted(const FirstUnrollingFixPoint< UnrollingListener >  *fp, BasicBlock* bb, const typename Problem::Domain& in, const typename Problem::Domain& out, CFG *cur_cfg, elm::genstruct::Vector<Edge*> *callStack) const
 * This is called whenever a block is processed. In the UnrollingListener, we do the LUB of all the in-states
 * of each block.
 * @param fp The FixPoint object
 * @param bb The Basic Block
 * @param in In-state
 * @param out Out-state (not used in this class)
 * @param cur_cfg Current CFG   
 */
 
/**
 * @fn void UnrollingListener::fixPointReached (const UnrollingFixPoint< UnrollingListener > *fp, BasicBlock *bb)
 * This is called whenever a loop's fixpoint is reached.
 * @param fp The FixPoint object
 * @param bb The loop header. 
 */

/**
  * Problem & UnrollingListener::getProb ()
  * This function is for getting the Problem associated with this listener.
  * @return The problem.
 */
 
 
#include <otawa/util/UnrollingListener.h>

namespace otawa {

} // otawa
