/*
 *	TimeDeltaObjectFunctionModifier implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2006-07, IRIT UPS.
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
#include <otawa/ipet/IPET.h>
#include <otawa/cfg.h>
#include <otawa/ilp.h>
#include <otawa/tsim/TimeDeltaObjectFunctionModifier.h>
#include <otawa/ipet/BasicObjectFunctionBuilder.h>
#include <otawa/proc/ProcessorException.h>

using namespace otawa::ilp;

namespace otawa { namespace tsim {

/**
 * @class TimeDeltaObjectFunctionModifier
 * This processor is used for modify the object function
 * to maximize for resolving the IPET system, that is,
 * @p For each edge where a @ref TIME_DELTA is found, it adds:
 * @p	tdelta * ei,j
 * @p where
 * <dl>
 * 	<dt>tdelta</dt><dd>Delta time found on edge (i, j).</dd>
 *  <dt>ei,j</dt><dd>Count of execution of edge (i, j).</dd>
 * </dl>
 *
 * @par Provided Feature
 * @li @ref EDGE_TIME_FEATURE
 *
 * @par Required Feature
 * @li @ref ILP_SYSTEM_FEATURE
 */

p::declare TimeDeltaObjectFunctionModifier::reg = p::init("otawa::tsim::TimeDeltaObjectFunctionModifier", Version(1, 0, 1))
	.provide(EDGE_TIME_FEATURE)
	.require(ipet::ILP_SYSTEM_FEATURE)
	.require(ipet::OBJECT_FUNCTION_FEATURE)
	.maker<TimeDeltaObjectFunctionModifier>();


/**
 * Build a new basic object function builder.
 * @param props		Configuration properties.
 */
TimeDeltaObjectFunctionModifier::TimeDeltaObjectFunctionModifier(p::declare& r): BBProcessor(r) {
}


/**
 */
void TimeDeltaObjectFunctionModifier::processBB(
	WorkSpace *fw,
	CFG *cfg,
	BasicBlock *bb)
{
	if(!bb->isEntry()) {
		System *system = ipet::SYSTEM(fw);
		for(BasicBlock::InIterator edge(bb); edge; edge++) {
			int time = ipet::TIME_DELTA(edge);
			if(time)
				system->addObjectFunction(time, ipet::VAR(edge));
		}
	}
}

/**
 * This feature ensurers that the @ref TIME_DELTA property linked to the CFG edges are
 * used in the maximized object function of the IPET ILP system.
 */
Feature<TimeDeltaObjectFunctionModifier> EDGE_TIME_FEATURE("otawa::tsim::EDGE_TIME_FEATURE");

} } // otawa::tsim
