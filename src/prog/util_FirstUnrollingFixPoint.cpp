
/**
 * @class FirstUnrollingFixPoint
 *
 * FixPoint class for HalfAbsInt
 * Makes the distinction between the first iteration and the other iterations of any loop.
 * getIter() method enables the Problem to know which iteration it is.
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
