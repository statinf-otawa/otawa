/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/prog/ipet_TrivialBBTime.cpp -- TrivialBBTime class implementation.
 */

#include <otawa/ipet/TrivialBBTime.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/ipet/IPET.h>

using namespace elm;

namespace otawa { namespace ipet {

/**
 * @class TrivialBBTime
 * This processor is used for computing execution of basic blocks in a trivial
 * way, that is, the multiplication of basic block instruction count by the
 * pipeline depth.
 */


/**
 * Build the processor.
 * @param depth	Depth of the pipeline.
 */
TrivialBBTime::TrivialBBTime(int depth, const PropList& props)
: BBProcessor("otawa::TrivialBBTime", Version(1, 0, 0), props), dep(depth) {
	assert(depth > 0);
}



/**
 * @fn int TrivialBBTime::depth(void) const;
 * Get the depth of the pipeline.
 * @return	Pipeline depth.
 */


/**
 * See @ref CFGProcessor::processBB().
 */
void TrivialBBTime::processBB(FrameWork *fw, CFG *cfg, BasicBlock *bb) {
	TIME(bb) = dep * bb->countInstructions();
}

} } // otawa::ipet
