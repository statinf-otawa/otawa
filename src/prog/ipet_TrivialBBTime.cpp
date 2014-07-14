/*
 *	TrivialBBTime processor implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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

p::declare TrivialBBTime::reg = p::init("otawa::ipet::TrivialBBTime", Version(1, 1, 0))
	.base(BBProcessor::reg)
	.maker<TrivialBBTime>()
	.provide(BB_TIME_FEATURE);

/**
 * Build the processor.
 */
TrivialBBTime::TrivialBBTime(p::declare& r): BBProcessor(r), dep(5) {
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
 *
 * @par Properties
 * @li otawa::ipet::TIME
 * @li otawa::ipet::DELTA_TIME
 */
SilentFeature BB_TIME_FEATURE("otawa::ipet::BB_TIME_FEATURE", maker);

} } // otawa::ipet
