/*
 * $Id$
 * Copyright (c) 2006 IRIT-UPS
 *
 * include/otawa/util/DFAEngine.h -- DFAEngine class interface.
 */

#include <otawa/util/DFAEngine.h>

namespace otawa { namespace util {

/**
 * @class DFAEngine
 * This class is a replacement for the old class @ref otawa::DFA.
 * It provides more flexibility and more performances
 * @par
 * This class implements a Data Flow Analysis based on CFG and gen and kill sets.
 * It requires three parameter classes.
 * <dl>
 * 
 * <dt>Set</dt>
 * <dd>This class represents the sets of computed data. It must provides
 * methods for resetting the set, for adding a set, removing
 * a set (remove()) and for comparing by equality.
 * <pre><code>
 * class Set {
 * 		void reset(void);
 * 		void add(Set *set);
 * 		void remove(Set *set);
 * 		bool equals(Set *set);
 * };
 * </code></pre>
 * </dd>
 * 
 * <dt>Problem</dt>
 * <dd>The problems provides an empty set and gen and kill sets.</dd>
 * <pre><code>
 * class Problem {
 * 		Set *empty(void);
 * 		Set *gen(BasicBlock *bb);
 * 		Set *kill(BasicBlock *bb);
 * };
 * </code></pre>
 * <dt>Iter</dt>
 * <dd>This class is an iterator on the edge of a basic block. Its default
 * value is @ref DFAPredecessor (for usual forward DFA) but it may also be
 * @ref DFASuccessor (for a CFG reverse computation). This class must be an iterator
 * on the edge of basic block passed to the constructor.</dd>
 * </code>
 * </dl>
 */


/**
 * @fn DFAEngine::DFAEngine(Problem& problem, CFG& cfg);
 * Build a DFA engine with the given problem and CFG.
 * @param problem	Problem to resolve.
 * @param cfg		CFG to resolve problem on.
 */


/**
 * @fn void DFAEngine::compute(void);
 * Compute the solution. According the size of the CFG and of the set, this call
 * may take a large bunch of time.
 */


/**
 * @fn Set *DFAEngine::inSet(BasicBlock *bb);
 * Get the IN set of the given basic block.
 * @param bb	Used basic block.
 * @return		Matching IN set.
 */


/**
 * @fn Set *DFAEngine::outSet(BasicBlock *bb);
 * Get the OUT set of the given basic block.
 * @param bb	Used basic block.
 * @return		Matching OUT set.
 */


/**
 * @fn Set *DFAEngine::genSet(BasicBlock *bb);
 * Get the GEN set of the given basic block.
 * @param bb	Used basic block.
 * @return		Matching GEN set.
 */


/**
 * @fn Set *DFAEngine::killSet(BasicBlock *bb); 
 * Get the KILL set of the given basic block.
 * @param bb	Used basic block.
 * @return		Matching KILL set.
 */


/**
 * @class DFAPredecessor
 * This class is an iterator on the input edges of a basic block. It is used
 * the @ref DFAEngine class for forward DFA computation.
 */


/**
 * @class DFASuccessor
 * This class is an iterator on the output edges of a basic block, ignoring call
 * edges. It is used the @ref DFAEngine class for backward DFA computation.
 */

} } // otawa::util
