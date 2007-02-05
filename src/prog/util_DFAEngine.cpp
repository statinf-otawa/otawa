/*
 * $Id$
 * Copyright (c) 2006-07 IRIT-UPS
 *
 * DFAEngine and IterativeDFA classes interface
 */

#include <otawa/util/DFAEngine.h>

namespace otawa {

namespace util {

/**
 * @class DFAEngine
 * See @ref dfa::IterativeDFA
 */

/**
 * @class DFAPredecessor
 * See @ref dfa::Predecessor.
 */


/**
 * @class DFASuccessor
 * See @ref dfa::Successor.
 */


} // dfa
	
namespace dfa {

/**
 * @class IterativeDFA
 * This class is a replacement for the old class @ref otawa::DFA.
 * It provides more flexibility and more performances
 * @par
 * This class implements a Data Flow Analysis based on CFG and gen and kill sets.
 * It requires three parameter classes.
 * @param Problem	Provides the functions describing the problem : generator
 * set, killer set, initial set and methods to handle sets.
 * @param Set		Set to compute.
 * @param Iter		Iterator on the predecessor of a node for computing
 * the IN set(default). If a DFASuccessor is passed, the problem is computed
 * in CFG reverse-order.
 * <dl>
 * 
 * @par Problem
 * 
 * The problems must match the signature below.
 * <pre><code>
 * class Problem {
 * 		Set *empty(void);
 * 		Set *gen(BasicBlock *bb);
 * 		Set *kill(BasicBlock *bb);
 * 		bool equals(Set *set1, Set *set2);
 * 		void reset(Set *set);
 * 		void merge(Set *set1, Set *set2);
 * 		void set(Set *dset, Set *tset);
 * 		void add(Set *dset, Set *tset);
 * 		void diff(Set *dset, Set *tset);
 * };
 * </code></pre> 
 */


/**
 * @fn IterativeDFA::IterativeDFA(Problem& problem, CFG& cfg);
 * Build a DFA engine with the given problem and CFG.
 * @param problem	Problem to resolve.
 * @param cfg		CFG to resolve problem on.
 */


/**
 * @fn void IterativeDFA::compute(void);
 * Compute the solution. According the size of the CFG and of the set, this call
 * may take a large bunch of time.
 */


/**
 * @fn Set *IterativeDFA::inSet(BasicBlock *bb);
 * Get the IN set of the given basic block.
 * @param bb	Used basic block.
 * @return		Matching IN set.
 */


/**
 * @fn Set *IterativeDFA::outSet(BasicBlock *bb);
 * Get the OUT set of the given basic block.
 * @param bb	Used basic block.
 * @return		Matching OUT set.
 */


/**
 * @fn Set *IterativeDFA::genSet(BasicBlock *bb);
 * Get the GEN set of the given basic block.
 * @param bb	Used basic block.
 * @return		Matching GEN set.
 */


/**
 * @fn Set *IterativeDFA::killSet(BasicBlock *bb); 
 * Get the KILL set of the given basic block.
 * @param bb	Used basic block.
 * @return		Matching KILL set.
 */


/**
 * @class Predecessor
 * This class is an iterator on the input edges of a basic block. It is used
 * the @ref DFAEngine class for forward DFA computation.
 */


/**
 * @class Successor
 * This class is an iterator on the output edges of a basic block, ignoring call
 * edges. It is used the @ref DFAEngine class for backward DFA computation.
 */

} } // dfa::util
