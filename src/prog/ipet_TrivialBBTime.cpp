/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/prog/ipet_TrivialBBTime.cpp -- TrivialBBTime class implementation.
 */

#include <otawa/ipet/TrivialBBTime.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/ipet/IPET.h>

namespace otawa {

/**
 * @class TrivialBBTime
 * This processor is used for computing execution of basic blocks in a trivial
 * way, that is, the multiplication of basic block instruction count by the
 * pipeline depth.
 */


/**
 * @fn TrivialBBTime::TrivialBBTime(int depth);
 * Build the processor.
 * @param depth	Depth of the pipeline.
 */


/**
 * @fn int TrivialBBTime::depth(void) const;
 * Get the depth of the pipeline.
 * @return	Pipeline depth.
 */


/**
 * See @ref CFGProcessor::processBB().
 */
void TrivialBBTime::processBB(BasicBlock *bb) {
	bb->set<int>(&IPET::ID_Time, dep * bb->countInstructions());
}

} // otawa
