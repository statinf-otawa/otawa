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
 *
 * @par Configuration Properties
 * @li @ref ipet::PIPELINE_DEPTH - used to compute execution time of the basic
 * block.
 *
 * @par Provided Features
 * @li @ref ipet::BB_TIME_FEATURE
 */


/**
 * Build the processor.
 */
TrivialBBTime::TrivialBBTime(void)
: BBProcessor("otawa::TrivialBBTime", Version(1, 0, 0)), dep(5) {
	provide(BB_TIME_FEATURE);
}


/**
 */
void TrivialBBTime::configure(const PropList& props) {
	BBProcessor::configure(props);
	dep = PIPELINE_DEPTH(props);
}


/**
 */
void TrivialBBTime::processBB(WorkSpace *fw, CFG *cfg, BasicBlock *bb) {
	TIME(bb) = dep * bb->countInsts();
}


/**
 * This property is used to configure the @ref TrivialBBTime processor
 * with the depth of the used pipeline.
 *
 * @par Hooks
 * @li Configuration of @ref ipet::TrivialBBTime.
 */
Identifier<unsigned> PIPELINE_DEPTH("otawa::ipet::PIPELINE_DEPTH", 5);


static SilentFeature::Maker<TrivialBBTime> maker;
/**
 * This feature ensures that the execution time of each basic block has been
 * computed.
 */
SilentFeature BB_TIME_FEATURE("otawa::BB_TIME", maker);

} } // otawa::ipet
