/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/prog/BBProcessor.h -- BBProcessor class implementation.
 */

#include <otawa/proc/BBProcessor.h>
#include <otawa/cfg/CFG.h>

namespace otawa {

/**
 * @class BBProcessor
 * This processor is dedicated to the basic block process thru proccessBB()
 * method. Yet, it may also be applied to CFG and framework. In this case,
 * it is applied at each basic block from these higher structures.
 */


/**
 * @fn void BBProcessor::processBB(BasicBlock *bb);
 * Perform the work of the given basic block.
 * @param bb	Basic block to process.
 */


/**
 * See @ref CFGProcessor::processCFG()
 */
void BBProcessor::processCFG(CFG *cfg) {
	for(CFG::BBIterator bb(cfg); bb; bb++)
		processBB(bb);
}
	
} // otawa
