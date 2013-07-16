/*
 *	ipet::BasicObjectFunctionBuilder implementation
 *	Copyright (c) 2005-12, IRIT UPS <casse@irit.fr>
 *
 *	This file is part of OTAWA
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
 *	along with Foobar; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include <otawa/ipet/IPET.h>
#include <otawa/cfg.h>
#include <otawa/ilp.h>
#include <otawa/ipet/BasicObjectFunctionBuilder.h>
#include <otawa/proc/ProcessorException.h>
#include <otawa/ipet/VarAssignment.h>
#include <otawa/ipet/TrivialBBTime.h>
#include <otawa/ipet/ILPSystemGetter.h>

using namespace otawa::ilp;

namespace otawa { namespace ipet {

/**
 * @class BasicObjectFunctionBuilder
 * This processor is used for building the basic object function function
 * to maximize for resolving the IPET system, that is,
 * @p Then, it set the object function as maximizing the following expression:
 * @p	t1 * n1 + t2 * n2 + ... + tm * nm
 * @p where
 * <dl>
 * 	<dt>tk</dt><dd>Time of execution of basic block k.</dd>
 *  <dt>nk</dt><dd>Count of execution if basic block k.</dd>
 * </dl>
 *
 * @par Required Features
 * @li @ref ipet::ILP_SYSTEM_FEATURE
 * @li @ref ipet::ASSIGNED_VARS_FEATURE
 * @li @ref ipet::BB_TIME_FEATURE
 *
 * @par Provided Features
 * @li @ref ipet::OBJECT_FUNCTION_FEATURE
 */

p::declare BasicObjectFunctionBuilder::reg = p::init("otawa::ipet::BasicObjectFunctionBuilder", Version(1, 0, 1))
	.require(ASSIGNED_VARS_FEATURE)
	.require(BB_TIME_FEATURE)
	.require(ILP_SYSTEM_FEATURE)
	.provide(OBJECT_FUNCTION_FEATURE)
	.maker<BasicObjectFunctionBuilder>();

/**
 * Build a new basic object function builder.
 */
BasicObjectFunctionBuilder::BasicObjectFunctionBuilder(p::declare& r)
: BBProcessor(r) {
}


/**
 */
void BasicObjectFunctionBuilder::processBB(
	WorkSpace *fw,
	CFG *cfg,
	BasicBlock *bb)
{
	if(!bb->isEntry() && !bb->isExit()) {
		System *system = SYSTEM(fw);
		bool times_on_edges = true;
		int time;
		for (BasicBlock::InIterator edge(bb); edge; edge++) {
		    if (TIME(edge) >= 0)
			time = TIME(edge);
		    else
			time = TIME(bb);
		    if(time < 0)
			throw ProcessorException(*this, _ << "no time on BB " << bb->address()
						 << " (" << bb->number() << " of " << &cfg->label());
		    system->addObjectFunction(time, VAR(edge));
		}
	}
}

static SilentFeature::Maker<BasicObjectFunctionBuilder> maker;
/**
 * This feature ensures that the object function of the ILP system to solve
 * has been built.
 */
SilentFeature OBJECT_FUNCTION_FEATURE("otawa::ipet::OBJECT_FUNCTION_FEATURE", maker);


} } // otawa::ipet
